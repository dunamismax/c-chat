<p align="center">
  <img src="c.png" alt="C-Chat Logo" width="200" />
</p>

<p align="center">
  <a href="https://github.com/dunamismax/c-chat">
    <img src="https://readme-typing-svg.demolab.com/?font=Fira+Code&size=24&pause=1000&color=3071A4&center=true&vCenter=true&width=800&lines=C-Chat+Production+Ready;End-to-End+Encrypted+CLI+Chat;LibSodium+Powered+Security;ARM64+Optimized+Performance;Pure+C+Implementation." alt="Typing SVG" />
  </a>
</p>

<p align="center">
  <a href="https://clang.llvm.org/"><img src="https://img.shields.io/badge/Clang-17+-blue.svg?logo=llvm" alt="Clang Version"></a>
  <a href="https://libsodium.gitbook.io/doc/"><img src="https://img.shields.io/badge/LibSodium-1.0.20+-green.svg" alt="LibSodium"></a>
  <a href="https://developer.apple.com/documentation/apple-silicon"><img src="https://img.shields.io/badge/ARM64-Apple_Silicon-black.svg?logo=apple" alt="ARM64 Apple Silicon"></a>
  <a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/badge/License-MIT-green.svg" alt="MIT License"></a>
  <a href="https://github.com/dunamismax/c-chat/pulls"><img src="https://img.shields.io/badge/PRs-welcome-brightgreen.svg" alt="PRs Welcome"></a>
  <a href="https://github.com/dunamismax/c-chat/stargazers"><img src="https://img.shields.io/github/stars/dunamismax/c-chat" alt="GitHub Stars"></a>
</p>

---

## About This Project

A **production-grade, end-to-end encrypted command-line chat application** built entirely in pure C with enterprise-level security. C-Chat provides secure private messaging with cryptographically strong encryption, secure key management, and zero-knowledge architecture.

**Key Features:**

- **End-to-End Encryption**: ChaCha20-Poly1305 encryption via libsodium (NaCl)
- **Secure Key Management**: Argon2-based password encryption for private key storage
- **Memory Safety**: Comprehensive buffer overflow protection and secure memory clearing
- **ARM64 Optimized**: Apple Silicon-specific optimizations with parallel builds and LTO
- **Zero-Knowledge**: Private keys never leave your device, server only relays encrypted data
- **CLI Interface**: Clean, distraction-free terminal experience for focused conversations
- **Production Ready**: Full test suite, error handling, and security validation

---

## Quick Start

### Prerequisites

**Required Dependencies:**

```bash
# macOS (Primary Platform)
xcode-select --install
brew install libsodium

# Ubuntu/Debian
sudo apt-get install build-essential clang make libsodium-dev

# Fedora/RHEL
sudo dnf install clang make libsodium-devel

# Optional development tools
brew install clang-format llvm    # macOS
```

### Installation & Setup

```bash
# Clone repository
git clone https://github.com/dunamismax/c-chat.git
cd c-chat

# Build application (production-optimized)
make                          # Release mode (default)
make MODE=debug              # Debug with sanitizers
make MODE=profile            # Profile build

# Verify installation
./build/release/bin/c-chat --version
make test                    # Run comprehensive test suite
```

### First Time Usage

```bash
# Register a new user account
./build/release/bin/c-chat --register alice

# Login to start chatting
./build/release/bin/c-chat --login alice

# List registered users
./build/release/bin/c-chat --list-users
```

---

## Usage Guide

### Account Management

```bash
# Register new user (generates cryptographic keys)
c-chat --register <username>    # Creates ~/.c-chat/<username>.keys

# Login to your account (decrypts your private key)
c-chat --login <username>       # Prompts for password

# View registered users
c-chat --list-users            # Shows available users
```

### Secure Chat Commands

Once logged in, use these commands in the chat interface:

```bash
# Start encrypted chat session
chat <username>                # Begin secure messaging

# In-chat commands
/exit                         # Leave current chat session
/quit                         # Exit c-chat application
```

### Example Chat Session

```bash
$ ./build/release/bin/c-chat --register alice
Enter password to encrypt your private key: [hidden]
Confirm password: [hidden]
Generated cryptographically secure keypair
Keys saved securely to: /Users/alice/.c-chat/alice.keys
User registration completed successfully!

$ ./build/release/bin/c-chat --login alice
Enter your password: [hidden]
Authentication successful!

c-chat> chat bob
Enter your password to start secure chat: [hidden]
Retrieving bob's public key from server...
Secure chat established with bob
End-to-end encryption active (ChaCha20-Poly1305)

bob> Hello Bob! This message is encrypted end-to-end.
You: Hello Bob! This message is encrypted end-to-end.
[Message encrypted (89 bytes) and ready for transmission]
[Delivered to bob]

bob> /exit
Chat session ended.
c-chat> /quit
Goodbye!
```

