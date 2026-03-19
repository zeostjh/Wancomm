# VoiceChat - Real-time Voice Communication Application

A cross-platform C++ voice chat application similar to Unity Intercom, featuring high-quality audio streaming with low latency.

## Features

- **Real-time Audio Streaming**: Low-latency voice communication
- **High-Quality Audio Codec**: Uses Opus codec for efficient compression
- **Cross-Platform**: Works on macOS, Linux, and Windows
- **Simple Architecture**: Client-server model with UDP networking
- **Mute/Unmute**: Easy control over microphone input
- **Statistics**: Monitor packets sent, received, and lost

## Architecture

The application consists of several key components:

- **AudioCapture**: Captures microphone input using PortAudio
- **AudioPlayback**: Plays received audio through speakers
- **AudioCodec**: Encodes/decodes audio using Opus codec
- **NetworkManager**: Handles UDP packet transmission
- **VoiceChatClient**: Orchestrates all components

## Prerequisites

### macOS

```bash
# Install dependencies using Homebrew
brew install cmake portaudio opus
```

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install cmake build-essential libportaudio2 portaudio19-dev libopus0 libopus-dev
```

### Windows

- Install Visual Studio 2019 or later with C++ support
- Download and install PortAudio and Opus libraries
- Set environment variables for library paths

## Building

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make

# The executable will be created as 'voicechat'
```

## Usage

### Starting a Server

```bash
./voicechat -s 5000
```

This starts a voice chat server listening on port 5000.

### Connecting as a Client

```bash
./voicechat -c 192.168.1.100 5000
```

This connects to a voice chat server at IP address 192.168.1.100 on port 5000.

### Commands

While the application is running, you can use these commands:

- `m` - Toggle mute/unmute your microphone
- `s` - Show statistics (packets sent/received/lost)
- `q` - Quit the application

## Configuration

Audio settings can be modified in `VoiceChatConfig`:

- **Sample Rate**: 48000 Hz (default)
- **Frames per Buffer**: 960 (20ms at 48kHz)
- **Channels**: 1 (mono)
- **Bitrate**: 24000 bps (24 kbps)

## Network Protocol

The application uses UDP for real-time audio transmission. Each packet contains:

- Sequence number (4 bytes)
- Timestamp (4 bytes)
- Data size (2 bytes)
- Encoded audio data (variable)

## Performance

- **Latency**: ~40-60ms end-to-end (depends on network)
- **Bandwidth**: ~24 kbps per audio stream
- **CPU Usage**: Low (efficient Opus encoding)

## Troubleshooting

### No audio input detected

- Check microphone permissions in System Preferences (macOS)
- Verify microphone is not being used by another application
- Run with verbose output to see PortAudio device list

### Connection issues

- Ensure firewall allows UDP traffic on the specified port
- Verify server IP address is correct and reachable
- Check that both client and server are using the same port

### Audio quality issues

- Increase bitrate in `VoiceChatConfig`
- Reduce network packet loss
- Check CPU usage isn't causing buffer underruns

## Future Enhancements

- Multiple simultaneous clients (conference mode)
- Echo cancellation and noise reduction
- Automatic gain control (AGC)
- GUI interface
- Encryption for secure communication
- Jitter buffer for better packet loss handling
- WebRTC integration

## License

This project is provided as-is for educational purposes.

## Dependencies

- **PortAudio**: Cross-platform audio I/O library
- **Opus**: High-quality audio codec
- **CMake**: Build system
- **C++17**: Modern C++ standard

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Author

Created as a demonstration of real-time audio streaming in C++.
