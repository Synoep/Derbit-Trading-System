cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)

# Set project details
project(DeribitTrader VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include external dependencies
include(FetchContent)

# Fetch websocketpp library
FetchContent_Declare(
    websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG master
)
FetchContent_MakeAvailable(websocketpp)

# Fetch fmt library
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG e69e5f977d458f2650bb346dadf2ad30c5320281
)
FetchContent_MakeAvailable(fmt)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Add executable and its source files
add_executable(deribit_trader 
    src/authentication/password.cpp
    src/api/api.cpp
    src/utils/utils.cpp
    src/main.cpp
    src/websocket/websocket_client.cpp
    src/latency/tracker.cpp
)

# Add include directories
target_include_directories(deribit_trader 
    PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
        ${websocketpp_SOURCE_DIR}
        ${fmt_SOURCE_DIR}
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include # Include your project headers
)

# Link required libraries
target_link_libraries(deribit_trader 
    PRIVATE 
        Boost::system
        Boost::thread
        OpenSSL::SSL
        OpenSSL::Crypto
        fmt::fmt
        readline
)

set_target_properties(deribit_trader PROPERTIES
    LINK_FLAGS "-Wl,--export-dynamic"
)

# Debugging information
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
