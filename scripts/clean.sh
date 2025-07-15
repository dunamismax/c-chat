#!/bin/bash

# Clean script for C Monorepo - ARM64 Optimized
# Author: dunamismax <dunamismax@tutamail.com>
# Comprehensive cleanup of ARM64-optimized build artifacts

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
C Monorepo Clean Script

Usage: $0 [OPTIONS]

OPTIONS:
    -a, --all            Clean everything including dependencies
    -d, --deep           Deep clean including backup files
    -v, --verbose        Verbose output
    -n, --dry-run        Show what would be cleaned without doing it
    -h, --help           Show this help message

EXAMPLES:
    $0                   # Standard clean of build artifacts
    $0 --all             # Clean everything including temp files
    $0 --dry-run         # Show what would be cleaned
    $0 --verbose         # Clean with detailed output

EOF
}

# Default values
CLEAN_ALL=false
DEEP_CLEAN=false
VERBOSE=false
DRY_RUN=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -a|--all)
            CLEAN_ALL=true
            shift
            ;;
        -d|--deep)
            DEEP_CLEAN=true
            CLEAN_ALL=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -n|--dry-run)
            DRY_RUN=true
            VERBOSE=true
            shift
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Change to project root
cd "$PROJECT_ROOT"

print_info "C Monorepo Clean Script"

if [[ "$DRY_RUN" == true ]]; then
    print_warning "DRY RUN MODE - No files will be deleted"
fi

# Function to remove files/directories
clean_path() {
    local path="$1"
    local description="$2"
    
    if [[ -e "$path" ]]; then
        if [[ "$VERBOSE" == true ]]; then
            print_info "Cleaning: $description ($path)"
        fi
        
        if [[ "$DRY_RUN" != true ]]; then
            rm -rf -- "$path"
        fi
        
        return 0
    else
        if [[ "$VERBOSE" == true ]]; then
            print_info "Not found: $description ($path)"
        fi
        return 1
    fi
}

# Function to clean build artifacts
clean_build_artifacts() {
    print_info "Cleaning build artifacts..."
    
    # Main build directory
    clean_path "build/" "Main build directory"
    
    # Object files in subdirectories (if any missed)
    find . -name "*.o" -type f | while read -r file; do
        clean_path "$file" "Object file"
    done
    
    # Static libraries
    find . -name "*.a" -type f | while read -r file; do
        clean_path "$file" "Static library"
    done
    
    # Executable files in apps (backup cleanup)
    find apps/ -name "*.exe" -o -name "a.out" -type f 2>/dev/null | while read -r file; do
        clean_path "$file" "Executable file"
    done
}

# Function to clean editor and IDE files
clean_editor_files() {
    print_info "Cleaning editor and IDE files..."
    
    # Vim files
    find . -name "*.swp" -o -name "*.swo" -o -name "*~" -type f | while read -r file; do
        clean_path "$file" "Vim swap/backup file"
    done
    
    # Emacs files
    find . -name "#*#" -o -name ".#*" -type f | while read -r file; do
        clean_path "$file" "Emacs backup file"
    done
    
    # IDE files
    clean_path ".vscode/" "VS Code settings"
    clean_path ".idea/" "IntelliJ IDEA settings"
    clean_path "*.code-workspace" "VS Code workspace"
}

# Function to clean system files
clean_system_files() {
    print_info "Cleaning system files..."
    
    # macOS files
    find . -name ".DS_Store" -type f | while read -r file; do
        clean_path "$file" "macOS .DS_Store file"
    done
    
    # Thumbs.db (Windows)
    find . -name "Thumbs.db" -type f | while read -r file; do
        clean_path "$file" "Windows Thumbs.db file"
    done
    
    # Core dumps
    find . -name "core" -o -name "core.*" -type f | while read -r file; do
        clean_path "$file" "Core dump file"
    done
}

# Function to clean log and temporary files
clean_temp_files() {
    print_info "Cleaning temporary files..."
    
    # Log files
    find . -name "*.log" -type f | while read -r file; do
        clean_path "$file" "Log file"
    done
    
    # Temporary files
    find . -name "*.tmp" -o -name "*.temp" -type f | while read -r file; do
        clean_path "$file" "Temporary file"
    done
    
    # Backup files
    find . -name "*.bak" -o -name "*.backup" -type f | while read -r file; do
        clean_path "$file" "Backup file"
    done
}

# Function to use make clean
clean_with_make() {
    print_info "Running make clean..."
    
    if [[ "$DRY_RUN" != true ]]; then
        if make clean > /dev/null 2>&1; then
            print_success "Make clean completed"
        else
            print_warning "Make clean failed or no Makefile found"
        fi
    else
        print_info "Would run: make clean"
    fi
}

# Function to show disk space before and after
show_disk_usage() {
    if command -v du &> /dev/null; then
        local size=$(du -sh . 2>/dev/null | cut -f1)
        print_info "Current directory size: $size"
    fi
}

# Show initial disk usage
if [[ "$VERBOSE" == true ]]; then
    show_disk_usage
fi

# Always clean build artifacts
clean_with_make
clean_build_artifacts

# Clean additional files if requested
if [[ "$CLEAN_ALL" == true ]]; then
    clean_editor_files
    clean_system_files
    clean_temp_files
fi

# Deep clean additional artifacts
if [[ "$DEEP_CLEAN" == true ]]; then
    print_info "Performing deep clean..."
    
    # Clean any remaining hidden files
    find . -name ".*~" -type f | while read -r file; do
        clean_path "$file" "Hidden backup file"
    done
    
    # Clean empty directories
    find . -type d -empty | while read -r dir; do
        if [[ "$dir" != "." && "$dir" != "./build" ]]; then
            clean_path "$dir" "Empty directory"
        fi
    done
fi

# Show final disk usage
if [[ "$VERBOSE" == true ]]; then
    show_disk_usage
fi

if [[ "$DRY_RUN" == true ]]; then
    print_info "Dry run completed - no files were actually deleted"
else
    print_success "Clean completed successfully"
fi

# Verify clean
if [[ "$DRY_RUN" != true ]]; then
    if [[ -d "build" ]] && [[ -n "$(ls -A build 2>/dev/null)" ]]; then
        print_warning "Some build artifacts may remain in build/"
    else
        print_success "All build artifacts cleaned"
    fi
fi