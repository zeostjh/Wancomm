cmake_minimum_required(VERSION 3.15)
project(VoiceChat VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt setup
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt")

# Find packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/server/include
)

# Shared source files (used by both server and client)
set(SHARED_SOURCES
    ${CMAKE_SOURCE_DIR}/src/AudioCapture.cpp
    ${CMAKE_SOURCE_DIR}/src/AudioPlayback.cpp
    ${CMAKE_SOURCE_DIR}/src/AudioCodec.cpp
    ${CMAKE_SOURCE_DIR}/src/NetworkManager.cpp
)

# Server-specific source files
set(SERVER_SOURCES
    server/src/main_server.cpp
    server/src/ServerMainWindow.cpp
    server/include/ServerMainWindow.h
    server/src/UserManager.cpp
    server/include/UserManager.h
    ${SHARED_SOURCES}
)

# Build server executable
add_executable(voicechat-server ${SERVER_SOURCES})
target_link_libraries(voicechat-server
    Qt6::Core
    Qt6::Widgets
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

# Installation
install(TARGETS voicechat-server DESTINATION bin)
