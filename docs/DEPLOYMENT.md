# Production Deployment Guide

## Overview

This guide provides comprehensive instructions for deploying the C monorepo applications in production environments with security, performance, and reliability considerations.

## Pre-Deployment Checklist

### 1. Security Verification

- [ ] All security patches applied
- [ ] Security tests pass
- [ ] No hardcoded credentials
- [ ] Input validation implemented
- [ ] Error handling reviewed
- [ ] Logging configured securely

### 2. Performance Testing

- [ ] Load testing completed
- [ ] Memory usage profiled
- [ ] Resource limits defined
- [ ] Optimization flags applied
- [ ] Performance benchmarks met

### 3. Quality Assurance

- [ ] All unit tests pass
- [ ] Integration tests pass
- [ ] Code review completed
- [ ] Documentation updated
- [ ] Static analysis clean

## Build Configuration

### Production Build

```bash
# Clean build with optimizations
make clean
make MODE=release

# Verify build
make test MODE=release
make security
```

### Compiler Optimizations

```makefile
# Production flags (already configured)
RELEASE_FLAGS := -O3 -DNDEBUG -flto -ffast-math -funroll-loops -fomit-frame-pointer

# Security hardening
SECURITY_FLAGS := -fstack-protector-strong -D_FORTIFY_SOURCE=2
```

### Strip Debug Symbols

```bash
# Reduce binary size for production
strip build/release/bin/*
```

## System Requirements

### Minimum Requirements

- **OS**: Linux, macOS, or Unix-like system
- **Architecture**: ARM64 (Apple Silicon) or x86_64
- **Memory**: 512MB RAM minimum
- **Storage**: 100MB available space
- **Compiler**: Clang 15+ or GCC 11+

### Recommended Requirements

- **Memory**: 2GB RAM for optimal performance
- **Storage**: 1GB available space
- **CPU**: Multi-core processor for parallel operations

## Installation Methods

### 1. System-Wide Installation

```bash
# Install to /usr/local (requires sudo)
make install

# Verify installation
which calculator
calculator --help
```

### 2. User-Local Installation

```bash
# Install to ~/local
make install-local

# Add to PATH
echo 'export PATH=$HOME/local/bin:$PATH' >> ~/.bashrc
source ~/.bashrc
```

### 3. Container Deployment

```dockerfile
FROM alpine:latest

# Install dependencies
RUN apk add --no-cache gcc musl-dev make

# Copy source code
COPY . /app
WORKDIR /app

# Build and install
RUN make MODE=release
RUN make install

# Create non-root user
RUN adduser -D -s /bin/sh appuser
USER appuser

# Expose applications
EXPOSE 8080
CMD ["calculator"]
```

### 4. Package Distribution

```bash
# Create distribution package
make clean
make MODE=release
tar -czf c-monorepo-v1.0.tar.gz build/release/bin/ docs/ LICENSE

# Checksum for verification
sha256sum c-monorepo-v1.0.tar.gz > c-monorepo-v1.0.tar.gz.sha256
```

## Configuration Management

### Environment Variables

```bash
# Application configuration
export CALC_PRECISION=10
export FILE_UTILS_SAFE_MODE=1
export TEXT_PROC_MAX_LINES=1000

# Security settings
export SECURE_MODE=1
export LOG_LEVEL=INFO
export MAX_INPUT_SIZE=1024
```

### Configuration Files

Create `/etc/c-monorepo/config.conf`:

```ini
[security]
safe_mode=true
max_input_size=1024
path_validation=strict

[performance]
buffer_size=4096
max_files=100
timeout=30

[logging]
level=info
destination=/var/log/c-monorepo.log
rotate=daily
```

## Security Hardening

### 1. File Permissions

```bash
# Set secure permissions
chmod 755 /usr/local/bin/calculator
chmod 755 /usr/local/bin/file_utils
chmod 755 /usr/local/bin/text_processor

# Remove execute for others if needed
chmod 750 /usr/local/bin/*
```

### 2. SELinux/AppArmor

```bash
# SELinux context (RHEL/CentOS)
setsebool -P allow_execheap=0
setsebool -P allow_execmem=0

# AppArmor profile (Ubuntu/Debian)
aa-genprof calculator
aa-enforce /etc/apparmor.d/calculator
```

### 3. Resource Limits

