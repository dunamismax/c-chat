#!/bin/bash

# Test script for C Monorepo - ARM64 Optimized
# Author: dunamismax <dunamismax@tutamail.com>
# Runs comprehensive tests for ARM64-optimized libraries and applications

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
C Monorepo Test Script

Usage: $0 [OPTIONS]

OPTIONS:
    -v, --verbose        Verbose output
    -q, --quick          Quick tests only (skip long-running tests)
    -l, --libs-only      Test libraries only
    -a, --apps-only      Test applications only
    -h, --help           Show this help message

EXAMPLES:
    $0                   # Run all tests
    $0 --verbose         # Run all tests with verbose output
    $0 --libs-only       # Test only libraries
    $0 --quick           # Run only quick tests

EOF
}

# Default values
VERBOSE=false
QUICK=false
LIBS_ONLY=false
APPS_ONLY=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -q|--quick)
            QUICK=true
            shift
            ;;
        -l|--libs-only)
            LIBS_ONLY=true
            shift
            ;;
        -a|--apps-only)
            APPS_ONLY=true
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

print_info "C Monorepo Test Suite"
print_info "Starting comprehensive testing..."

# Test counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test
run_test() {
    local test_name="$1"
    local test_command="$2"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    print_info "Running test: $test_name"
    
    if [[ "$VERBOSE" == true ]]; then
        echo "Command: $test_command"
    fi
    
    if eval "$test_command" > /dev/null 2>&1; then
        print_success "✓ $test_name"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        print_error "✗ $test_name"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
}

# Function to test if executables exist and are functional
test_executable() {
    local exe_name="$1"
    local exe_path="build/bin/$exe_name"
    
    if [[ ! -f "$exe_path" ]]; then
        print_error "Executable not found: $exe_path"
        return 1
    fi
    
    if [[ ! -x "$exe_path" ]]; then
        print_error "Executable not executable: $exe_path"
        return 1
    fi
    
    return 0
}

# Build everything first
print_info "Building project for testing..."
if ! make all MODE=debug > /dev/null 2>&1; then
    print_error "Build failed - cannot run tests"
    exit 1
fi
print_success "Build completed"

# Test library compilation
if [[ "$APPS_ONLY" != true ]]; then
    print_info "Testing library compilation..."
    
    run_test "Data Structures Library Build" "make -C libs/data_structures clean && make -C libs/data_structures"
    run_test "Math Utils Library Build" "make -C libs/math_utils clean && make -C libs/math_utils"
    
    # Check if library files exist
    run_test "Data Structures Library File" "test -f build/lib/libdatastructures.a"
    run_test "Math Utils Library File" "test -f build/lib/libmathutils.a"
fi

# Test application compilation and basic functionality
if [[ "$LIBS_ONLY" != true ]]; then
    print_info "Testing application builds..."
    
    # Test calculator
    run_test "Calculator Build" "make -C apps/cli/calculator clean && make -C apps/cli/calculator"
    if test_executable "calculator"; then
        run_test "Calculator Basic Test" "echo 'help\nquit' | timeout 5 '$PROJECT_ROOT/build/bin/calculator'"
    fi
    
    # Test file_utils
    run_test "File Utils Build" "make -C apps/cli/file_utils clean && make -C apps/cli/file_utils"
    if test_executable "file_utils"; then
        run_test "File Utils Basic Test" "echo 'help\nquit' | timeout 5 build/bin/file_utils"
    fi
    
    # Test text_processor
    run_test "Text Processor Build" "make -C apps/cli/text_processor clean && make -C apps/cli/text_processor"
    if test_executable "text_processor"; then
        run_test "Text Processor Basic Test" "echo 'help\nquit' | timeout 5 build/bin/text_processor"
    fi
    
    # Test games
    run_test "Tic-Tac-Toe Build" "make -C apps/games/tic_tac_toe clean && make -C apps/games/tic_tac_toe"
    if test_executable "tic_tac_toe"; then
        run_test "Tic-Tac-Toe Basic Test" "echo '3' | timeout 5 build/bin/tic_tac_toe"
    fi
    
    run_test "Number Guessing Build" "make -C apps/games/number_guessing clean && make -C apps/games/number_guessing"
    if test_executable "number_guessing"; then
        run_test "Number Guessing Basic Test" "echo '3' | timeout 5 build/bin/number_guessing"
    fi
fi

# Functional tests (if not quick mode)
if [[ "$QUICK" != true && "$LIBS_ONLY" != true ]]; then
    print_info "Running functional tests..."
    
    # Test calculator functionality
    if test_executable "calculator"; then
        run_test "Calculator Addition" "echo '+ 5 3\nquit' | timeout 5 build/bin/calculator | grep -q 'Result: 8.00'"
        run_test "Calculator Factorial" "echo 'fact 5\nquit' | timeout 5 build/bin/calculator | grep -q 'Result: 120'"
    fi
    
    # Test file_utils functionality
    if test_executable "file_utils"; then
        # Create a test file
        echo "Test content for file utils" > /tmp/test_file_utils.txt
        run_test "File Utils Info" "echo 'info /tmp/test_file_utils.txt\nquit' | timeout 5 build/bin/file_utils | grep -q 'File Information'"
        run_test "File Utils Count" "echo 'count /tmp/test_file_utils.txt\nquit' | timeout 5 build/bin/file_utils | grep -q 'File Statistics'"
        rm -f /tmp/test_file_utils.txt
    fi
fi

# Memory leak tests (if valgrind is available and not quick mode)
if [[ "$QUICK" != true ]] && command -v valgrind &> /dev/null; then
    print_info "Running memory leak tests..."
    
    if test_executable "calculator"; then
        run_test "Calculator Memory Leak" "echo 'help\nquit' | timeout 10 valgrind --leak-check=full --error-exitcode=1 build/bin/calculator"
    fi
else
    print_warning "Skipping memory leak tests (valgrind not available or quick mode)"
fi

# Code quality tests (if tools are available)
if command -v cppcheck &> /dev/null && [[ "$QUICK" != true ]]; then
    print_info "Running static analysis..."
    run_test "Static Analysis - Libraries" "cppcheck --enable=all --error-exitcode=1 --suppress=missingIncludeSystem libs/ 2>/dev/null"
    run_test "Static Analysis - Applications" "cppcheck --enable=all --error-exitcode=1 --suppress=missingIncludeSystem apps/ 2>/dev/null"
else
    print_warning "Skipping static analysis (cppcheck not available or quick mode)"
fi

# Clean mode tests
print_info "Testing clean functionality..."
run_test "Top-level Clean" "make clean"
run_test "Verify Clean" "test ! -d build/bin || test -z \"\$(ls -A build/bin 2>/dev/null)\""

# Rebuild test
run_test "Rebuild After Clean" "make all MODE=debug"

# Display results
echo
print_info "Test Summary"
echo "=============="
echo "Total tests: $TOTAL_TESTS"
echo "Passed:      $PASSED_TESTS"
echo "Failed:      $FAILED_TESTS"

if [[ $FAILED_TESTS -eq 0 ]]; then
    print_success "All tests passed! 🎉"
    exit 0
else
    print_error "$FAILED_TESTS test(s) failed"
    exit 1
fi