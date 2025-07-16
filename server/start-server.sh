#!/bin/bash

# C-Chat Server Startup Script
# This script builds and starts the C-Chat server with proper configuration

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_DIR="$SCRIPT_DIR"
BUILD_MODE="${1:-release}"

echo "Starting C-Chat Server..."
echo "Build Mode: $BUILD_MODE"
echo "Server Directory: $SERVER_DIR"

# Check for libsodium
if ! command -v pkg-config >/dev/null 2>&1; then
    echo "Warning: pkg-config not found, cannot verify libsodium installation"
elif ! pkg-config --exists libsodium; then
    echo "Error: libsodium not found"
    echo "Please install libsodium:"
    echo "  macOS: brew install libsodium"
    echo "  Ubuntu/Debian: sudo apt-get install libsodium-dev"
    echo "  Fedora/RHEL: sudo dnf install libsodium-devel"
    exit 1
fi

# Build the server
echo "Building C-Chat Server..."
cd "$SERVER_DIR"
make clean
make MODE="$BUILD_MODE"

if [ ! -f "build/$BUILD_MODE/bin/c-chat-server" ]; then
    echo "Error: Failed to build c-chat-server"
    exit 1
fi

echo "Build completed successfully"

# Create logs directory if it doesn't exist
mkdir -p logs

# Set up signal handlers for graceful shutdown
cleanup() {
    echo ""
    echo "Shutting down C-Chat Server gracefully..."
    exit 0
}

trap cleanup SIGINT SIGTERM

# Start the server
echo "Starting C-Chat Server on port 8080..."
echo "Press Ctrl+C to stop the server"
echo "=========================="

# Run the server with output to both console and log
exec ./build/"$BUILD_MODE"/bin/c-chat-server 2>&1 | tee logs/server-$(date +%Y%m%d-%H%M%S).log