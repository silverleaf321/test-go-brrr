#!/bin/bash

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

# Install Brotli using Homebrew
if ! command -v brotli &> /dev/null; then
    echo "Installing Brotli..."
    brew install brotli
fi

echo "Installation complete!"