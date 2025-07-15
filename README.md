<p align="center">
  <img src="c.png" alt="C-Chat Logo" width="200" />
</p>

<p align="center">
  <a href="https://github.com/dunamismax/c-chat">
    <img src="https://readme-typing-svg.demolab.com/?font=Fira+Code&size=24&pause=1000&color=3071A4&center=true&vCenter=true&width=800&lines=C-Chat+Secure+Messaging;End-to-End+Encrypted+CLI+Chat;ARM64+Optimized+Security;Zero-Knowledge+Server+Architecture;Pure+C+Implementation." alt="Typing SVG" />
  </a>
</p>

<p align="center">
  <a href="https://clang.llvm.org/"><img src="https://img.shields.io/badge/Clang-17+-blue.svg?logo=llvm" alt="Clang Version"></a>
  <a href="https://developer.apple.com/documentation/apple-silicon"><img src="https://img.shields.io/badge/ARM64-Apple_Silicon-black.svg?logo=apple" alt="ARM64 Apple Silicon"></a>
  <a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/badge/License-MIT-green.svg" alt="MIT License"></a>
  <a href="https://github.com/dunamismax/c-chat/pulls"><img src="https://img.shields.io/badge/PRs-welcome-brightgreen.svg" alt="PRs Welcome"></a>
  <a href="https://github.com/dunamismax/c-chat/stargazers"><img src="https://img.shields.io/github/stars/dunamismax/c-chat" alt="GitHub Stars"></a>
</p>

---

## About This Project

A secure, end-to-end encrypted command-line chat application built entirely in pure C. C-Chat provides private messaging with zero-knowledge server architecture, ensuring maximum security and transparency through open-source implementation.

**Key Features:**

- **End-to-End Encryption**: Messages encrypted on your device, decryptable only by intended recipient
- **Zero-Knowledge Server**: Server never accesses private keys or unencrypted content
- **ARM64 Optimized**: Apple Silicon-specific optimizations with parallel builds and LTO
- **Security Hardened**: Buffer overflow protection, input validation, memory safety
- **Pure C Implementation**: Standard C for transparency, auditability, and performance
- **CLI Interface**: Clean, distraction-free terminal experience for focused conversations

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
git clone https://github.com/dunamismax/c-chat.git
cd c-chat

# Build the application
make                          # Release mode (default)
make MODE=debug              # Debug with sanitizers
make MODE=profile            # Profile build

# Run c-chat
make run                     # Launch application
./build/release/bin/c-chat   # Direct execution

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
make c-chat                 # Build main application
make test                   # Build and run test suite
make clean                  # Clean build artifacts

# Quality Assurance
make format                 # Format code with clang-format
make lint                   # Static analysis with clang-tidy
make benchmark             # Performance testing

# Execution
make run                   # Run c-chat application
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
c-chat/
├── src/                   # Source code
│   ├── main.c            # Application entry point
│   ├── interface.c       # CLI interface and commands
│   ├── user.c            # User management and authentication
│   ├── chat.c            # Chat functionality and messaging
│   ├── crypto.c          # Cryptographic operations
│   ├── network.c         # Network communication
│   └── utils.c           # Utility functions
├── include/               # Header files
├── tests/                 # Comprehensive test suite
├── scripts/               # Build automation
└── Makefile              # Build system
```

### Technology Stack

- **C11 Standard** with ARM64-specific optimizations
- **Clang Compiler** for Apple Silicon and cross-platform compatibility
- **End-to-End Encryption** with public-key cryptography
- **Zero-Knowledge Architecture** for maximum privacy
- **Cross-Platform Support** for macOS and Linux

---

## How It Works

C-Chat implements a secure client-server architecture with end-to-end encryption:

### Security Architecture

**Key Generation**: Each user generates a unique cryptographic key pair on first use - a public key shared with the server and a private key stored exclusively on the local device.

**Zero-Knowledge Server**: The server manages user accounts and relays encrypted messages but never has access to private keys or unencrypted content.

**End-to-End Encryption**: Messages are encrypted with the recipient's public key before transmission and can only be decrypted by the recipient's private key.

**Message Relay**: The server simply forwards encrypted data between clients, making intercepted communications unreadable to any third party.

### Usage Commands

```bash
# Account Management
c-chat --register <username>    # Register new user account
c-chat --login <username>       # Login to your account
c-chat --list-users            # List all registered users

# In-Chat Commands
chat <username>                # Start encrypted chat session
/exit                         # Leave current chat session
/quit                         # Exit c-chat application
```

---

## Testing & Quality

### Comprehensive Test Suite

```bash
make test                   # All tests with security validation
make test MODE=debug       # Debug build testing
make test MODE=release     # Release validation
```

**Test Coverage:**

- **Unit Tests**: Cryptographic functions, user validation, utility functions
- **Integration Tests**: CLI interface, command parsing, error handling
- **Security Tests**: Input validation, buffer overflow protection
- **Cross-Platform**: macOS and Linux compatibility

### Automated Testing

Comprehensive test automation with security focus:

- **Build & Test**: Debug, release, and profile modes across platforms
- **Security Testing**: Memory safety and vulnerability detection
- **Static Analysis**: clang-tidy integration with comprehensive checks
- **Performance Testing**: Benchmarking on optimized builds
- **Quality Assurance**: All tests must pass with security validation

---

## Security Features

### Cryptographic Security

- End-to-end encryption with public-key cryptography
- Secure key generation and local private key storage
- Zero-knowledge server architecture prevents data exposure
- No plaintext message storage on server infrastructure

### Memory Safety

- Buffer overflow protection with bounds checking
- Safe string operations with size validation
- Memory leak prevention with comprehensive cleanup
- AddressSanitizer and UndefinedBehaviorSanitizer integration

### Input Validation

- Username validation with character restrictions
- Message length limits and sanitization
- Command parsing with secure input handling
- Protection against injection attacks

### Secure Development

- Compiler hardening flags (`-fstack-protector-strong`)
- Static analysis integration for vulnerability detection
- Security-focused code review and testing procedures

---

## Troubleshooting

**Build Issues**: Run `make clean && make` to rebuild. Ensure Xcode Command Line Tools installed on macOS.

**Test Failures**: Run `make test MODE=debug` for detailed debugging output with sanitizers enabled.

**Missing Tools**: clang-tidy is optional - install with `brew install llvm` or lint target will skip.

**Performance**: Use `make MODE=release` for optimized builds. Debug mode includes sanitizers that impact performance.

---

## Support This Project

If you find this project valuable for your secure communication needs, consider supporting its continued development:

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
  <strong>Built with Pure C for Maximum Security</strong><br>
  <sub>Secure, private, end-to-end encrypted communication for the command line</sub>
</p>
