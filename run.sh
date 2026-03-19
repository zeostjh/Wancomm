#!/bin/bash

# VoiceChat Launch Script
# Professional Intercom System

echo "╔═══════════════════════════════════════════════════════╗"
echo "║                                                       ║"
echo "║     VoiceChat - Professional Intercom System         ║"
echo "║              ClearCom-Inspired Interface              ║"
echo "║                                                       ║"
echo "╚═══════════════════════════════════════════════════════╝"
echo ""
echo "Starting VoiceChat..."
echo ""
echo "Features:"
echo "  ✓ Multi-user support (up to 10 users)"
echo "  ✓ 4 independent channels"
echo "  ✓ Press-to-Talk button"
echo "  ✓ Listen toggle control"
echo "  ✓ Real-time user list"
echo "  ✓ Professional dark theme"
echo ""
echo "Quick Tips:"
echo "  • Press and HOLD red button to talk"
echo "  • Toggle green button to listen on/off"
echo "  • Watch user list for who's online"
echo "  • Choose same channel to communicate"
echo ""
echo "Documentation:"
echo "  • README.md - Full user guide"
echo "  • QUICKSTART.md - 5-minute setup"
echo "  • INTERFACE_GUIDE.md - UI reference"
echo ""

# Change to build directory
cd "$(dirname "$0")/build"

# Check if binary exists
if [ ! -f "voicechat" ]; then
    echo "❌ Error: voicechat binary not found!"
    echo "   Please run: ./build.sh"
    exit 1
fi

# Launch the application
./voicechat

echo ""
echo "VoiceChat closed."
