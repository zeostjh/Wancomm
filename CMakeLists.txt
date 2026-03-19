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

# Options
option(BUILD_SERVER "Build server application" ON)
option(BUILD_CLIENT "Build client application" ON)

# Find packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# PortAudio for audio I/O
find_path(PORTAUDIO_INCLUDE_DIR portaudio.h)
find_library(PORTAUDIO_LIBRARY NAMES portaudio)

# Opus for audio codec
find_path(OPUS_INCLUDE_DIR opus/opus.h)
find_library(OPUS_LIBRARY NAMES opus)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${PORTAUDIO_INCLUDE_DIR}
    ${OPUS_INCLUDE_DIR}
)

# Shared source files (used by both server and client)
set(SHARED_SOURCES
    src/NetworkManager.cpp
)

# Audio-related sources (client only)
set(AUDIO_SOURCES
    src/AudioCapture.cpp
    src/AudioPlayback.cpp
    src/AudioCodec.cpp
    src/AudioDevice.cpp
)

# Build server if requested
if(BUILD_SERVER)
    set(SERVER_SOURCES
        server/src/main_server.cpp
        server/src/ServerMainWindow.cpp
        server/include/ServerMainWindow.h
        server/src/UserManager.cpp
        server/include/UserManager.h
        ${SHARED_SOURCES}
    )
    
    include_directories(${CMAKE_SOURCE_DIR}/server/include)
    
    add_executable(voicechat-server ${SERVER_SOURCES})
    target_link_libraries(voicechat-server
        Qt6::Core
        Qt6::Widgets
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
    )
    
    install(TARGETS voicechat-server DESTINATION bin)
endif()

# Build client if requested
if(BUILD_CLIENT)
    set(CLIENT_SOURCES
        client/src/main_client.cpp
        client/src/ClientMainWindow.cpp
        client/include/ClientMainWindow.h
        src/VoiceChatClient.cpp
        ${SHARED_SOURCES}
        ${AUDIO_SOURCES}
    )
    
    include_directories(${CMAKE_SOURCE_DIR}/client/include)
    
    add_executable(voicechat-client ${CLIENT_SOURCES})
    target_link_libraries(voicechat-client
        Qt6::Core
        Qt6::Widgets
        ${PORTAUDIO_LIBRARY}
        ${OPUS_LIBRARY}
        Threads::Threads
    )
    
    install(TARGETS voicechat-client DESTINATION bin)
endif()
