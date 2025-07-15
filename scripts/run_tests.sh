#!/bin/bash

# Test runner script for C Monorepo
# Author: dunamismax <dunamismax@tutamail.com>
# Version: 1.0.0

set -euo pipefail

# Colors for output
RED='\033[31m'
GREEN='\033[32m'
YELLOW='\033[33m'
BLUE='\033[34m'
MAGENTA='\033[35m'
CYAN='\033[36m'
NC='\033[0m' # No Color

# Test configuration
BUILD_MODE="${MODE:-debug}"
VERBOSE="${VERBOSE:-0}"
COVERAGE="${COVERAGE:-0}"
INDIVIDUAL="${INDIVIDUAL:-0}"

# Directories
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/${BUILD_MODE}"
BIN_DIR="${BUILD_DIR}/bin"

# Test executables
TESTS=(
    "test_math_utils"
    "test_vector" 
    "test_calculator"
    "test_integration"
)

# Statistics
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
FAILED_SUITES=()

print_banner() {
    echo -e "${CYAN}"
    echo "╔══════════════════════════════════════════════════════════════════════╗"
    echo "║                      C Monorepo Test Runner                         ║"
    echo "║                                                                      ║"
    echo "║  Mode: ${BUILD_MODE}$(printf '%*s' $((56 - ${#BUILD_MODE})) '')║"
    echo "║  Coverage: $([ "$COVERAGE" = "1" ] && echo "Enabled" || echo "Disabled")$(printf '%*s' $((49 - $([ "$COVERAGE" = "1" ] && echo 7 || echo 8))) '')║"
    echo "╚══════════════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -v, --verbose       Enable verbose output"
    echo "  -c, --coverage      Enable coverage reporting (requires profile mode)"
    echo "  -i, --individual    Run tests individually with detailed output"
    echo "  -m, --mode MODE     Build mode (debug|release|profile, default: debug)"
    echo ""
    echo "Environment Variables:"
    echo "  MODE                Build mode (debug|release|profile)"
    echo "  VERBOSE             Enable verbose output (0|1)"
    echo "  COVERAGE            Enable coverage (0|1)"
    echo "  INDIVIDUAL          Run individual tests (0|1)"
    echo ""
    echo "Examples:"
    echo "  $0                  Run all tests in debug mode"
    echo "  $0 -m release       Run tests in release mode"
    echo "  $0 -c -m profile    Run tests with coverage in profile mode"
    echo "  $0 -i -v            Run individual tests with verbose output"
}

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

check_dependencies() {
    log_info "Checking dependencies..."
    
    if ! command -v make >/dev/null 2>&1; then
        log_error "make is required but not installed"
        exit 1
    fi
    
    if ! command -v clang >/dev/null 2>&1; then
        log_error "clang is required but not installed"
        exit 1
    fi
    
    if [ "$COVERAGE" = "1" ] && [ "$BUILD_MODE" != "profile" ]; then
        log_warning "Coverage requires profile mode, setting MODE=profile"
        BUILD_MODE="profile"
        BUILD_DIR="${PROJECT_ROOT}/build/${BUILD_MODE}"
        BIN_DIR="${BUILD_DIR}/bin"
    fi
    
    log_success "Dependencies check passed"
}

build_tests() {
    log_info "Building applications and tests in ${BUILD_MODE} mode..."
    
    cd "$PROJECT_ROOT"
    
    # Build applications first (needed for integration tests)
    local build_exit_code=0
    
    if [ "$VERBOSE" = "1" ]; then
        make MODE="$BUILD_MODE" apps || build_exit_code=$?
        if [ $build_exit_code -eq 0 ]; then
            make MODE="$BUILD_MODE" build-tests || build_exit_code=$?
        fi
    else
        make MODE="$BUILD_MODE" apps >/dev/null 2>&1 || build_exit_code=$?
        if [ $build_exit_code -eq 0 ]; then
            make MODE="$BUILD_MODE" build-tests >/dev/null 2>&1 || build_exit_code=$?
        fi
    fi
    
    if [ $build_exit_code -eq 0 ]; then
        log_success "Applications and tests built successfully"
    else
        log_error "Failed to build applications and tests"
        exit 1
    fi
}

