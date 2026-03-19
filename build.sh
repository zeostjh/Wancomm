#!/bin/bash

# VoiceChat Build Script
# This script helps automate the build process

set -e  # Exit on error

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${YELLOW}ℹ $1${NC}"
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check for required tools
check_dependencies() {
    print_info "Checking dependencies..."
    
    local missing_deps=()
    
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    fi
    
    if ! command_exists make; then
        missing_deps+=("make")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        
        if [[ "$OSTYPE" == "darwin"* ]]; then
            echo "Install with: brew install ${missing_deps[*]}"
        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            echo "Install with: sudo apt-get install ${missing_deps[*]}"
        fi
        exit 1
    fi
    
    print_success "All build tools found"
}

# Check for audio libraries
check_libraries() {
    print_info "Checking for audio libraries..."
    
    local missing_libs=()
    
    # Check for PortAudio
    if ! pkg-config --exists portaudio-2.0 2>/dev/null; then
        if [ ! -f "/usr/local/lib/libportaudio.dylib" ] && \
           [ ! -f "/usr/lib/x86_64-linux-gnu/libportaudio.so" ] && \
           [ ! -f "/usr/lib/libportaudio.so" ]; then
            missing_libs+=("portaudio")
        fi
    fi
    
    # Check for Opus
    if ! pkg-config --exists opus 2>/dev/null; then
        if [ ! -f "/usr/local/lib/libopus.dylib" ] && \
           [ ! -f "/usr/lib/x86_64-linux-gnu/libopus.so" ] && \
           [ ! -f "/usr/lib/libopus.so" ]; then
            missing_libs+=("opus")
        fi
    fi
    
    if [ ${#missing_libs[@]} -ne 0 ]; then
        print_error "Missing libraries: ${missing_libs[*]}"
        
        if [[ "$OSTYPE" == "darwin"* ]]; then
            echo "Install with: brew install ${missing_libs[*]}"
        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            echo "Install with: sudo apt-get install"
            for lib in "${missing_libs[@]}"; do
                if [ "$lib" == "portaudio" ]; then
                    echo "  - portaudio19-dev"
                elif [ "$lib" == "opus" ]; then
                    echo "  - libopus-dev"
                fi
            done
        fi
        exit 1
    fi
    
    print_success "All required libraries found"
}

# Clean build directory
clean_build() {
    print_info "Cleaning build directory..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    else
        print_info "Build directory doesn't exist, nothing to clean"
    fi
}

# Build the project
build_project() {
    print_info "Creating build directory..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    print_info "Running CMake..."
    if [ "$1" == "debug" ]; then
        cmake -DCMAKE_BUILD_TYPE=Debug ..
    else
        cmake -DCMAKE_BUILD_TYPE=Release ..
    fi
    print_success "CMake configuration complete"
    
    print_info "Building project..."
    local cpu_count=1
    if [[ "$OSTYPE" == "darwin"* ]]; then
        cpu_count=$(sysctl -n hw.ncpu)
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        cpu_count=$(nproc)
    fi
    
    make -j"$cpu_count"
    print_success "Build complete!"
    
    cd "$PROJECT_DIR"
}

# Print usage
print_usage() {
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  build        Build the project (default, release mode)"
    echo "  debug        Build the project in debug mode"
    echo "  clean        Clean build directory"
    echo "  rebuild      Clean and build"
    echo "  check        Check dependencies and libraries"
    echo "  run-server   Build and run as server on port 5000"
    echo "  run-client   Build and run as client connecting to localhost:5000"
    echo "  help         Show this help message"
}

# Main script logic
main() {
    cd "$PROJECT_DIR"
    
    case "${1:-build}" in
        check)
            check_dependencies
            check_libraries
            ;;
        clean)
            clean_build
            ;;
        debug)
            check_dependencies
            check_libraries
            build_project debug
            ;;
        build)
            check_dependencies
            check_libraries
            build_project release
            ;;
        rebuild)
            check_dependencies
            check_libraries
            clean_build
            build_project release
            ;;
        run-server)
            check_dependencies
            check_libraries
            build_project release
            print_info "Starting VoiceChat server on port 5000..."
            "$BUILD_DIR/voicechat" -s 5000
            ;;
        run-client)
            check_dependencies
            check_libraries
            build_project release
            print_info "Connecting to localhost:5000..."
            "$BUILD_DIR/voicechat" -c localhost 5000
            ;;
        help|--help|-h)
            print_usage
            ;;
        *)
            print_error "Unknown command: $1"
            print_usage
            exit 1
            ;;
    esac
}

main "$@"
