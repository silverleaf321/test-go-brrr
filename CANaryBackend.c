#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

typedef struct Sensor_Signal {
    char* name;
    char* unit;
    uint8_t startBit;
    int len;
    float scalingMultiplier;
    float scalingAddition;
    uint8_t sign;
    float value;
    struct Sensor_Signal* next;
} Sensor_Signal;

typedef struct Tree_Node {
    uint16_t id;
    int height;
    uint8_t type;
    uint64_t data;
    struct Tree_Node* left;
    struct Tree_Node* right;
    Sensor_Signal* head;
} Tree_Node;

// Parent Node for Signal tree
Tree_Node* parent = NULL;
int max_signals = 0;

// Paths for DBC file, Log file, New (Converted) Log file, Compressed Log file, and Brotli Log file
char dbcFile[256];
char logFile[256];
char newLogFile[256];
char compressedFile[256];
char brotliFile[256];

// Function Definitions
int max(int a, int b);
int height(Tree_Node* node);
int getBalance(Tree_Node* node);
void preOrder(Tree_Node* root, uint8_t operation, FILE* fp);
Tree_Node* rightRotate(Tree_Node* y);
Tree_Node* leftRotate(Tree_Node* x);
Tree_Node* newNode(uint16_t id, uint8_t type);
Tree_Node* insertNode(Tree_Node* node, uint16_t id, uint8_t type);
Tree_Node* findNode(Tree_Node* head, int id);
Tree_Node* parseDBC(const char* fileName, Tree_Node* head);
int compressData(const char* fileName, int brotliFlag);
int parseData(const char* fileName, int rate);
int updateLog(const char* fileName);

int max(int a, int b) {
    return (a > b) ? a : b;
}

int height(Tree_Node* node) {
    if (node == NULL) {
        return 0;
    }
    return node->height;
}

int getBalance(Tree_Node* node) {
    if (node == NULL) {
        return 0;
    }
    return height(node->left) - height(node->right);
}

void preOrder(Tree_Node* root, uint8_t operation, FILE* fp) {
    if (root != NULL) {
        if (operation == 0) {
            // Print all IDs
            printf("%X\n", root->id);
        } else if (operation == 1) {
            // Print Channel Names to CSV
            Sensor_Signal* curSensor = root->head; 
            while (curSensor != NULL) {
                fprintf(fp, ",%s", curSensor->name);
                curSensor = curSensor->next;
            }
        } else if (operation == 2) {
            // Print Channel Units to CSV
            Sensor_Signal* curSensor = root->head; 
            while (curSensor != NULL) {
                fprintf(fp, ",%s", curSensor->unit);
                curSensor = curSensor->next;
            }
        } else if (operation == 3) {
            // Print Channel Values to CSV
            Sensor_Signal* curSensor = root->head; 
            while (curSensor != NULL) {
                if (curSensor->value == 0) {
                    fprintf(fp, ",%i", 0);
                } else if (curSensor->value == ceilf(curSensor->value)) {
                    fprintf(fp, ",%i", (int) curSensor->value);
                } else if (curSensor->scalingMultiplier == (float) 0.1 || curSensor->scalingMultiplier == (float) 0.5) {
                    fprintf(fp, ",%.1f", curSensor->value);
                } else if (curSensor->scalingMultiplier == (float) 0.01) {
                    fprintf(fp, ",%.2f", curSensor->value);
                } else if (curSensor->scalingMultiplier == (float) 0.001) {
                    fprintf(fp, ",%.3f", curSensor->value);
                } else {
                    fprintf(fp, ",%f", curSensor->value);
                }
                curSensor = curSensor->next;
            }
        }

        preOrder(root->left, operation, fp);
        preOrder(root->right, operation, fp);

        if (operation == 4) {
            // Freeing nodes and sensors
            Sensor_Signal* curSensor = root->head; 
            Sensor_Signal* nextSensor;
            while (curSensor != NULL) {
                nextSensor = curSensor->next;
                free(curSensor->name);
                free(curSensor->unit);
                free(curSensor);
                curSensor = nextSensor;
            }
            free(root);
        }
    }
    return;
}

