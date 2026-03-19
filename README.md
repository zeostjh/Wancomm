# VoiceChat - Professional Intercom System

A professional real-time voice communication system inspired by ClearCom HelixNet and FreeSpeak beltpack systems. Features a sleek dark mode interface with intuitive Talk/Listen controls and support for up to 10 simultaneous users.

## Features

### 🎨 Professional ClearCom-Inspired Interface
- **Dark mode design** with professional orange accent colors
- **Large, tactile buttons** - Talk (red) and Listen (green) controls
- **Live LED indicators** showing Talk and Listen states
- **Audio level meters** with color-coded TX/RX visualization
- **Compact, focused layout** optimized for production environments

### 👥 Multi-User Support
- **Up to 10 simultaneous users** with custom usernames
- **Live user list** showing who's connected and their status
- **Real-time indicators** showing who's talking (🔴) and listening (🟢)
- **User authentication** via login dialog on startup

### 📡 4-Channel Intercom System
- **4 independent channels** for different production groups:
  - 🔴 Channel 1 - PRODUCTION
  - 🟢 Channel 2 - STAGE
  - 🔵 Channel 3 - TECH
  - 🟡 Channel 4 - DIRECTOR
- **Channel-isolated audio** - only users on the same channel hear each other

### 🎙️ Professional Audio Controls
- **Press-to-Talk button** - Hold to transmit (ClearCom style)
- **Listen toggle** - Turn listening on/off as needed
- **Visual feedback** - LED indicators for all states
- **Audio level monitoring** - Real-time TX/RX meters

### 🖥️ Server/Client Architecture
- **Dedicated server mode** for network hub
- **Multi-client support** with automatic user discovery
- **Heartbeat system** for connection monitoring
- **Automatic cleanup** of disconnected clients

## Installation

### Prerequisites
- CMake 3.10 or higher
- Qt 6.x
- PortAudio
- Opus audio codec library

### macOS
```bash
brew install cmake qt portaudio opus
```

### Build
```bash
./build.sh
```

Or manually:
```bash
mkdir -p build
cd build
cmake ..
make
```

## Usage

### Starting the Application

1. **Run the application:**
   ```bash
   ./build/voicechat
   ```

2. **Login Dialog** will appear:
   - Enter your **username** (up to 20 characters)
   - Select **Server Mode** or **Client Mode**
   - Enter **Server IP** (for clients)
   - Enter **Port** (default: 5000)

### Server Mode

1. Select "Server Mode" in the login dialog
2. Click "START SERVER"
3. Server will listen for incoming client connections
4. Monitor connected users in the users list

### Client Mode

1. Select "Client Mode" in the login dialog
2. Enter the server's IP address
3. Enter your username
4. Click "CONNECT TO SERVER"
5. You'll see other connected users appear in the list

### Using the Intercom

#### Talk Button (Red)
- **Press and hold** to transmit audio
- **Release** to stop transmitting
- LED indicator turns green when talking
- Only users on your channel will hear you

#### Listen Button (Green)
- **Click to toggle** listening on/off
- Green when ON, gray when OFF
- When ON, you hear other users on your channel
- When OFF, no audio is received (privacy mode)

#### Channel Selection
- Choose from 4 available channels
- Only users on the same channel can communicate
- Switch channels require disconnecting and reconnecting

#### Audio Levels
- **TX (Transmit)** - Green/Yellow/Red meter showing your microphone level
- **RX (Receive)** - Blue meter showing incoming audio level
- Levels update in real-time

### Connected Users List

The bottom section shows:
- **User count** - Total users online
- **User list** with status indicators:
  - 🔴 Username (talking) - User is currently transmitting
  - 🟢 Username - User is listening
  - ⚪ Username - User is muted/not listening

## Network Architecture

### Server Functions
- Acts as a central hub for all audio traffic
- Routes audio packets between clients on the same channel
- Maintains list of connected users
- Monitors client heartbeats
- Automatically disconnects inactive clients (5 second timeout)

### Client Functions
- Connects to server with unique username
- Sends audio only when Talk button is pressed
- Receives audio only when Listen is enabled
- Sends heartbeat every second
- Filters audio by channel

### Protocol

Audio packets include:
- Packet type (Audio, UserJoin, UserLeave, Heartbeat)
- Sequence number
- Timestamp
- Username
- Channel number
- Talk/Listen status
- Encoded audio data (Opus codec)

## Configuration

### Audio Settings
- **Sample Rate:** 48 kHz
- **Bitrate:** 24 kbps Opus
- **Frame Size:** 20ms (960 samples)
- **Channels:** Mono

### Network Settings
- **Protocol:** UDP
- **Default Port:** 5000
- **Packet Size:** Up to 4KB
- **Heartbeat Interval:** 1 second
- **Client Timeout:** 5 seconds

## Keyboard Shortcuts

*(Future enhancement - not yet implemented)*
- `Space` - Hold to Talk
- `L` - Toggle Listen
- `1-4` - Quick channel switch
- `Ctrl+Q` - Quit

## Troubleshooting

### Audio Issues
- Check audio device selection
- Ensure microphone permissions are granted
- Verify input/output levels in system settings

### Connection Issues
- Verify server IP address is correct
- Check firewall settings (UDP port must be open)
- Ensure server is running before clients connect
- Check network connectivity

### Multi-User Issues
- Ensure all users are on the same channel
- Verify Talk button is being pressed
- Check that Listen is enabled
- Confirm users aren't timed out (check server console)

## System Requirements

- macOS 10.15+ (Catalina or later)
- 4GB RAM minimum
- Network connection (LAN or Internet)
- Microphone and speakers/headphones
- UDP port access (default: 5000)

## Technical Details

### Architecture
- **GUI Framework:** Qt 6
- **Audio I/O:** PortAudio
- **Audio Codec:** Opus
- **Network:** BSD Sockets (UDP)
- **Threading:** C++ std::thread with atomic operations

### Performance
- Low latency: ~20-50ms typical
- Bandwidth: ~24 kbps per active talker
- Scales to 10 simultaneous users on same channel
- Efficient packet routing at server

## Future Enhancements

- [ ] Keyboard shortcuts for Talk button
- [ ] Volume controls per user
- [ ] Audio recording/playback
- [ ] Text chat integration
- [ ] Multiple simultaneous channel monitoring
- [ ] Encrypted audio transmission
- [ ] Dynamic audio quality adjustment
- [ ] Mobile app version

## Credits

Inspired by professional intercom systems:
- Clear-Com HelixNet
- Clear-Com FreeSpeak II
- RTS (Radio Television Specialist) systems

## License

[Add your license here]

## Support

For issues or questions, please contact [your contact info].
