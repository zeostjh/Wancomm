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

# PortAudio for audio I/O
find_path(PORTAUDIO_INCLUDE_DIR portaudio.h)
find_library(PORTAUDIO_LIBRARY NAMES portaudio)

# Opus for audio codec
find_path(OPUS_INCLUDE_DIR opus/opus.h)
find_library(OPUS_LIBRARY NAMES opus)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/client/include
    ${PORTAUDIO_INCLUDE_DIR}
    ${OPUS_INCLUDE_DIR}
)

# Shared source files (used by both server and client)
set(SHARED_SOURCES
    ${CMAKE_SOURCE_DIR}/src/AudioCapture.cpp
    ${CMAKE_SOURCE_DIR}/src/AudioPlayback.cpp
    ${CMAKE_SOURCE_DIR}/src/AudioCodec.cpp
    ${CMAKE_SOURCE_DIR}/src/NetworkManager.cpp
    ${CMAKE_SOURCE_DIR}/src/VoiceChatClient.cpp
)

# Client-specific source files
set(CLIENT_SOURCES
    client/src/main_client.cpp
    client/src/ClientMainWindow.cpp
    client/include/ClientMainWindow.h
    ${SHARED_SOURCES}
)

# Build client executable
add_executable(voicechat-client ${CLIENT_SOURCES})
target_link_libraries(voicechat-client
    Qt6::Core
    Qt6::Widgets
    ${PORTAUDIO_LIBRARY}
    ${OPUS_LIBRARY}
    Threads::Threads
)

# Installation
install(TARGETS voicechat-client DESTINATION bin)
