# WanComm

A real-time voice communication system built with Qt and C++. VoiceChat provides low-latency audio transmission between multiple users over a network, with a simple press-to-talk interface and multi-channel support.

## Quick Start

**1. Build the application:**
```bash
./build.sh
```

**2. Start the server (on one machine):**
```bash
./build/voicechat-server
```
Click "START SERVER" - it will listen on port 5000.

**3. Connect clients (on other machines):**
```bash
./build/voicechat-client
```
Enter a username, the server's IP address, select a channel (1-4), and click Connect.

**4. Talk!**
- Press and hold the red TALK button to transmit
- Toggle the green LISTEN button on to hear others
- Only users on the same channel can hear each other

That's it! See below for detailed installation and usage instructions.

---

## What It Does

VoiceChat is a networked intercom application that allows multiple users to communicate in real-time using voice. It features:

- **Press-to-talk audio transmission** - Hold the Talk button to broadcast your voice
- **4 independent channels** - Users on the same channel can hear each other
- **Multi-user support** - Up to 10 simultaneous users per server
- **Server/client architecture** - One machine runs as a server hub, others connect as clients
- **Visual feedback** - See who's talking and listening with real-time indicators
- **Audio level meters** - Monitor transmission and reception levels

## How It Works

VoiceChat uses a centralized server model where all audio is routed through a single server application:

1. **Server** - Acts as the central hub, receiving audio from all clients and routing it to other clients on the same channel
2. **Clients** - Connect to the server with a username, capture audio from the microphone when Talk is pressed, and play received audio through speakers
3. **Channels** - Audio is isolated by channel number, so only users on Channel 1 hear other Channel 1 users
4. **Codec** - Audio is compressed using Opus codec for efficient network transmission (~24 kbps)
5. **UDP Protocol** - Fast, low-latency packet delivery over UDP sockets

## Installation

### Prerequisites

You'll need these libraries installed:
- CMake 3.10+
- Qt 6.x
- PortAudio (audio I/O)
- Opus (audio codec)

**On macOS:**
```bash
brew install cmake qt portaudio opus
```

**On Linux (Ubuntu/Debian):**
```bash
sudo apt-get update
sudo apt-get install cmake build-essential qtbase6-dev \
    libportaudio2 portaudio19-dev libopus0 libopus-dev
```

### Building

Quick build:
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

This creates three executables:
- `voicechat` - Combined client/server chooser
- `voicechat-server` - Dedicated server
- `voicechat-client` - Dedicated client

## Usage

### Running as Server

Start the server application:
```bash
./build/voicechat-server
```

Or use the combined app and select "Server Mode" in the dialog. The server will listen on UDP port 5000 (configurable) and display connected users.

### Running as Client

Start the client application:
```bash
./build/voicechat-client
```

Or use the combined app and select "Client Mode". You'll need to:
1. Enter a username
2. Enter the server IP address
3. Select a channel (1-4)
4. Click Connect

### Controls

**Talk Button (Red)**
- Press and hold to transmit audio
- Your voice is sent to all users on your channel
- LED indicator shows when you're transmitting

**Listen Button (Green)**
- Click to toggle listening on/off
- When enabled, you hear other users on your channel
- When disabled, no audio is played (mute mode)

**Audio Meters**
- TX meter (green/yellow/red) - Shows your microphone input level
- RX meter (blue) - Shows incoming audio level from other users

**Users List**
- Shows all connected users and their usernames
- Indicators show who is currently talking (🔴) or listening (🟢)

## Technical Overview

### Architecture

**Server**
- Listens for UDP packets on a configurable port (default 5000)
- Maintains a list of connected users with their channels and states
- Routes audio packets to users on the same channel
- Monitors heartbeat packets and disconnects inactive clients

**Client**
- Captures audio via PortAudio when Talk button is pressed
- Compresses audio using Opus codec (24 kbps bitrate)
- Sends UDP packets to server with audio data, username, and state
- Receives and decodes audio from server
- Plays audio through speakers when Listen is enabled
- Sends periodic heartbeat packets to maintain connection

**Protocol**
Each packet contains:
- Packet type (Audio, UserJoin, UserLeave, Heartbeat)
- Sequence number and timestamp
- Username (up to 20 chars)
- Channel number (1-4)
- Talk/Listen status flags
- Encoded audio data (Opus frames)

### Audio Specifications

- Sample rate: 48 kHz
- Encoding: Opus codec, mono, 24 kbps
- Frame size: 20ms (960 samples)
- Latency: ~20-50ms typical
- Bandwidth: ~24 kbps per active talker

### Technology Stack

- **GUI:** Qt 6 (QWidget-based interface)
- **Audio I/O:** PortAudio (cross-platform audio)
- **Codec:** Opus (efficient, low-latency compression)
- **Network:** BSD sockets with UDP protocol
- **Threading:** C++ std::thread for audio processing
- **Build:** CMake

## Troubleshooting

**No audio received**
- Check that Listen button is enabled (green)
- Verify you're on the same channel as the speaker
- Check system audio settings and permissions

**No audio transmitted**
- Ensure Talk button is pressed and held down
- Verify microphone permissions in system settings
- Check microphone is selected as input device

**Cannot connect to server**
- Verify server is running
- Check server IP address is correct
- Ensure firewall allows UDP traffic on the configured port
- Confirm client and server are on the same network

**User disconnects unexpectedly**
- Check network stability
- Verify heartbeat packets are being sent (every second)
- Server timeout is 5 seconds - poor connections may disconnect

## System Requirements

- macOS 10.15+ or Linux
- 4GB RAM minimum
- Network connection (LAN recommended)
- Microphone and speakers/headphones
- Firewall access for UDP port (default 5000)
