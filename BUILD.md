# Building VoiceChat

## Quick Start

### macOS

1. **Install dependencies:**
   ```bash
   brew install cmake portaudio opus
   ```

2. **Build the project:**
   ```bash
   cd VoiceChat
   mkdir build && cd build
   cmake ..
   make
   ```

3. **Run the application:**
   ```bash
   # Server
   ./voicechat -s 5000
   
   # Client (in another terminal)
   ./voicechat -c localhost 5000
   ```

### Linux (Ubuntu/Debian)

1. **Install dependencies:**
   ```bash
   sudo apt-get update
   sudo apt-get install cmake build-essential \
       libportaudio2 portaudio19-dev \
       libopus0 libopus-dev
   ```

2. **Build the project:**
   ```bash
   cd VoiceChat
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ```

3. **Run the application:**
   ```bash
   # Server
   ./voicechat -s 5000
   
   # Client (in another terminal)
   ./voicechat -c localhost 5000
   ```

## Advanced Build Options

### Debug Build

```bash
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Release Build (Optimized)

```bash
mkdir build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Installation

```bash
cd build
sudo make install
```

This installs the `voicechat` binary to `/usr/local/bin/`.

## Troubleshooting Build Issues

### PortAudio not found

If CMake can't find PortAudio:

**macOS:**
```bash
# Reinstall PortAudio
brew reinstall portaudio

# Or specify the path manually
cmake -DPORTAUDIO_INCLUDE_DIR=/usr/local/include \
      -DPORTAUDIO_LIBRARY=/usr/local/lib/libportaudio.dylib ..
```

**Linux:**
```bash
# Make sure dev package is installed
sudo apt-get install portaudio19-dev

# Or specify the path manually
cmake -DPORTAUDIO_INCLUDE_DIR=/usr/include \
      -DPORTAUDIO_LIBRARY=/usr/lib/x86_64-linux-gnu/libportaudio.so ..
```

### Opus not found

If CMake can't find Opus:

**macOS:**
```bash
# Reinstall Opus
brew reinstall opus

# Or specify the path manually
cmake -DOPUS_INCLUDE_DIR=/usr/local/include \
      -DOPUS_LIBRARY=/usr/local/lib/libopus.dylib ..
```

**Linux:**
```bash
# Make sure dev package is installed
sudo apt-get install libopus-dev

# Or specify the path manually
cmake -DOPUS_INCLUDE_DIR=/usr/include \
      -DOPUS_LIBRARY=/usr/lib/x86_64-linux-gnu/libopus.so ..
```

### C++17 Support

Ensure your compiler supports C++17:

```bash
# Check GCC version (need 7+ for full C++17)
g++ --version

# Check Clang version (need 5+ for full C++17)
clang++ --version

# Upgrade if needed
# macOS: Xcode Command Line Tools should be recent
xcode-select --install

# Ubuntu:
sudo apt-get install g++-9
```

## Testing

To test the audio system locally:

1. **Terminal 1 (Server):**
   ```bash
   ./voicechat -s 5000
   ```

2. **Terminal 2 (Client):**
   ```bash
   ./voicechat -c localhost 5000
   ```

3. **Speak into your microphone** - you should hear your voice with a slight delay
4. **Press `m`** to mute/unmute
5. **Press `s`** to see packet statistics
6. **Press `q`** to quit

## Network Testing

To test across computers on the same network:

1. **Find server IP:**
   ```bash
   # macOS/Linux
   ifconfig | grep "inet "
   
   # Or
   hostname -I
   ```

2. **Start server on first computer:**
   ```bash
   ./voicechat -s 5000
   ```

3. **Connect from second computer:**
   ```bash
   ./voicechat -c <SERVER_IP> 5000
   ```

## Performance Tuning

### Reduce Latency

Edit `VoiceChatConfig` in the code:
- Reduce `framesPerBuffer` (480 = 10ms, 240 = 5ms)
- Note: Smaller buffers increase CPU usage

### Improve Quality

- Increase `bitrate` (e.g., 48000 for 48kbps)
- Note: Higher bitrate uses more bandwidth

### Network Optimization

- Use wired Ethernet instead of WiFi
- Ensure QoS/priority for UDP port 5000
- Reduce other network traffic

## Clean Build

If you encounter issues, try a clean build:

```bash
rm -rf build
mkdir build && cd build
cmake ..
make
```

## Cross-Compilation

Instructions for cross-compiling to other platforms would go here.