---

## Build System

Professional cross-platform Makefile with ARM64 optimization and comprehensive development tools.

### Core Commands

```bash
# Building
make                       # Build optimized release version
make c-chat               # Build main application only
make test                 # Build and run comprehensive tests
make clean                # Clean all build artifacts

# Development & Quality
make MODE=debug           # Debug build with sanitizers
make format               # Format code with clang-format
make lint                 # Static analysis with clang-tidy
make benchmark           # Performance testing
make install             # Install to /usr/local

# Information
make help                # Show all available targets
make sysinfo             # Display system information
```

### Optimization Features

- **Apple Silicon**: `-mcpu=apple-m1 -mtune=apple-m1 -arch arm64` for maximum performance
- **Link-Time Optimization**: `-flto=thin` in release builds for smaller, faster binaries
- **Parallel Builds**: Automatically detects and uses all CPU cores
- **Cross-Platform**: Intelligent flag adaptation for macOS, Linux, and other platforms
- **Security Hardening**: Stack protection, sanitizers, and secure compilation flags

---

## Security Architecture

### Cryptographic Implementation

**Key Generation & Storage:**

- **Algorithm**: Curve25519 key pairs via `crypto_box_keypair()`
- **Storage**: Private keys encrypted with Argon2-derived keys from user passwords
- **Protection**: 600 permissions on key files, stored in `~/.c-chat/`

**Message Encryption:**

- **Algorithm**: ChaCha20-Poly1305 via `crypto_box_seal()` (anonymous encryption)
- **Security**: Forward secrecy, authenticated encryption, resistant to quantum attacks
- **Implementation**: Uses libsodium (NaCl) - audited, production-grade cryptography

**Password Security:**

- **Derivation**: Argon2 key derivation function (memory-hard, side-channel resistant)
- **Salt**: Unique random salt per user for key derivation
- **Storage**: Only encrypted private keys stored, passwords never saved

### Memory Safety Features

```c
// Example security implementations:
void secure_zero_memory(void* ptr, size_t size) {
    if (ptr && size > 0) {
        sodium_memzero(ptr, size);  // Compiler-resistant memory clearing
    }
}

void safe_strncpy(char* dest, const char* src, size_t size) {
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';  // Always null-terminate
}
```

**Security Measures:**

- **Buffer Overflow Protection**: All string operations use size-bounded functions
- **Memory Clearing**: Sensitive data securely zeroed after use
- **Input Validation**: Username and message sanitization with length limits
- **Error Handling**: Secure cleanup on all error paths

### File Security

**📁 Key File Format:**

```sh
[32-byte salt][24-byte nonce][32-byte public_key][48-byte encrypted_private_key]
```

**Protection:**

- Files created with `0600` permissions (owner read/write only)
- Atomic file operations to prevent corruption
- Secure deletion of temporary data

---

## Testing & Quality Assurance

### Comprehensive Test Suite

```bash
# Run all tests
make test                   # Full test suite with security validation

# Specialized testing
make test MODE=debug       # Debug build with sanitizers
make test MODE=release     # Release build validation
make benchmark             # Performance benchmarks
```

**Test Coverage:**

- **Cryptographic Tests**: Key generation, encryption/decryption cycles, secure memory
- **Security Tests**: Buffer overflow protection, input validation, memory safety
- **Integration Tests**: CLI interface, command parsing, error handling
- **Cross-Platform**: macOS and Linux compatibility validation
- **Performance**: Benchmarking optimized builds

### Code Quality Tools

**Static Analysis:**

- **clang-tidy**: Comprehensive static analysis with security checks
- **AddressSanitizer**: Runtime memory error detection
- **UndefinedBehaviorSanitizer**: Undefined behavior detection

**Code Standards:**

- **C11 Standard**: Modern C with security-focused practices
- **LLVM Style**: Consistent formatting with clang-format
- **Security Review**: All cryptographic operations independently verified

---

## Project Structure

