#!/bin/bash

# Build script for VoiceChat Server and Client

echo "Building VoiceChat Server and Client..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SERVER=ON -DBUILD_CLIENT=ON ..

# Build
echo "Building..."
make -j$(sysctl -n hw.ncpu)

# Check if build was successful
if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Build successful!"
    echo ""
    echo "Executables:"
    echo "  Server: ./build/voicechat-server"
    echo "  Client: ./build/voicechat-client"
    echo ""
    echo "To run the server:"
    echo "  cd build && ./voicechat-server"
    echo ""
    echo "To run the client:"
    echo "  cd build && ./voicechat-client"
else
    echo ""
    echo "❌ Build failed!"
    exit 1
fi
