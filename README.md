<p align="center">
  <img src="c.png" alt="C Monorepo Logo" width="200" />
</p>

<p align="center">
  <a href="https://github.com/dunamismax/c-monorepo">
    <img src="https://readme-typing-svg.demolab.com/?font=Fira+Code&size=24&pause=1000&color=3071A4&center=true&vCenter=true&width=800&lines=C+Programming+Monorepo;ARM64+Optimized+Build+System;Educational+%2B+Production+Ready;High-Performance+C+Development;From+CLI+Tools+to+Games." alt="Typing SVG" />
  </a>
</p>

<p align="center">
  <a href="https://clang.llvm.org/"><img src="https://img.shields.io/badge/Clang-17+-blue.svg?logo=llvm" alt="Clang Version"></a>
  <a href="https://developer.apple.com/documentation/apple-silicon"><img src="https://img.shields.io/badge/ARM64-Apple_Silicon-black.svg?logo=apple" alt="ARM64 Apple Silicon"></a>
  <a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/badge/License-MIT-green.svg" alt="MIT License"></a>
  <a href="https://github.com/dunamismax/c-monorepo/pulls"><img src="https://img.shields.io/badge/PRs-welcome-brightgreen.svg" alt="PRs Welcome"></a>
  <a href="https://github.com/dunamismax/c-monorepo/stargazers"><img src="https://img.shields.io/github/stars/dunamismax/c-monorepo" alt="GitHub Stars"></a>
</p>

---

## About This Project

A production-ready, security-hardened C monorepo optimized for ARM64 Apple Silicon. Features 5 working applications, 2 optimized libraries, and a comprehensive build system with advanced compiler optimizations and automated testing.

**Key Features:**

- **ARM64 Optimized**: Apple Silicon-specific optimizations with parallel builds and LTO
- **Security Hardened**: Buffer overflow protection, input validation, memory safety
- **Production Ready**: Comprehensive testing and quality assurance with automated test suite
- **Professional Build System**: Cross-platform Makefile with intelligent dependency detection
- **Complete Applications**: Calculator, file utilities, text processor, games with AI
- **Optimized Libraries**: Math utilities and data structures for high-performance computing

---

## Quick Start

### Prerequisites

```bash
# macOS (Primary Platform)
xcode-select --install

# Ubuntu/Debian
sudo apt-get install build-essential clang make

# Optional tools
brew install clang-format    # macOS
```

### Build & Run

```bash
git clone https://github.com/dunamismax/c-monorepo.git
cd c-monorepo

# Build everything
make                          # Release mode (default)
make MODE=debug              # Debug with sanitizers
make MODE=profile            # Profile build

# Run applications
make run-calculator          # Scientific calculator
make run-tic_tac_toe        # AI-powered game
make run-file_utils         # File operations

# Development workflow
make test                    # Run comprehensive test suite
make format lint            # Code quality checks
```

---

## Build System

Professional cross-platform Makefile with ARM64 optimization, parallel builds, and comprehensive development tools.

### Core Commands

```bash
# Building
make libs apps              # Build libraries and applications
make test                   # Build and run test suite
make clean                  # Clean build artifacts

# Quality Assurance
make format                 # Format code with clang-format
make lint                   # Static analysis with clang-tidy
make benchmark             # Performance testing

# Execution
make run-<app>             # Run specific application
make install               # Install to /usr/local
make help                  # Show all targets
```

### Optimization Features

- **Apple Silicon**: `-mcpu=apple-m1 -mtune=apple-m1 -arch arm64` for maximum performance
- **Link-Time Optimization**: `-flto=thin` in release builds
- **Parallel Builds**: Automatically uses all CPU cores
- **Cross-Platform**: Adapts flags for macOS and Linux
- **Security**: Stack protection and memory sanitizers in debug mode

---

## Project Structure

```
c-monorepo/
├── apps/                   # Applications
│   ├── cli/               # Command-line tools
│   └── games/             # Interactive games
├── libs/                  # Shared libraries
│   ├── data_structures/   # Vector implementation
│   └── math_utils/        # Mathematical algorithms
├── tests/                 # Comprehensive test suite
├── scripts/               # Build automation
└── Makefile              # Build system
```

### Technology Stack

- **C11 Standard** with ARM64-specific optimizations
- **Clang Compiler** for Apple Silicon and cross-platform compatibility
- **Comprehensive Testing** with automated test suite and security scanning
- **Static Analysis** with clang-tidy and AddressSanitizer
- **Cross-Platform Support** for macOS and Linux