run_single_test() {
    local test_name="$1"
    local test_path="${BIN_DIR}/${test_name}"
    
    if [ ! -x "$test_path" ]; then
        log_error "Test executable not found: $test_path"
        return 1
    fi
    
    echo -e "${CYAN}Running ${test_name}...${NC}"
    
    if [ "$VERBOSE" = "1" ]; then
        "$test_path"
        local exit_code=$?
    else
        local output
        output=$("$test_path" 2>&1) || true
        local exit_code=$?
        
        if [ $exit_code -eq 0 ]; then
            local passed=$(echo "$output" | grep -o 'Tests passed: [0-9]*' | grep -o '[0-9]*' | head -1 || echo "")
            local total=$(echo "$output" | grep -o 'Tests run: [0-9]*' | grep -o '[0-9]*' | head -1 || echo "")
            
            # Default to 0 if parsing fails
            passed=${passed:-0}
            total=${total:-0}
            
            echo -e "${GREEN}✓ $test_name: $passed/$total tests passed${NC}"
            TOTAL_TESTS=$((TOTAL_TESTS + total))
            PASSED_TESTS=$((PASSED_TESTS + passed))
        else
            echo -e "${RED}✗ $test_name: FAILED${NC}"
            if [ "$VERBOSE" = "1" ]; then
                echo "$output"
            fi
            FAILED_SUITES+=("$test_name")
        fi
    fi
    
    return $exit_code
}

run_tests() {
    log_info "Running test suite..."
    echo ""
    
    local all_passed=0  # 0 = success in bash
    
    for test in "${TESTS[@]}"; do
        if [ "$INDIVIDUAL" = "1" ]; then
            echo -e "${BLUE}================================${NC}"
        fi
        
        if ! run_single_test "$test"; then
            all_passed=1  # 1 = failure in bash
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
        
        if [ "$INDIVIDUAL" = "1" ]; then
            echo -e "${BLUE}--------------------------------${NC}"
        fi
        echo ""
    done
    
    return $all_passed
}

generate_coverage() {
    if [ "$COVERAGE" = "1" ] && [ "$BUILD_MODE" = "profile" ]; then
        log_info "Generating coverage report..."
        
        cd "$PROJECT_ROOT"
        make coverage >/dev/null 2>&1
        
        if [ -d "build/coverage" ]; then
            log_success "Coverage report generated in build/coverage/"
        else
            log_warning "Coverage report generation failed"
        fi
    fi
}

print_summary() {
    echo ""
    echo -e "${CYAN}╔══════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║               Test Summary                   ║${NC}"
    echo -e "${CYAN}╚══════════════════════════════════════════════╝${NC}"
    echo ""
    
    if [ "$INDIVIDUAL" = "0" ]; then
        echo -e "Total tests:     ${BLUE}$TOTAL_TESTS${NC}"
        echo -e "Passed tests:    ${GREEN}$PASSED_TESTS${NC}"
        echo -e "Failed tests:    ${RED}$((TOTAL_TESTS - PASSED_TESTS))${NC}"
    fi
    
    echo -e "Test suites:     ${BLUE}${#TESTS[@]}${NC}"
    echo -e "Passed suites:   ${GREEN}$((${#TESTS[@]} - FAILED_TESTS))${NC}"
    echo -e "Failed suites:   ${RED}$FAILED_TESTS${NC}"
    
    if [ ${#FAILED_SUITES[@]} -gt 0 ]; then
        echo ""
        echo -e "${RED}Failed test suites:${NC}"
        for suite in "${FAILED_SUITES[@]}"; do
            echo -e "  ${RED}✗${NC} $suite"
        done
    fi
    
    echo ""
    if [ $FAILED_TESTS -eq 0 ]; then
        echo -e "${GREEN}🎉 All test suites passed!${NC}"
    else
        echo -e "${RED}❌ Some test suites failed!${NC}"
    fi
}

main() {
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                print_usage
                exit 0
                ;;
            -v|--verbose)
                VERBOSE=1
                shift
                ;;
            -c|--coverage)
                COVERAGE=1
                shift
                ;;
            -i|--individual)
                INDIVIDUAL=1
                shift
                ;;
            -m|--mode)
                BUILD_MODE="$2"
                BUILD_DIR="${PROJECT_ROOT}/build/${BUILD_MODE}"
                BIN_DIR="${BUILD_DIR}/bin"
                shift 2
                ;;
            *)
                log_error "Unknown option: $1"
                print_usage
                exit 1
                ;;
        esac
    done
    
    print_banner
    
    check_dependencies
    build_tests
    
    if run_tests; then
        generate_coverage
        print_summary
        exit 0
    else
        generate_coverage
        print_summary
        exit 1
    fi
}

main "$@"