Tree_Node* rightRotate(Tree_Node* y) {
    Tree_Node* x = y->left;
    Tree_Node* T2 = x->right;
 
    x->right = y;
    y->left = T2;
 
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
 
    return x;
}
 
Tree_Node* leftRotate(Tree_Node* x) {
    Tree_Node* y = x->right;
    Tree_Node* T2 = y->left;
 
    y->left = x;
    x->right = T2;
 
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
 
    return y;
}

Tree_Node* newNode(uint16_t id, uint8_t type) {
    Tree_Node* newNode = (Tree_Node*) malloc(sizeof(Tree_Node));
    newNode->id = id;
    newNode->height = 1;
    newNode->type = type;
    newNode->data = 0;
    newNode->right = NULL;
    newNode->left = NULL;
    newNode->head = (Sensor_Signal*) malloc(sizeof(Sensor_Signal));
    return newNode;
}

Tree_Node* insertNode(Tree_Node* node, uint16_t id, uint8_t type) {
    if (node == NULL) {
        node = newNode(id, type);
        return node;
    }

    if (id < node->id) {
        node->left  = insertNode(node->left, id, type);
    } else if (id > node->id) {
        node->right = insertNode(node->right, id, type);
    } else {
        return node;
    }
 
    node->height = 1 + max(height(node->left), height(node->right));
 
    int balance = getBalance(node);

    // Left Left
    if (balance > 1 && id < node->left->id) {
        return rightRotate(node);
    }

    // Right Right
    if (balance < -1 && id > node->right->id) {
        return leftRotate(node);
    }

    // Left Right
    if (balance > 1 && id > node->left->id) {
        node->left =  leftRotate(node->left);
        return rightRotate(node);
    }

    // Right Left
    if (balance < -1 && id < node->right->id) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

Tree_Node* findNode(Tree_Node* head, int id) {
    while (1) {
        if (head == NULL) {
            return NULL;
        } else if (head->id == id) {
            return head;
        } else if (head->id > id) {
            head = head->left;
        } else {
            head = head->right;
        }
    }
}

// Parses the DBC file that is specified
Tree_Node* parseDBC(const char* fileName, Tree_Node* head) {
    char* str = (char*) calloc(256, sizeof(char));
    char canMessageSig[] = "BO_";
    char signalSig[] = "SG_";
    char* token;
    char s[9] = "|@(),[]\"";
    char d[2] = "x";
    Sensor_Signal* curSensor;
    FILE *fp;
    uint16_t id;
    
    // Open DBC file
    fp = fopen(fileName, "r");
    if (!fp) {
        free(str);
        printf("Fopen DBC File Failed: %s\n", fileName);
        return NULL;
    }

    // Parse the DBC file TODO ngl this section is so scuffed and i dont wanna look at it anymore. will have to change for signed vs unsigned. that check for making new sensors is scuffed too. future bored greyson problem maybe
    while (!feof(fp)) {
        fscanf(fp, "%s", str);
        if (!strcmp(str, canMessageSig)) {
            fscanf(fp, "%s", str);
            token = strtok(str, d); 
            token = strtok(NULL, d);
            id = strtol(token, NULL, 16);
            //printf("CAN ID: %X\n", id);
            head = insertNode(head, id, 0);
            curSensor = findNode(head, id)->head;
            curSensor->len = -1;
            fscanf(fp, "%s %s %s", str, str, str);
        } else if (!strcmp(str, signalSig)) {
            // TODO switch this to have better logic?
            if (curSensor->len != -1) {
                curSensor->next = (Sensor_Signal*) malloc(sizeof(Sensor_Signal));
                curSensor = curSensor->next;
            }
            fscanf(fp, "%s", str);
            curSensor->name = strdup(str);
            fscanf(fp, "%s %s", str, str);
            token = strtok(str, s);
            curSensor->startBit = atoi(token);
            token = strtok(NULL, s);
            curSensor->len = atoi(token);
            token = strtok(NULL, s);
            curSensor->sign = token[1] == '-' ? 0 : 1;
            fscanf(fp, "%s", str);
            token = strtok(str, s);
            curSensor->scalingMultiplier = atof(token);
            token = strtok(NULL, s);
            curSensor->scalingAddition = atof(token);
            fscanf(fp, "%s %s", str, str);
            token = strtok(str, s);
            curSensor->unit = strdup(str+1);
            curSensor->next = NULL;
            curSensor->value = 0;
            // Diag Message
            // printf("%s: start: %i, len: %i, sign: %i, mult: %f, add: %i, unit: %s\n", curSensor->name, curSensor->startBit, curSensor->len, curSensor->sign, curSensor->scalingMultiplier, curSensor->scalingAddition, curSensor->unit);
        }
    }

    // Free and close files
    free(str);

    if (fclose(fp)) {
        return NULL;
    }

    return head;
}

// Compresses the Hex data in the specified file and additionally creates a Brotli compressed file if specified
int compressData(const char* fileName, int brotliFlag) {

    uint8_t* idPtr = (uint8_t*) calloc(3, sizeof(uint8_t));
    uint8_t* canData = (uint8_t*) calloc(9, sizeof(uint8_t));
    uint8_t* timestamp = (uint8_t*) calloc(4, sizeof(uint8_t));
    uint8_t* fullMessage = (uint8_t*) calloc(14, sizeof(uint8_t));
    FILE *fp;
    FILE *newfp;
    uint64_t tempData = 0;
    Tree_Node* curNode;
    
    fp = fopen(fileName, "r");
    if (!fp) {
        // Free before return
        free(idPtr);
        free(canData);
        free(timestamp);
        free(fullMessage);
        printf("Fopen Log File Failed\n");
        return -1;
    }

    newfp = fopen(compressedFile, "w");
    if (!newfp) {
        // Close File and free before return
        free(idPtr);
        free(canData);
        free(timestamp);
        free(fullMessage);
        fclose(fp);
        printf("Fopen Compressed Log File Failed: %s\n", compressedFile);
        return -1;
    }

    //TODO use one largeeee buffer. use a ptr to keep track
    while (!feof(fp)) {
        // Find node for the cur ID
        fread(idPtr, 2, 1, fp);
        curNode = findNode(parent, (idPtr[0] << 8) + idPtr[1]);
        if (curNode == NULL) {
            // If ID isn't yet defined, create it and set current node to the newly created one
            parent = insertNode(parent, ((idPtr[0] << 8) + idPtr[1]), 0);      
            curNode = findNode(parent, ((idPtr[0] << 8) + idPtr[1]));
        }
        
        // Read the data and timestamp
        fread(canData, 8, 1, fp);
        fread(timestamp, 3, 1, fp);
        tempData = (((uint64_t) canData[0]) << 56) + (((uint64_t) canData[1]) << 48) + (((uint64_t) canData[2]) << 40) + (((uint64_t) canData[3]) << 32) + (((uint64_t) canData[4]) << 24) + (((uint64_t) canData[5]) << 16) + (((uint64_t) canData[6]) << 8) + ((uint64_t) canData[7]);
        // If the data has changed for this node, update it and write the data to the compressed hex file
        if (curNode->data != tempData) {
            memcpy(fullMessage, idPtr, 2);
            memcpy(fullMessage+2, canData, 8);
            memcpy(fullMessage+10, timestamp, 3);
            fwrite(fullMessage, 13, 1, newfp);
            curNode->data = tempData;
        }
    }

    // Cool stats for ya bud
    long originalLen = ftell(fp);
    long homebrewLen = ftell(newfp);
    printf("Homebrew Compress Size Saved: %ld\n", originalLen - homebrewLen);

    // Close Files and free like a good programmer
    free(idPtr);
    free(canData);
    free(timestamp);
    free(fullMessage);

    if (fclose(fp) || fclose(newfp)) {
        printf("Fclose Log and Compressed Log Failed\n");
        return -1;
    }

    // Brotli Compression
    if (brotliFlag) {

        // Command will brotli compress at level 5 and set a custom name for the output file
        char brotliCMD[530];
        memset(brotliCMD, 0, 530);
        strcpy(brotliCMD, "brotli -5 -o ");
        strcat(brotliCMD, brotliFile);
        strcat(brotliCMD, " ");
        strcat(brotliCMD, compressedFile);

        // Perform the Brotli Compression
        int ret = system(brotliCMD);
        if (ret != 0) {
            printf("Brotli CMD Failed. Check for existing file or too long of a function call\n");
            return -1;
        }

        // Checking length of Brotli Compressed file
        fp = fopen(brotliFile, "r");
        if (!fp) {
            printf("Fopen Brotli Failed\n");
            return -1;
        }

        // Cool stats for ya bud
        fseek(fp, 0, SEEK_END);
        long brotliLen = ftell(fp);
        printf("Brotli Compress Size Saved: %ld\n", homebrewLen - brotliLen);
        printf("Final Size Saved: %ld\n", originalLen - brotliLen);
        if (fclose(fp)) {
            printf("Fclose Brotli Failed\n");
            return -1;
        }
    }
    return 0;
}

// Parses the Hex data in the file given with the specified logging rate
int parseData(const char* fileName, int rate) {
    
    // Open hex and csv file
    FILE *hexfp, *csvfp;

    hexfp = fopen(fileName, "r");
    if (!hexfp) {
        printf("Fopen Hex Failed: %s\n", fileName);
        return -1;
    }

    char csvName[256];
    memset(csvName, 0, 256);
    strncpy(csvName, logFile, strlen(logFile)-4);
    strcat(csvName, ".csv");
    csvfp = fopen(csvName, "w");
    if (!csvfp) {
        printf("Fopen Csv Failed: %s\n", csvName);
        return -1;
    }

    // Set up header
    fprintf(csvfp, "Time");
    preOrder(parent, 1, csvfp);
    fprintf(csvfp, "\nS");
    preOrder(parent, 2, csvfp);
    fprintf(csvfp, "\n");
    
    // Read messages until timestamp changes
    uint8_t* idPtr = (uint8_t*) calloc(3, sizeof(uint8_t));
    uint8_t* canData = (uint8_t*) calloc(9, sizeof(uint8_t));
    uint8_t* timestamp = (uint8_t*) calloc(4, sizeof(uint8_t));
    uint32_t initialTimestamp = 0;
    uint32_t curTimestamp = 0;
    uint32_t reportRate = 1000 / rate;
    uint64_t tempData = 0;
    Tree_Node* curNode;
    Sensor_Signal* curSensor;

    // Grab initial timestamp for report rate purposes
    fread(idPtr, 2, 1, hexfp);
    fread(canData, 8, 1, hexfp);
    fread(timestamp, 3, 1, hexfp);
    initialTimestamp = ((timestamp[0] << 16) + (timestamp[1] << 8) + timestamp[2]);
    curTimestamp = initialTimestamp;

    // Return to beginning of file
    fseek(hexfp, 0, SEEK_SET);

    while (!feof(hexfp)) {
        // Grap ID of message and check if DBC defines it
        fread(idPtr, 2, 1, hexfp);
        curNode = findNode(parent, (idPtr[0] << 8) + idPtr[1]);
        if (curNode == NULL) {
            // ID is not defined by DBC
            fread(canData, 8, 1, hexfp);
            fread(timestamp, 3, 1, hexfp);
        } else {
            // ID is defined by DBC
            fread(canData, 8, 1, hexfp);
            fread(timestamp, 3, 1, hexfp);
            // Check if timestamp is transitioning
            while (curTimestamp != ((timestamp[0] << 16) + (timestamp[1] << 8) + timestamp[2])) {
                // Check if data should be written due to report rate
                if ((curTimestamp - initialTimestamp) % reportRate == 0) {
                    fprintf(csvfp, "%.3f", (curTimestamp - initialTimestamp) * 0.001);
                    preOrder(parent, 3, csvfp);
                    fprintf(csvfp, "\n");
                }
                curTimestamp += 1;
            }
            tempData = (((uint64_t) canData[0]) << 56) + (((uint64_t) canData[1]) << 48) + (((uint64_t) canData[2]) << 40) + (((uint64_t) canData[3]) << 32) + (((uint64_t) canData[4]) << 24) + (((uint64_t) canData[5]) << 16) + (((uint64_t) canData[6]) << 8) + ((uint64_t) canData[7]);
            curSensor = curNode->head;
            // Update data for each sensor in the message
            while (curSensor != NULL) {
                curSensor->value = (((tempData >> ((64 - (curSensor->len + curSensor->startBit)))) & ((0b1 << curSensor->len) - 0b1)) * curSensor->scalingMultiplier) + curSensor->scalingAddition;
                curSensor = curSensor->next;
            }
        }
    }

    // Close files and free
    free(idPtr);
    free(canData);
    free(timestamp);

    if (fclose(hexfp) || fclose(csvfp)) {
        return -1;
    }

    return 0;
}

// Updates a log created by B24 to the new format with proper analog logging
int updateLog(const char* fileName) {
    uint8_t* idPtr = (uint8_t*) calloc(3, sizeof(uint8_t));
    uint8_t* analogData = (uint8_t*) calloc(41, sizeof(uint8_t));
    uint8_t* canData = (uint8_t*) calloc(9, sizeof(uint8_t));
    uint8_t* timestamp = (uint8_t*) calloc(4, sizeof(uint8_t));
    uint8_t* fullMessage = (uint8_t*) calloc(14, sizeof(uint8_t));
    FILE *fp;
    FILE *newfp;
    Tree_Node* curNode;
    uint16_t id = 0;
    uint64_t analogMsg1 = 0;
    uint64_t analogMsg2 = 0;
    
    fp = fopen(fileName, "r");
    if (!fp) {
        // Free before return
        free(idPtr);
        free(analogData);
        free(canData);
        free(timestamp);
        free(fullMessage);
        printf("Fopen Log File Failed\n");
        return -1;
    }

    newfp = fopen(newLogFile, "w");
    if (!newfp) {
        // Close File and free before return
        free(idPtr);
        free(analogData);
        free(canData);
        free(timestamp);
        free(fullMessage);
        fclose(fp);
        printf("Fopen Compressed Log File Failed: %s\n", newLogFile);
        return -1;
    }

    while (!feof(fp)) {
        //TODO use one largeeee buffer. use a ptr to keep track
        fread(idPtr, 2, 1, fp);
        id = (idPtr[0] << 8) + idPtr[1];
        if (id > 8) {
            fread(canData, 8, 1, fp);
            fread(timestamp, 3, 1, fp);
            memcpy(fullMessage, idPtr, 2);
            memcpy(fullMessage+2, canData, 8);
            memcpy(fullMessage+10, timestamp, 3);
            fwrite(fullMessage, 13, 1, newfp);
        } else {
            // ID is Analog
            fread(analogData, 40, 1, fp);

            // Checking to make sure analog section has IDs 3->4->5->6->7->2 (end)
            if ((analogData[33] + analogData[34]) != 2) {
                printf("Log Update Aborted\nLast Id In Analog Zone: %x%x\nIf output is normal, Log was already updated\n", analogData[33], analogData[34]);
                fclose(fp);
                fclose(newfp);
                remove(newLogFile);
                return -1;
            }

            // First Analog Msg
            fullMessage[0] = 0;
            fullMessage[1] = 1;
            memset(fullMessage+2, 0, 4);
            memcpy(fullMessage+6, analogData+35, 2);
            memcpy(fullMessage+8, analogData, 2);
            memcpy(fullMessage+10, analogData+37, 3);
            fwrite(fullMessage, 13, 1, newfp);

            // Second Analog Msg
            fullMessage[0] = 0;
            fullMessage[1] = 2;
            memcpy(fullMessage+2, analogData+7, 2);
            memcpy(fullMessage+4, analogData+14, 2);
            memcpy(fullMessage+6, analogData+21, 2);
            memcpy(fullMessage+8, analogData+28, 2);
            memcpy(fullMessage+10, analogData+37, 3);
            fwrite(fullMessage, 13, 1, newfp);
        }
    }

    // Cool stats for ya bud
    long originalLen = ftell(fp);
    long newLen = ftell(newfp);
    printf("Log Update Size Saved: %ld\n", originalLen - newLen);

    // Close Files and free like a good programmer
    free(idPtr);
    free(analogData);
    free(canData);
    free(timestamp);
    free(fullMessage);

    if (fclose(fp) || fclose(newfp)) {
        printf("Fclose Log and Compressed Log Failed\n");
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {

    int ret;

    // Option flags
    int brotliSet = 0;
    int parseSet = 0;
    int alreadyBrotli = 0;

    // Report Rate of CSV. Defaults at 100Hz
    int reportRate = 100;

    // Clock vars for runtime tracking
    clock_t begin, curTime, prevTime, end;
    double timespent;
    begin = clock();
    curTime = begin;

    // Zeoring paths for DBC file, Log file, New (Converted) Log file, Compressed Log file, and Brotli Log file
    memset(dbcFile, 0, 256);
    memset(logFile, 0, 256);
    memset(newLogFile, 0, 256);
    memset(compressedFile, 0, 256);
    memset(brotliFile, 0, 256);
    
    // Argument Parsing
    if (argc == 1) {
        printf("You must pass arguments. Use -h to see argument options.\n");
        return 0;
    }

    // Help Argument
    if (strcmp(argv[1], "-h") == 0) {
        printf("ARGUMENTS: (order doesn't matter)\n-h --> Displays help page\n-c --> Compress input file with Homebrew and Brotli Compression algorithms\n-p --> Parses input file to a csv\n-a --> Compress input file with Homebrew and Brotli Compression algorithms and Parses input file to a csv\n-r __ --> Set report rate to __ (DEFAULT 100Hz)\nMust always input DBC file path and Log file path (unless -h is first argument)\n");
        return 0;
    }

    // Checking for -p, -c, -a, DBC File Path, and Log File Path
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            parseSet = 1;
        } else if (strcmp(argv[i], "-c") == 0) {
            brotliSet = 1;
        } else if (strcmp(argv[i], "-a") == 0) {
            parseSet = 1;
            brotliSet = 1;
        } else if (strcmp(argv[i], "-r") == 0) {
            if ((i+1) >= argc) {
                printf("No report rate entered\n");
                return 0;
            }
            reportRate = atoi(argv[i+1]);
            if (reportRate != 100 && reportRate != 1000) {
                printf("Invalid report rate entered: %s\nValid report rates are 100 and 1000\n", argv[i+1]);
                return 0;
            }
        } else if (strstr(argv[i], ".dbc") != NULL) {
            if (strlen(argv[i]) >= 236) {
                printf("DBC file path too long: %s\n Please limit path size to 236 chars.\n", argv[2]);
                return -1;
            }
            // TODO this memset is ensure inputting multiple DBC files don't mess up the program
            memset(dbcFile, 0, 256);
            strcpy(dbcFile, argv[i]);
        } else if (strstr(argv[i], ".TXT") != NULL) {
            if (strlen(argv[i]) >= 236) {
                printf("Log file path too long: %s\n Please limit path size to 236 chars.\n", argv[3]);
                return -1;
            }
            // TODO this memset is to ensure inputting multiple log files don't mess up the program
            memset(logFile, 0, 256);
            strcpy(logFile, argv[i]);
        }
    }

    // Default option if no options set
    if (!parseSet && !brotliSet) {
        parseSet = 1;
        brotliSet = 1;
    }

    // Must always provide a log file
    if (strcmp(logFile, "") == 0) {
        printf("No valid Log file input\n");
        return 0;
    }

    // Check that log file exists
    FILE *fp = fopen(logFile, "r");
    if (!fp) {
        printf("Log file: %s does not exist\n", logFile);
        return -1;
    } else {
        fclose(fp);
    }

    // Decompress log file if it is Broli compressed
    if (strstr(logFile, ".br") != NULL) {
        char brotliCMD[268];
        memset(brotliCMD, 0, 268);
        strcpy(brotliCMD, "brotli -d ");
        strcat(brotliCMD, logFile);

        // Set flag that log file came from a Brotli compressed file
        alreadyBrotli = 1;

        // Perform the Brotli Compression
        int ret = system(brotliCMD);
        if (ret != 0) {
            return -1;
        }

        // Temp assignment to shuffle around the file paths
        strncpy(brotliFile, logFile, strlen(logFile)-3);
        strcpy(logFile, brotliFile);
        memset(brotliFile, 0, 256);
        strcpy(brotliFile, logFile);
        strcat(brotliFile, ".br");

        // Get rid of flag for Brotli compression
        if (brotliSet) {
            printf("Inputted file was already Brotli compressed. No need to compress with Brotli again\n");
            brotliSet = 0;
        }
    } else {
        // Set rest of file paths TODO have base name be a hash of metadata?
        memset(newLogFile, 0, 256);
        memset(compressedFile, 0, 256);
        memset(brotliFile, 0, 256);

        strncpy(newLogFile, logFile, strlen(logFile)-4);
        strcat(newLogFile, "New.TXT");
        strncpy(compressedFile, logFile, strlen(logFile)-4);
        strcat(compressedFile, "Compressed.TXT");
        strcpy(brotliFile, logFile);
        strcat(brotliFile, ".br");
    }

    // Must provide a DBC file if parsing
    if (strcmp(dbcFile, "") == 0 && parseSet) {
        printf("No valid DBC file input\n");
        return 0;
    }

    // Check that DBC file exists
    fp = fopen(dbcFile, "r");
    if (!fp && parseSet) {
        printf("DBC file: %s does not exist\n", dbcFile);
        return -1;
    } else {
        fclose(fp);
    }

    // Tell user what is occuring
    if (parseSet && !brotliSet) {
        printf("Parsing\nDBC file: %s\nLog file: %s\nLogging Rate: %i\n", dbcFile, logFile, reportRate);
    } else if (!parseSet && brotliSet) {
        printf("Compressing\nLog file: %s\n", logFile);
    } else {
        printf("Compressing and Parsing\nDBC file: %s\nLog file: %s\nLogging Rate: %i\n", dbcFile, logFile, reportRate);
    }

    // Update old log to new Analog format, unless log came from Brotli file
    ret = alreadyBrotli ? 1 : updateLog(logFile);
    if (ret == 0) {
        printf("Updated Log\n");
    }

    // Compress log with homebrew method (and Brotli if -c or -a), unless log came from Brotli file
    ret = alreadyBrotli ? 0 : compressData(ret == 0 ? newLogFile : logFile, brotliSet);
    if (ret != 0) {
        printf("Compression failed\n");
        // Delete intermediate files
        remove(compressedFile);
        remove(newLogFile);
        // Free tree structs
        preOrder(parent, 4, NULL);
        return ret;
    }

    // Parse homebrewed, compressed hex file into csv (if -p or -a), or log file if log came from Brotli file
    if (parseSet) {

        // Reset DBC tree from possible null sensors created during compression
        preOrder(parent, 4, NULL);
        parent = NULL;

        // Parse DBC
        parent = parseDBC(dbcFile, parent);
        if (parent == NULL) {
            printf("Parsing the DBC failed\n");
            // Delete intermediate files
            remove(compressedFile);
            remove(newLogFile);
            // Free tree structs
            preOrder(parent, 4, NULL);
            return -1;
        }

        ret = parseData(alreadyBrotli ? logFile : compressedFile, reportRate);
        if (ret != 0) {
            printf("Parsing data failed\n");
            // Delete intermediate files
            remove(compressedFile);
            remove(newLogFile);
            // Free tree structs
            preOrder(parent, 4, NULL);
            return ret;
        }
        // Runtime for Data Parsing
        prevTime = curTime;
        curTime = clock();
        timespent = (double)(curTime - prevTime) / CLOCKS_PER_SEC;
        printf("Parse Runtime: %f\n", timespent);
    }

    // Delete intermediate files
    remove(compressedFile);
    remove(newLogFile);

    // Free tree structs
    preOrder(parent, 4, NULL);

    // Total runtime
    end = clock();
    timespent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Total Runtime: %f\n", timespent);

    return 0;
}