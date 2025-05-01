#!/bin/bash

echo "Setting up Deribit Trading System on Ubuntu/Debian..."

# Update system and install dependencies
echo "Updating system and installing dependencies..."
sudo apt update && sudo apt upgrade -y
sudo apt install -y cmake gcc g++ libboost-all-dev libssl-dev git libreadline-dev

# Build the project
echo "Building the project..."
mkdir -p build && cd build
cmake .. -Wno-dev
cmake --build .

# Notify user
echo "Setup complete. You can now run './deribit_trader' to start the application."

./deribit_trader