```sh
c-chat/
├── 📁 src/                   # Core application source
│   ├── main.c               # Application entry point & CLI argument parsing
│   ├── interface.c          # Interactive CLI interface & command handling
│   ├── user.c               # User registration, login & account management
│   ├── chat.c               # Secure chat sessions & message handling
│   ├── crypto.c             # Cryptographic operations (libsodium wrapper)
│   ├── network.c            # Network communication & server interaction
│   └── utils.c              # Utility functions & security helpers
├── 📁 include/              # Header files
│   └── c-chat.h            # Main header with all declarations
├── 📁 tests/                # Comprehensive test suite
│   └── test_basic.c        # Core functionality & security tests
├── 📁 build/                # Build artifacts (auto-generated)
│   ├── debug/              # Debug builds with sanitizers
│   ├── release/            # Optimized production builds
│   └── profile/            # Profiling builds
├── Makefile             # Advanced build system
└── 📄 README.md            # This documentation
```

### Technology Stack

- **Language**: C11 Standard with ARM64-specific optimizations
- **Cryptography**: libsodium (NaCl) - industry-standard secure crypto library
- **Compiler**: Clang with security hardening flags
- **Build**: Advanced Makefile with cross-platform support
- **Testing**: Custom test framework with security validation
- **Platforms**: macOS (primary), Linux, BSD support

---

## Advanced Configuration

### Environment Variables

```bash
# Optional: Custom keys directory
export CCHAT_KEYS_DIR="$HOME/.config/c-chat"

# Optional: Custom server settings
export CCHAT_SERVER_HOST="your-server.com"
export CCHAT_SERVER_PORT="8443"
```

### Build Customization

```bash
# Custom optimization levels
make OPTS="-O3 -march=native"

# Custom libsodium location
make SODIUM_INCLUDE="/usr/local/include" SODIUM_LIB="/usr/local/lib"

# Debug with specific sanitizers
make MODE=debug SANITIZERS="-fsanitize=address,leak"
```

---

## Troubleshooting

### Common Issues

**Build Problems:**

```bash
# libsodium not found
brew install libsodium        # macOS
sudo apt install libsodium-dev  # Ubuntu

# Clean rebuild
make clean && make

# Check dependencies
make sysinfo
```

**Key Issues:**

```bash
# Reset user keys ( destroys existing keys)
rm ~/.c-chat/username.keys

# Check key file permissions
ls -la ~/.c-chat/
```

**Test Failures:**

```bash
# Debug mode testing
make test MODE=debug

# Verbose test output
make test VERBOSE=1
```

### Performance Optimization

**Release vs Debug:**

- **Release**: Optimized for production use, maximum performance
- **Debug**: Includes sanitizers and debug info, slower but safer for development

**Memory Usage:**

- Typical usage: ~1-2MB RAM
- Key storage: ~200 bytes per user
- Message overhead: +48 bytes per encrypted message

---

## Contributing

We welcome contributions! Please ensure all security-related changes are thoroughly tested.

### Development Workflow

```bash
# Setup development environment
git clone https://github.com/dunamismax/c-chat.git
cd c-chat
make MODE=debug

# Before submitting PR
make format lint test
```

### Security Guidelines

- All cryptographic operations must use libsodium
- Sensitive data must be securely cleared after use
- Input validation required for all user-controlled data
- Memory safety paramount - use bounded string functions

---

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

---

## Support This Project

If you find C-Chat valuable for your secure communication needs, consider supporting its continued development:

<p align="center">
  <a href="https://www.buymeacoffee.com/dunamismax" target="_blank">
    <img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 60px !important;width: 217px !important;" />
  </a>
</p>

---

## Connect

<p align="center">
  <a href="https://twitter.com/dunamismax" target="_blank"><img src="https://img.shields.io/badge/Twitter-%231DA1F2.svg?&style=for-the-badge&logo=twitter&logoColor=white" alt="Twitter"></a>
  <a href="https://bsky.app/profile/dunamismax.bsky.social" target="_blank"><img src="https://img.shields.io/badge/Bluesky-blue?style=for-the-badge&logo=bluesky&logoColor=white" alt="Bluesky"></a>
  <a href="https://reddit.com/user/dunamismax" target="_blank"><img src="https://img.shields.io/badge/Reddit-%23FF4500.svg?&style=for-the-badge&logo=reddit&logoColor=white" alt="Reddit"></a>
  <a href="https://discord.com/users/dunamismax" target="_blank"><img src="https://img.shields.io/badge/Discord-dunamismax-7289DA.svg?style=for-the-badge&logo=discord&logoColor=white" alt="Discord"></a>
  <a href="https://signal.me/#p/+dunamismax.66" target="_blank"><img src="https://img.shields.io/badge/Signal-dunamismax.66-3A76F0.svg?style=for-the-badge&logo=signal&logoColor=white" alt="Signal"></a>
</p>

---

<p align="center">
  <strong> Built with Pure C & LibSodium for Maximum Security</strong><br>
  <sub>Production-grade, end-to-end encrypted communication for the command line</sub>
</p>
