#!/bin/bash

echo "Setting up Deribit Trading System on macOS..."

# Install Homebrew if not installed
if ! command -v brew &> /dev/null; then
    echo "Homebrew not found, installing..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

# Install dependencies
echo "Installing dependencies..."
brew install cmake gcc boost openssl git readline

# Build the project
echo "Building the project..."
mkdir -p build && cd build
cmake .. -Wno-dev
cmake --build .

# Notify user
echo "Setup complete. You can now run './deribit_trader' to start the application."

./deribit_trader