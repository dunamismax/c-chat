#!/bin/bash

# Build script for C Monorepo - ARM64 Optimized
# Author: dunamismax <dunamismax@tutamail.com>
# Provides convenient interface to the ARM64-optimized build system

set -euo pipefail  # Exit on error, undefined variables, pipe failures
IFS=$'\n\t'        # Secure Internal Field Separator

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Default values
MODE="debug"
VERBOSE=false
CLEAN=false
TARGET=""

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show usage
show_usage() {
    cat << EOF
C Monorepo Build Script

Usage: $0 [OPTIONS] [TARGET]

OPTIONS:
    -m, --mode MODE      Build mode: debug or release (default: debug)
    -c, --clean          Clean before building
    -v, --verbose        Verbose output
    -h, --help           Show this help message

TARGETS:
    all                  Build everything (default)
    libs                 Build only libraries
    apps                 Build only applications
    <specific-app>       Build specific application (e.g., calculator)
    test                 Build and run tests
    install              Install binaries to /usr/local/bin

EXAMPLES:
    $0                   # Build everything in debug mode
    $0 -m release        # Build everything in release mode
    $0 -c apps           # Clean and build applications
    $0 calculator        # Build only calculator application
    $0 --verbose test    # Run tests with verbose output

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -m|--mode)
            MODE="$2"
            if [[ "$MODE" != "debug" && "$MODE" != "release" ]]; then
                print_error "Invalid mode: $MODE. Use 'debug' or 'release'"
                exit 1
            fi
            shift 2
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        -*)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
        *)
            if [[ -z "$TARGET" ]]; then
                TARGET="$1"
            else
                print_error "Multiple targets specified. Use only one target."
                exit 1
            fi
            shift
            ;;
    esac
done

# Set default target
if [[ -z "$TARGET" ]]; then
    TARGET="all"
fi

# Change to project root
cd "$PROJECT_ROOT"

print_info "C Monorepo Build System"
print_info "Mode: $MODE"
print_info "Target: $TARGET"

# Clean if requested
if [[ "$CLEAN" == true ]]; then
    print_info "Cleaning previous builds..."
    make clean
    print_success "Clean completed"
fi

# Prepare make arguments
MAKE_ARGS="MODE=$MODE"
if [[ "$VERBOSE" == true ]]; then
    MAKE_ARGS="$MAKE_ARGS V=1"
fi

# Validate make arguments
if [[ ! "$MODE" =~ ^(debug|release)$ ]]; then
    print_error "Invalid mode: $MODE"
    exit 1
fi

# Build the target
print_info "Building target: $TARGET"
start_time=$(date +%s)

if make "$TARGET" $MAKE_ARGS; then
    end_time=$(date +%s)
    duration=$((end_time - start_time))
    print_success "Build completed successfully in ${duration}s"
    
    # Show available executables if build was successful
    if [[ "$TARGET" == "all" || "$TARGET" == "apps" ]]; then
        print_info "Available executables in build/bin/:"
        if [[ -d "build/bin" ]]; then
            ls -la "$PROJECT_ROOT/build/bin/" | tail -n +2 | while IFS= read -r line; do
                echo "  $line"
            done
        fi
    fi
else
    print_error "Build failed"
    exit 1
fi

# Additional actions based on target
case "$TARGET" in
    test)
        print_info "Tests completed"
        ;;
    install)
        print_success "Installation completed"
        print_info "Binaries installed to /usr/local/bin"
        ;;
esac

print_success "Build script completed successfully"