```bash
# Set ulimits for security
ulimit -f 10240      # File size limit (10MB)
ulimit -v 1048576    # Virtual memory limit (1GB)
ulimit -u 100        # Process limit

# Systemd service limits
cat > /etc/systemd/system/calculator.service << EOF
[Unit]
Description=Calculator Service
After=network.target

[Service]
Type=simple
User=calculator
Group=calculator
ExecStart=/usr/local/bin/calculator
Restart=always
RestartSec=10

# Security restrictions
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadOnlyPaths=/

# Resource limits
LimitNOFILE=1024
LimitNPROC=64
LimitFSIZE=10485760

[Install]
WantedBy=multi-user.target
EOF
```

## Monitoring and Logging

### 1. Application Monitoring

```bash
# Monitor resource usage
watch -n 5 'ps aux | grep -E "(calculator|file_utils|text_processor)"'

# Memory monitoring
valgrind --tool=massif ./calculator

# Performance monitoring
perf record ./calculator
perf report
```

### 2. Log Configuration

```c
// Application logging (example)
#ifdef PRODUCTION
    openlog("calculator", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Calculator started");
#endif
```

### 3. Log Rotation

```bash
# Configure logrotate
cat > /etc/logrotate.d/c-monorepo << EOF
/var/log/c-monorepo.log {
    daily
    rotate 7
    compress
    delaycompress
    missingok
    create 0644 root root
    postrotate
        systemctl reload c-monorepo
    endscript
}
EOF
```

## Performance Tuning

### 1. Compiler Optimizations

```bash
# Profile-guided optimization
make MODE=profile
# Run typical workload
./build/profile/bin/calculator < benchmark_input.txt
# Rebuild with profile data
make MODE=release EXTRA_FLAGS="-fprofile-use"
```

### 2. System Tuning

```bash
# Increase file descriptor limits
echo "* soft nofile 65536" >> /etc/security/limits.conf
echo "* hard nofile 65536" >> /etc/security/limits.conf

# Optimize TCP settings for network applications
echo "net.core.rmem_default = 262144" >> /etc/sysctl.conf
echo "net.core.wmem_default = 262144" >> /etc/sysctl.conf
```

### 3. Memory Optimization

```c
// Compile-time memory optimization
#ifdef PRODUCTION
    #define VECTOR_INITIAL_CAPACITY 32
    #define MAX_INPUT_BUFFER 1024
#else
    #define VECTOR_INITIAL_CAPACITY 8
    #define MAX_INPUT_BUFFER 256
#endif
```

## Backup and Recovery

### 1. Data Backup

```bash
# Backup configuration
tar -czf config-backup-$(date +%Y%m%d).tar.gz /etc/c-monorepo/

# Backup logs
rsync -av /var/log/c-monorepo.log* backup-server:/backups/logs/
```

### 2. Recovery Procedures

```bash
# Restore from backup
tar -xzf config-backup-20240101.tar.gz -C /

# Verify installation
make test
systemctl status calculator
```

## Troubleshooting

### Common Issues

#### 1. Permission Denied

```bash
# Check file permissions
ls -la /usr/local/bin/calculator
# Fix permissions
chmod +x /usr/local/bin/calculator
```

#### 2. Library Not Found

```bash
# Check library paths
ldd /usr/local/bin/calculator
# Add library path
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

#### 3. Memory Issues

```bash
# Check memory usage
free -h
# Monitor for memory leaks
valgrind --leak-check=full ./calculator
```

### Debug Mode

```bash
# Enable debug mode
make MODE=debug
export DEBUG=1
./build/debug/bin/calculator
```

## Maintenance

### Regular Tasks

- **Daily**: Monitor logs and resource usage
- **Weekly**: Update security patches
- **Monthly**: Performance review and optimization
- **Quarterly**: Full security audit

### Update Procedures

```bash
# 1. Backup current installation
cp -r /usr/local/bin/calculator /backup/

# 2. Download and verify new version
wget https://releases.example.com/c-monorepo-v1.1.tar.gz
sha256sum -c c-monorepo-v1.1.tar.gz.sha256

# 3. Test in staging environment
tar -xzf c-monorepo-v1.1.tar.gz
make test

# 4. Deploy to production
make install
systemctl restart calculator
```

## Support and Documentation

### Resources

- **Documentation**: `/usr/local/share/doc/c-monorepo/`
- **Configuration Examples**: `/usr/local/share/c-monorepo/examples/`
- **Troubleshooting**: This guide and `docs/TROUBLESHOOTING.md`

### Contact

- **Technical Support**: `support@[domain]`
- **Security Issues**: `security@[domain]`
- **Bug Reports**: GitHub Issues

---

**Last Updated**: [Current Date]  
**Version**: 1.0  
**Environment**: Production Ready