---

## Applications & Libraries

### Applications

**Calculator**: Scientific calculator with advanced mathematical operations, factorial, prime checking, and comprehensive input validation.

**File Utils**: Secure file operations with path traversal protection and system programming features.

**Text Processor**: String manipulation tool with case conversion, find/replace, and interactive sorting.

**Tic-Tac-Toe**: AI-powered game with intelligent strategies and two-player mode.

**Number Guessing**: Multi-difficulty game with hint system and performance tracking.

### Libraries

**Math Utils** (`libmath_utils.a`): Euclidean GCD/LCM, factorial, Fibonacci, prime checking, and power functions optimized for ARM64.

**Data Structures** (`libdata_structures.a`): Dynamic vector implementation with bounds checking and efficient memory management.

---

## Testing & Quality

### Comprehensive Test Suite

```bash
make test                   # All tests (29 tests across 4 suites)
make test MODE=debug       # Debug build testing
make test MODE=release     # Release validation
```

**Test Coverage:**

- **Unit Tests**: Math utilities, data structures
- **Integration Tests**: Application functionality and security
- **Security Tests**: Buffer overflow, format string protection
- **Cross-Platform**: macOS and Linux compatibility

### Automated Testing

Comprehensive test automation with multi-platform support:

- **Build & Test**: Debug, release, and profile modes across macOS and Linux
- **Security Testing**: Buffer overflow and vulnerability detection
- **Static Analysis**: clang-tidy integration with comprehensive checks
- **Performance Testing**: Benchmarking on release builds
- **Quality Assurance**: All tests must pass with comprehensive quality gates

---

## Security Features

### Memory Safety

- Buffer overflow protection with bounds checking
- Safe string operations with size validation
- Memory leak prevention with comprehensive cleanup
- AddressSanitizer and UndefinedBehaviorSanitizer integration

### Input Validation

- Path traversal prevention in file operations
- Format string attack protection in calculator
- Comprehensive input sanitization across all applications
- Integer overflow detection in mathematical operations

### Secure Development

- Compiler hardening flags (`-fstack-protector-strong`)
- Static analysis integration for vulnerability detection
- Security-focused code review and testing procedures

---

## Troubleshooting

**Build Issues**: Run `make clean && make` to rebuild. Ensure Xcode Command Line Tools installed on macOS.

**Test Failures**: Build applications first with `make apps`. Tests require built binaries for integration testing.

**Missing Tools**: clang-tidy is optional - install with `brew install llvm` or lint target will skip.

**Performance**: Use `make MODE=release` for optimized builds. Debug mode includes sanitizers that impact performance.

---

## Support This Project

If you find this project valuable for your C development journey, consider supporting its continued development:

<p align="center">
  <a href="https://www.buymeacoffee.com/dunamismax" target="_blank">
    <img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 60px !important;width: 217px !important;" />
  </a>
</p>

---

## Let's Connect

<p align="center">
  <a href="https://twitter.com/dunamismax" target="_blank"><img src="https://img.shields.io/badge/Twitter-%231DA1F2.svg?&style=for-the-badge&logo=twitter&logoColor=white" alt="Twitter"></a>
  <a href="https://bsky.app/profile/dunamismax.bsky.social" target="_blank"><img src="https://img.shields.io/badge/Bluesky-blue?style=for-the-badge&logo=bluesky&logoColor=white" alt="Bluesky"></a>
  <a href="https://reddit.com/user/dunamismax" target="_blank"><img src="https://img.shields.io/badge/Reddit-%23FF4500.svg?&style=for-the-badge&logo=reddit&logoColor=white" alt="Reddit"></a>
  <a href="https://discord.com/users/dunamismax" target="_blank"><img src="https://img.shields.io/badge/Discord-dunamismax-7289DA.svg?style=for-the-badge&logo=discord&logoColor=white" alt="Discord"></a>
  <a href="https://signal.me/#p/+dunamismax.66" target="_blank"><img src="https://img.shields.io/badge/Signal-dunamismax.66-3A76F0.svg?style=for-the-badge&logo=signal&logoColor=white" alt="Signal"></a>
</p>

---

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

---

<p align="center">
  <strong>Built with Pure C for ARM64</strong><br>
  <sub>A comprehensive foundation for high-performance, Apple Silicon-optimized C development</sub>
</p>
