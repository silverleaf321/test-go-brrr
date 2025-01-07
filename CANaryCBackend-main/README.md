# CANaryCBackend
C implementation for converting CANary logs in Hex to CSV and i2. Also compresses the custom Hex files using a homebrewed algorithm and Brotli compression

ARGUMENTS: (order doesn't matter)
-h --> Displays help page
-c --> Compress input file with Homebrew and Brotli Compression algorithms
-p --> Parses input file to a csv
-a --> Compress input file with Homebrew and Brotli Compression algorithms and Parses input file to a csv (DEFAULT IF NO OPTIONS SELECTED)
-r __ --> Set report rate to __ (DEFAULT 100Hz) (ONLY 1kHz AND 100Hz ALLOWED)
Must always input a Log file path (unless -h is first argument)
Must always input a DBC file path if parsing to csv

11/20/24 - Brandon Kim

-Fixed case sensitivity of file extensions
-Integrated Brotli configured for Windows machines (?)
-Still needs testing

7/24/24

Optimizations:
-

New Features:
-Passing in a Brotli compressed file will automatically uncompress if selected to parse it

Bug Fixes:
-Compress step added NULL channels to csv log

Notes:
-Should add an verbose argument so that user can choose to see time and data stats
-If multiple log files or DBC files are submitted, the last one will be selected for each type
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-End of data may be cut off due to compressed file 

7/23/24

Optimizations:
-

New Features:
-Program will automatically update old logs to the new format
-File output names more readable
-Intermediate files are deleted
-Compressing no longer requires a DBC file
-More comments :)
-Ensures DBC and Log files actually exist if such args are passed

Bug Fixes:
-

Notes:
-Should add an verbose argument so that user can choose to see time and data stats
-Parsing a Brotli compressed file not working
-If multiple log files or DBC files are submitted, the last one will be selected for each type
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Selected report rates untested for values other than 1000, 100 (values other than these aren't valid)
-End of data may be cut off due to compressed file 

7/22/24

Optimizations:
-Compress feature will compress messages not defined in DBC

New Features:
-Log converter from B24 to B25

Bug Fixes:
-

Notes:
-To fix below two points, need to detect if file is old and update if so
-Trying to update a new style log will abort operation
-Must still pass in a DBC w/ old analog when working w/ old analog signals or compressor will break
-Parsing a Brotli compressed file not working
-If multiple log files or DBC files are submitted, the last one will be selected for each type
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Selected report rates untested for values other than 1000, 100 (values other than these aren't valid)
-End of data may be cut off due to compressed file 

7/18/24

Optimizations:
-

New Features:
-New arguments to select which functions user wants to perform

Bug Fixes:
-

Notes:
-Parsing a Brotli compressed file not working
-If multiple log files or DBC files are submitted, the last one will be selected for each type
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Selected report rates untested for values other than 1000, 100 (values other than these aren't valid)
-NEED TO THINK AB WHERE FILES WILL BE STORED TO AND TAKEN FROM
-End of data may be cut off due to compressed file 
-Create log converter for B24 to B25 format

7/17/24

Optimizations:
-

New Features:
-

Bug Fixes:
-Report Rate feature input always invalid

Notes:
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Selected report rates untested for values other than 1000, 100 (values other than these aren't valid)
-When CANary HW changes, will need to fix the way analog is handled (prob after summer)
-NEED TO THINK AB WHERE FILES WILL BE STORED TO AND TAKEN FROM
-End of data may be cut off due to compressed file 

7/16/24

Optimizations:
-

New Features:
-Report rate passed in as argument

Bug Fixes:
-

Notes:
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Selected report rates untested for values other than 1000, 100 (values other than these aren't valid)
-When CANary HW changes, will need to fix the way analog is handled (prob after summer)
-NEED TO THINK AB WHERE FILES WILL BE STORED TO AND TAKEN FROM
-End of data may be cut off due to compressed file 

7/14/24

Optimizations:
-

New Features:
-File path length checks and err messages
-Better comments for file readability

Bug Fixes:
-CAN messages weren't triggering new csv line with rate other than 1000

Notes:
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Selected report rates untested for values other than 1000, 100
-When CANary HW changes, will need to fix the way analog is handled (prob after summer)
-NEED TO THINK AB WHERE FILES WILL BE STORED TO AND TAKEN FROM
-End of data may be cut off due to compressed file 

7/13/24

Optimizations:
-

New Features:
-Now requires DBC file to be entered as an argument
-Some new error messages
-New Runtime stats for compressing and parsing

Bug Fixes:
-File names can be sorta up to 256 chars

Notes:
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Selected report rates untested for values other than 1000, 100
-When CANary HW changes, will need to fix the way analog is handled (prob after summer)
-NEED TO MAKE ERROR HANDLING BETTER
-NEED TO ENSURE THAT FILE NAMES ARE SMALL ENOUGH
-NEED TO THINK AB WHERE FILES WILL BE STORED TO AND TAKEN FROM

7/12/24

Optimizations:
-

New Features:
-

Bug Fixes:
-File names are (sorta) capped at 256 chars

Notes:
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Selected report rates untested for values other than 1000, 100
-When CANary HW changes, will need to fix the way analog is handled (prob after summer)
-NEED TO MAKE ERROR HANDLING BETTER
-NEED TO ENSURE THAT FILE NAMES ARE SMALL ENOUGH
-NEED TO THINK AB WHERE FILES WILL BE STORED TO AND TAKEN FROM

7/11/24

Optimizations:
-

New Features:
-Can be run from cmd line

Notes:
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Selected report rates untested for values other than 1000, 100
-When CANary HW changes, will need to fix the way analog is handled (prob after summer)

7/7/24

Optimizations:
-

New Features:
-Dynamic file naming
-Runtime stats

Notes:
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Selected report rates untested for values other than 1000, 100
-Memory leaks not checked yet
-When CANary HW changes, will need to fix the way analog is handled (prob after summer)

7/5/24

Optimizations:
-

New Features:
-Brotli compression added to main compression function
-Dynamic file naming (Partial)

Notes:
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Selected report rates untested for values other than 1000, 100
-Memory leaks not checked yet
-When CANary HW changes, will need to fix the way analog is handled (prob after summer)
-Some file names are hardcoded

6/22/24

Optimizations:
-

New Features:
-Selectable report rates

Notes:
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Selected report rates untested for values other than 1000, 100
-Memory leaks not checked yet
-When CANary HW changes, will need to fix the way analog is handled (prob after summer)
-File names are hardcoded

6/21/24

Optimizations:
-Removed trailing 0's for csv size reasons

New Features:
-Freed structs after program has run

Notes:
-Optimizing trailing 0's wont work for scaling values that aren't 0.1, 0.5, 0.01, 0.001 or wild number like 10.1
-Memory leaks not checked yet
-When CANary HW changes, will need to fix the way analog is handled (prob after summer)
-File names are hardcoded

6/20/24

New Features:
-Parsing CAN and Analog to CSV fully working

Optimizations:
-Removed trailing 0's from signals whose value is 0

Notes:
-File names are hardcoded
-Need to further optimize trailing 0's for size reasons
-Need to fully free structs after parsing finished
-When CANary HW changes, will need to fix the way analog is handled (prob after summer)
