# Deribit Trading System

## Overview

Deribit Trading System is a high-performance order execution and management system that operates through a command-line interface. It uses a WebSocket client to connect to the Deribit TESTNET and manage trading portfolios with advanced functionality.

[![image](https://github.com/user-attachments/assets/4cc060ac-5168-40a2-a2a0-9639cc7855fb)](https://youtu.be/r6a5EGWrwl8?si=QQchaGp8tsG5MC1d)

## Features

- WebSocket-based trading interface
- Command-line trading operations
- Support for various order types
- API authentication
- Open orders management
- Order modification and cancellation

## Tech Stack

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white)
![WebSocket++](https://img.shields.io/badge/websocketpp-3F54A3?style=for-the-badge&logo=websocket&logoColor=white)
![OpenSSL](https://img.shields.io/badge/OpenSSL-721817?style=for-the-badge&logo=openssl&logoColor=white)
![JSON](https://img.shields.io/badge/json%20library-00599C?style=for-the-badge&logo=json&logoColor=white)
![Boost](https://img.shields.io/badge/Boost%20C++-f34b7d?style=for-the-badge&logo=boost&logoColor=white)
![Readline](https://img.shields.io/badge/Readline-4A90E2?style=for-the-badge&logo=gnu&logoColor=white)


## Prerequisites

### System Requirements

- C++ Compiler (g++)
- CMake
- Boost Libraries
- OpenSSL
- Git

### Fast setup
To quickly configure your system and set up the project, please go ahead and execute the appropriate setup script for your operating system as detailed below.

- NOTE: Please make sure you are in the repository's root directory before running the script.

##### macOS:
Run the following commands to execute the setup script:
```sh
chmod +x ./scripts/setup_mac.sh
./scripts/setup_mac.sh
```

##### Linux:
Run the following commands to execute the setup script:
```sh
chmod +x ./scripts/setup_linux.sh
./scripts/setup_linux.sh
```

#### Windows
For Windows, use the PowerShell script as follows:
```ps1
Set-ExecutionPolicy Bypass -Scope Process -Force
.\scripts\setup_windows.ps1
```
This will install the necessary dependencies, build the project, and prepare it for use.


### Installation Guide

#### Mac OS

1. Install Homebrew (if not already installed):
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

2. Install dependencies:
```bash
brew install cmake gcc boost openssl git readline
```

#### Linux (Ubuntu/Debian)

1. Update package list:
```bash
sudo apt update
```

2. Install dependencies:
```bash
sudo apt install cmake gcc libboost-all-dev libssl-dev git libreadline-dev
```

#### Windows

1. Download and Install:
   - [CMake](https://cmake.org/download/)
   - [Git](https://git-scm.com/download/win)
   - [Visual Studio Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/)
   - [Boost Binaries](https://www.boost.org/users/download/)
   - [OpenSSL](https://slproweb.com/products/Win32OpenSSL.html)

## Project Setup

1. Clone the Repository:
```bash
git clone https://github.com/Venkatesan-M/deribit-trading-system.git
cd deribit-trading-system
```

2. Create Build Directory:
```bash
mkdir build
cd build
```

3. Build the Project:
```bash
cmake .. -Wno-dev
cmake --build .
```

4. Run the Application:
```bash
./deribit_trader
```

## Disclaimer

This is a trading system for educational and testing purposes. Always use caution and understand the risks involved in cryptocurrency trading.

## Getting Started with Deribit

### Account Setup

1. Create an account on [Deribit Testnet](https://test.deribit.com)

![image](https://github.com/user-attachments/assets/cffeae56-da67-4aa5-a7cc-ab4f5db13b88)

2. Generate API Keys:
   - Navigate to Account Settings
   - Create API Key
   - Set permissions (read/read_write)
   - Save Client ID and Client Secret securely

### Application Usage

#### Basic Commands

- `help / man`: Show all supported commands
- `quit / exit`: Close WebSocket connections and exit
- `close <id> [code] [reason]` : Closes the connection with the given id; optional: specify exit code and/or reason
- `connect <URI>` : Creates a connection with the given URI
- `show <id>`: Get connection metadata
- `send <id> msg`: Send message to specific connection
- `show_messages <id>`: View message exchanges
- `send <id> <message>`: Sends the message to the specified connection
- `view_subscriptions`: Displays the list of subscribed symbols to stream continuous orderbook updates
- `view_stream`: Displays the stream continuous orderbook updates subscribed symbols
- `latency_report` : Generates a latency report of the current session
- `reset_report` : Delete's the data of the latency report of the current session

#### Deribit API Commands

#### Connection and Authentication
1. Connect to Testnet:
Creates a new connection to the Deribit testnet website
```sh
Deribit connect
```

```bash
connect wss://test.deribit.com/ws/api/v2
```

2. Authenticate:
Sends the authorization message to retrieve the access token; optional: use -s to remember the token for the session
```bash
Deribit <id> authorize <connection_id> <client_id> <client_secret> [-s]
```

#### Order Management

1. Buy Order:
 Places a buy order for the specified instrument
```bash
Deribit <id> buy <instrument> <transaction_name>
```

2. Sell Order:
 Places a sell order for the specified instrument
```bash
Deribit <id> sell <instrument> <transaction_name>
```

3. Modify Order:
 Modifies the price or amount of an active order
```bash
Deribit <id> modify <order_id>
```

4. Cancel Order:
 Cancels the specified order
```bash
Deribit <id> cancel <order_id>
```
Cancell all orders
```sh
Deribit <id> cancel_all
```
#### Information Retrieval

1. Get Open Orders:
 Fetches all open orders with optional filters
```bash
Deribit <id> get_open_orders [<currency>] [<instrument>] [<label>]
```
2. View Positions:
 Fetches all your current open positions; optional: use options to be specific
```bash
Deribit <id> positions [currency] [kind]
```
3. Get OrderBook:
Fetches all current buy and sell orders for the specified instrument; optional: specify depth of search
```bash
Deribit <id> orderbook <instrument> [<depth>]
```

#### Symbol Subscription
1. Subscribe to a symbol:
Subscribes to that symbol to stream continuous orderbook updates
```sh
Deribit <id> subscribe [symbol]
```
2. Unsubscribe to a symbol:
Unsubscribes to that symbol to stream continuous orderbook updates
```sh
Deribit <id> unsubscribe [symbol]
```
3. Unsubscribe to all symbol:
Unsubscribes to all symbols that have been subscribed to stream real time data
```sh
Deribit <id> unsubscribe_all
```

## Supported Order Types

| **Order Type**      | **Flag**        |
|---------------------|-----------------|
| Limit               | `limit`         |
| Stop Limit          | `stop_limit`    |
| Take Limit          | `take_limit`    |
| Market              | `market`        |
| Stop Market         | `stop_market`   |
| Take Market         | `take_market`   |
| Market Limit        | `market_limit`  |
| Trailing Stop       | `trailing_stop` |




## Time in Force Options

| **Option**                | **Flag**                |
|---------------------------|-------------------------|
| Good Till Cancelled       | `good_til_cancelled`    |
| Good Till Day             | `good_til_day`         |
| Fill or Kill              | `fill_or_kill`          |
| Immediate or Cancel       | `immediate_or_cancel`   |


## Project Structure

```sh

├── include # Header files
│   ├── api
│   │   └── api.h
│   ├── authentication
│   │   └── password.h
│   ├── json
│   │   └── json.hpp
│   ├── utils
│   │   └── utils.h 
│   └── latency
│       └── tracker.h
│   └── websocket
│       └── websocket_client.h
├── README.md
├── setup.sh
└── src
    ├── main.cpp # main function (deribit trader)
    ├── api
    │   └── api.cpp # Implementation Deribit API's Functionality
    ├── authentication
    │   └── password.cpp # Authentication and Session Management Functionality
    ├── latency
    │   └── tracker.cpp # Latenct tracking logic
    ├── utils
    │   └── utils.cpp # Helper functions and utils
    └── websocket
        └── websocket_client.cpp # websocketpp and WebSocket endpoint implementation
```

## Development

[![image](https://github.com/user-attachments/assets/a5d8aee6-9b40-4b07-ac2f-852b595fe12e)](https://youtu.be/c6sYTvpzR7s?si=e7Lfe-rqktacIdq_)

1. you are free to develop more feature on this, just refer the new [DERIBIT API DOCS](https://docs.deribit.com/)
2. Test the API Call on [Deribit API Console](https://test.deribit.com/api_console)

## Contribution

Contributions are welcome! Please follow these steps:
1. Fork the repository
2. Create a new branch
3. Make your changes
4. Submit a pull request

## License

MIT License

Copyright (c) 2025 Venkatesan M
