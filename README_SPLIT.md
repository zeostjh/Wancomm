# VoiceChat - Split Server/Client Architecture

VoiceChat is now split into two separate applications:

## Applications

### 1. VoiceChat Server (`voicechat-server`)
The server application provides:
- **User Management**: Admin interface to create and manage users with passwords
- **Authentication**: Secure user authentication before allowing connections
- **Audio Routing**: Routes audio packets between connected clients
- **Client Monitoring**: Real-time view of connected clients and their status

### 2. VoiceChat Client (`voicechat-client`)
The client application provides:
- **4 Talk Buttons**: Push-to-talk buttons for 4 independent channels
- **4 Listen Toggle Buttons**: Enable/disable listening on each of the 4 channels
- **Audio Level Meters**: Real-time input and output audio level visualization
- **Network Settings**: Configure server address and port
- **User Settings**: Login with username and password
- **Audio Device Selection**: Choose input/output audio devices

## Building

### Prerequisites
- CMake 3.15 or higher
- Qt6 (Widgets)
- PortAudio (for audio I/O)
- Opus (for audio codec)
- OpenSSL (for password hashing)
- C++17 compatible compiler

### macOS Installation
```bash
brew install qt6 portaudio opus openssl cmake
```

### Build Both Applications
```bash
./build.sh
```

This will build both `voicechat-server` and `voicechat-client` in the `build/` directory.

### Build Only Server
```bash
mkdir -p build && cd build
cmake -DBUILD_CLIENT=OFF ..
make -j$(nproc)
```

### Build Only Client
```bash
mkdir -p build && cd build
cmake -DBUILD_SERVER=OFF ..
make -j$(nproc)
```

## Running

### Server Application

1. Start the server:
```bash
./build/voicechat-server
```

2. Configure server settings:
   - Set bind address (default: 0.0.0.0)
   - Set port (default: 5000)
   - Click "Start Server"

3. Create users:
   - Enter username and password
   - Click "Create User"
   - Users are saved to `users.dat` file

4. Monitor connected clients in real-time

### Client Application

1. Start the client:
```bash
./build/voicechat-client
```

2. Login dialog will appear:
   - Enter your username (created by server admin)
   - Enter your password
   - Enter server address (e.g., localhost or server IP)
   - Enter port (default: 5000)
   - Click "Login"

3. Once connected:
   - **Talk**: Press and hold any of the 4 TALK buttons to transmit on that channel
   - **Listen**: Toggle the Listen checkbox for each channel to receive audio
   - **Meters**: Watch input/output levels in real-time
   - **Settings**: Change audio devices anytime

## Features

### Server Features
- ✅ User database with password authentication
- ✅ Create/delete users with admin interface
- ✅ Real-time client connection monitoring
- ✅ Multi-channel audio routing (4 channels)
- ✅ Automatic client timeout/cleanup
- ✅ User activity tracking (last login, etc.)

### Client Features
- ✅ 4 independent talk channels
- ✅ 4 independent listen channels
- ✅ Push-to-talk functionality
- ✅ Real-time audio level meters
- ✅ Audio device selection
- ✅ Secure authentication
- ✅ Connection status monitoring
- ✅ Automatic reconnection handling

## Architecture

### Communication Flow
1. Client sends authentication request to server
2. Server validates credentials using UserManager
3. Server sends authentication response
4. If authenticated, client can send/receive audio packets
5. Server routes audio packets between clients based on channels

### Channel System
- **Channel 0-3**: Four independent audio channels
- Clients can talk on any channel (one at a time per client)
- Clients can listen to multiple channels simultaneously
- Server routes audio only to clients listening on the same channel

### Security
- Passwords are hashed using SHA-256
- Authentication required before any audio transmission
- User database stored securely on server

## File Structure
```
VoiceChat/
├── server/
│   ├── src/
│   │   ├── main_server.cpp
│   │   ├── ServerMainWindow.cpp
│   │   └── UserManager.cpp
│   └── include/
│       ├── ServerMainWindow.h
│       └── UserManager.h
├── client/
│   ├── src/
│   │   ├── main_client.cpp
│   │   └── ClientMainWindow.cpp
│   └── include/
│       └── ClientMainWindow.h
├── src/ (shared)
│   ├── AudioCapture.cpp
│   ├── AudioPlayback.cpp
│   ├── AudioCodec.cpp
│   ├── NetworkManager.cpp
│   └── VoiceChatClient.cpp
└── include/ (shared)
    ├── AudioCapture.h
    ├── AudioPlayback.h
    ├── AudioCodec.h
    ├── AudioDevice.h
    ├── NetworkManager.h
    └── VoiceChatClient.h
```

## Troubleshooting

### Server won't start
- Check if port is already in use: `lsof -i :5000`
- Verify OpenSSL is installed: `brew list openssl`

### Client can't connect
- Verify server is running and started
- Check server address and port
- Verify user credentials with server admin
- Check firewall settings

### No audio
- Verify audio devices are selected correctly
- Check audio permissions in macOS System Preferences
- Test with different audio devices
- Check level meters for activity

## License
See LICENSE file for details.
