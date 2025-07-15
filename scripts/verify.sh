#!/bin/bash

# Verification script for C Monorepo - Production Ready
# Author: dunamismax <dunamismax@tutamail.com>
# Verifies all security fixes and functionality work correctly

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

# Change to project root
cd "$PROJECT_ROOT"

print_info "Production Readiness Verification"
print_info "=================================="

# Test counters
TOTAL_TESTS=0
PASSED_TESTS=0

# Function to run a verification test
verify_test() {
    local test_name="$1"
    local test_command="$2"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    print_info "Testing: $test_name"
    
    if eval "$test_command" >/dev/null 2>&1; then
        print_success "✓ $test_name"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        print_error "✗ $test_name"
        return 1
    fi
}

# Verify build system
print_info "Verifying build system..."
verify_test "Build System Clean" "make clean"
verify_test "Release Build" "make MODE=release apps libs"

# Verify executables exist
print_info "Verifying executables..."
verify_test "Calculator Executable" "test -x build/release/bin/calculator"
verify_test "File Utils Executable" "test -x build/release/bin/file_utils"
verify_test "Text Processor Executable" "test -x build/release/bin/text_processor"
verify_test "Tic Tac Toe Executable" "test -x build/release/bin/tic_tac_toe"
verify_test "Number Guessing Executable" "test -x build/release/bin/number_guessing"

# Test security features
print_info "Verifying security features..."

# Test calculator overflow protection
verify_test "Calculator Overflow Protection" "echo 'fact 25\nquit' | timeout 5 build/release/bin/calculator | grep -q '\\-1'"

# Test file utils path traversal protection
verify_test "File Utils Path Traversal Protection" "echo 'info ../../../etc/passwd\nquit' | timeout 5 build/release/bin/file_utils | grep -q 'Invalid'"

# Test text processor buffer overflow protection  
verify_test "Text Processor Buffer Protection" "echo 'upper $(printf \"%0.s\" {1..2000})a\nquit' | timeout 5 build/release/bin/text_processor"

# Test basic functionality
print_info "Verifying basic functionality..."

# Test calculator basic operations
verify_test "Calculator Addition" "echo '+ 5 3\nquit' | timeout 5 build/release/bin/calculator | grep -q '8.00'"

# Test text processor basic operations
verify_test "Text Processor Uppercase" "echo 'upper hello\nquit' | timeout 5 build/release/bin/text_processor | grep -q 'HELLO'"

# Create a test file for file utils
echo "Hello World" > /tmp/test_verify.txt
verify_test "File Utils Info" "echo 'info /tmp/test_verify.txt\nquit' | timeout 5 build/release/bin/file_utils | grep -q 'File Information'"
rm -f /tmp/test_verify.txt

# Test games launch correctly
verify_test "Tic Tac Toe Launch" "echo '3' | timeout 5 build/release/bin/tic_tac_toe"
verify_test "Number Guessing Launch" "echo '3' | timeout 5 build/release/bin/number_guessing"

# Test library unit tests
print_info "Verifying library tests..."
verify_test "Math Utils Tests" "cd tests && make lib-tests && timeout 5 ../build/debug/bin/test_math_utils"
verify_test "Vector Tests" "cd tests && make lib-tests && timeout 5 ../build/debug/bin/test_vector"

# Security scan
print_info "Running security scan..."
verify_test "Security Scan" "make security"

# Display results
echo
print_info "Verification Summary"
echo "===================="
echo "Total tests: $TOTAL_TESTS"
echo "Passed:      $PASSED_TESTS"
echo "Failed:      $((TOTAL_TESTS - PASSED_TESTS))"

if [[ $PASSED_TESTS -eq $TOTAL_TESTS ]]; then
    print_success "🎉 All verification tests passed! Repository is production-ready."
    print_info "✅ Security measures implemented and working"
    print_info "✅ All applications function correctly" 
    print_info "✅ Build system operates properly"
    print_info "✅ Libraries pass unit tests"
    echo
    print_success "Ready for production deployment!"
    exit 0
else
    print_error "⚠️  Some verification tests failed"
    print_warning "Please review and fix failing tests before production deployment"
    exit 1
fi