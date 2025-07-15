# Security Guidelines and Best Practices

## Overview

This C monorepo implements comprehensive security measures to prevent common vulnerabilities in C applications. This document outlines the security features, practices, and guidelines for maintaining and extending the codebase.

## Security Features Implemented

### 1. Buffer Overflow Protection

#### Text Processor (`text_processor.c`)

- **Dynamic memory allocation** instead of fixed-size stack buffers
- **Safe string operations** with bounds checking (`safe_strcpy`, `safe_strcat`)
- **Input length validation** before processing
- **Memory cleanup** on error conditions

```c
// SECURE: Dynamic allocation with bounds checking
char **lines = malloc(MAX_LINES * sizeof(char*));
if (safe_strcpy(lines[count], line_len, input_buffer) != 0) {
    // Handle error and cleanup
}
```

#### All Applications

- **Bounded input functions** (`fgets` with size limits)
- **String termination validation** using `strcspn`
- **Format string specification** in `sscanf` calls (`%255s` instead of `%s`)

### 2. Path Traversal Protection

#### File Utils (`file_utils.c`)

- **Path validation** functions reject dangerous patterns:
  - `../` directory traversal attempts
  - Absolute paths to system directories (`/etc/`, `/usr/`, etc.)
  - Null bytes and control characters
- **Canonical path resolution** using `realpath()`
- **Filename validation** against reserved names and dangerous characters
- **Atomic file operations** using temporary files

```c
// SECURE: Comprehensive path validation
static int validate_path(const char *path) {
    if (strstr(path, "../") != NULL) return -1;
    // Additional checks...
}
```

### 3. Format String Vulnerability Prevention

#### Calculator (`calculator.c`)

- **Safe input parsing** with custom tokenization
- **Input validation** before string operations
- **Format string character detection** (`%` filtering)
- **Bounds-checked parsing** using `strtod` with error checking

```c
// SECURE: Custom parsing instead of vulnerable sscanf
static int safe_parse_input(const char *input, char *op_str, size_t op_size,
                           double *num1, double *num2);
```

### 4. Integer Overflow Protection

#### Math Utils (`math_utils.c`)

- **Overflow detection** in arithmetic operations
- **Safe multiplication** and **addition** functions
- **Range limits** for factorial (≤20) and Fibonacci (≤92)
- **Bounds checking** before mathematical operations

```c
// SECURE: Overflow-safe multiplication
static int safe_multiply_long_long(long long a, long long b, long long *result) {
    if (a > LLONG_MAX / b) return -1; // Overflow detected
    *result = a * b;
    return 0;
}
```

### 5. Memory Safety

#### Vector Library (`vector.c`)

- **Null pointer validation** in all functions
- **Bounds checking** for array access
- **Safe reallocation** with error handling
- **Memory leak prevention** with proper cleanup

### 6. Shell Script Hardening

#### All Scripts (`build.sh`, `test.sh`, `clean.sh`)

- **Strict error handling**: `set -euo pipefail`
- **Secure IFS**: `IFS=$'\\n\\t'`
- **Variable quoting**: All variables properly quoted
- **Input validation**: Parameter validation before use

## Security Testing

### 1. Integration Tests

- **Buffer overflow tests**: Long input validation
- **Path traversal tests**: Malicious path rejection
- **Format string tests**: Format specifier injection attempts
- **Integer overflow tests**: Large number handling

### 2. Static Analysis

- **Compiler warnings**: `-Wall -Wextra -Wpedantic`
- **Security flags**: `-fstack-protector-strong -D_FORTIFY_SOURCE=2`
- **Address sanitizer**: `-fsanitize=address` in debug builds
- **Undefined behavior sanitizer**: `-fsanitize=undefined`

### 3. Manual Security Review

- Code review checklist for all changes
- Security-focused testing protocols
- Penetration testing guidelines

## Secure Coding Guidelines

### 1. Input Validation

- **Always validate input** before processing
- **Check buffer sizes** before string operations
- **Sanitize file paths** before file operations
- **Validate numeric inputs** for range and format

### 2. Memory Management

- **Initialize all variables** before use
- **Check malloc return values** for NULL
- **Free allocated memory** in all code paths
- **Use safe string functions** (`strncpy`, `snprintf`)

### 3. Error Handling

- **Check return values** of all system calls
- **Provide meaningful error messages** without information disclosure
- **Clean up resources** on error conditions
- **Log security events** appropriately

### 4. File Operations

- **Validate file paths** before access
- **Use temporary files** for atomic operations
- **Set restrictive permissions** on created files
- **Check file types** before processing

## Vulnerability Disclosure

### Reporting Security Issues

If you discover a security vulnerability:

1. **DO NOT** create a public issue
2. Email security reports to: `security@[domain]`
3. Include detailed reproduction steps
4. Allow reasonable time for response

### Response Process

1. **Acknowledgment** within 48 hours
2. **Initial assessment** within 1 week
3. **Fix development** and testing
4. **Coordinated disclosure** after fix deployment

## Security Maintenance

### Regular Security Tasks

- **Update dependencies** regularly
- **Review security advisories** for used tools
- **Run security scans** with updated tools
- **Audit access controls** and permissions

### Code Review Requirements

- **Security-focused review** for all changes
- **Two-person approval** for security-critical code
- **Automated security testing** with comprehensive test suite
- **Documentation updates** for security changes

## Compliance and Standards

### Security Standards

- **OWASP Top 10** - Protection against common vulnerabilities
- **CWE (Common Weakness Enumeration)** - Weakness prevention
- **SANS Top 25** - Most dangerous software errors
- **ISO 27001** - Information security management

### Audit Trail

- **Git history** maintains complete change log
- **Signed commits** for verification
- **Security test results** archived
- **Vulnerability assessments** documented

## Tools and Resources

### Security Tools

- **Static Analysis**: `clang-tidy`, `cppcheck`
- **Dynamic Analysis**: `valgrind`, `AddressSanitizer`
- **Fuzzing**: `AFL`, `libFuzzer`
- **Code Review**: Security-focused checklists

### Learning Resources

- [OWASP Secure Coding Practices](https://owasp.org/www-project-secure-coding-practices-quick-reference-guide/)
- [CERT C Coding Standard](https://wiki.sei.cmu.edu/confluence/display/c/SEI+CERT+C+Coding+Standard)
- [CWE/SANS Top 25](https://cwe.mitre.org/top25/)

## Contact Information

- **Security Team**: `security@[domain]`
- **General Contact**: `info@[domain]`
- **Documentation**: This file and inline code comments

---

**Last Updated**: [Current Date]  
**Version**: 1.0  
**Classification**: Public
