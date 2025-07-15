# ARM64 Apple Silicon Optimized Makefile
# Maximum performance C build system for macOS M-series processors

# =============================================================================
# CORE CONFIGURATION
# =============================================================================

# Apple Silicon optimized toolchain
CC := clang
AR := ar
MODE ?= release

# Platform-specific optimizations
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Darwin)
  # Apple Silicon optimizations for macOS
  ifeq ($(UNAME_M),arm64)
    ARM64_OPTS := -mcpu=apple-m1 -mtune=apple-m1 -arch arm64
  else
    ARM64_OPTS := -march=native
  endif
else
  # Linux/other platforms
  ifeq ($(UNAME_M),aarch64)
    ARM64_OPTS := -march=native -mtune=native
  else ifeq ($(UNAME_M),x86_64)
    ARM64_OPTS := -march=native -mtune=native
  else
    ARM64_OPTS := -march=native
  endif
endif

FAST_MATH := -ffast-math -fno-math-errno -ffinite-math-only
VECTORIZE := -fvectorize -fslp-vectorize
UNROLL := -funroll-loops
INLINE := -finline-functions -finline-hint-functions
BRANCH := -falign-functions=32 -falign-loops=32

# Build modes
ifeq ($(MODE),debug)
  OPTS := -g3 -O0 -DDEBUG -fsanitize=address,undefined
else ifeq ($(MODE),profile)
  OPTS := -O2 -g -pg -fprofile-arcs -ftest-coverage
else
  OPTS := -O3 -DNDEBUG -flto=thin $(FAST_MATH) $(VECTORIZE) $(UNROLL) $(INLINE) $(BRANCH) -fomit-frame-pointer
endif

# Final flags optimized for M-series
CFLAGS := -std=c11 -Wall -Wextra $(ARM64_OPTS) $(OPTS) -fstack-protector-strong

# =============================================================================
# SMART DISCOVERY & PATHS
# =============================================================================

BUILD_DIR := build/$(MODE)
LIBS := $(shell find libs -name "*.c" | sed 's|libs/\([^/]*\)/.*|libs/\1|' | sort -u)
APPS := $(shell find apps -name "*.c" | sed 's|apps/[^/]*/\([^/]*\)/.*|\1|' | sort -u)
LIB_TARGETS := $(LIBS:libs/%=$(BUILD_DIR)/lib/lib%.a)
APP_TARGETS := $(APPS:%=$(BUILD_DIR)/bin/%)

# Auto-detect include paths
INCLUDES := $(shell find libs -name include -type d | sed 's/^/-I/')

# =============================================================================
# TARGETS
# =============================================================================

.PHONY: all libs apps tests build-tests clean install help
.DEFAULT_GOAL := all

# Parallel builds enabled by default
ifeq ($(UNAME_S),Darwin)
  MAKEFLAGS += -j$(shell sysctl -n hw.ncpu)
else
  MAKEFLAGS += -j$(shell nproc)
endif

all: libs apps
	@echo "✓ Build completed ($(MODE) mode) - Optimized for $(UNAME_S) $(UNAME_M)"

libs: $(LIB_TARGETS)
apps: $(APP_TARGETS)

build-tests: libs
	@echo "Building test suite..."
	@mkdir -p $(BUILD_DIR)/bin
	@for test_file in $$(find tests -name "*.c" -not -name "test_framework.c"); do \
		test_name=$$(basename $$test_file .c); \
		echo "Building test: $$test_name"; \
		$(CC) $(CFLAGS) $(INCLUDES) -Itests $$test_file tests/test_framework.c $(LIB_TARGETS) -o $(BUILD_DIR)/bin/$$test_name; \
	done
	@echo "✓ Test suite built"

# Smart library building
$(BUILD_DIR)/lib/lib%.a: libs/%
	@mkdir -p $(dir $@) $(BUILD_DIR)/obj
	@echo "Building library: $(notdir $@)"
	@for src in $$(find $< -name "*.c"); do \
		obj=$$(echo $$src | sed 's|libs/|$(BUILD_DIR)/obj/|' | sed 's|\.c|\.o|'); \
		mkdir -p $$(dirname $$obj); \
		$(CC) $(CFLAGS) $(INCLUDES) -c $$src -o $$obj; \
	done
	@$(AR) rcs $@ $$(find $(BUILD_DIR)/obj/$* -name "*.o")

# Smart application building  
$(BUILD_DIR)/bin/%: $(LIB_TARGETS)
	@mkdir -p $(dir $@) $(BUILD_DIR)/obj
	@echo "Building app: $*"
	@app_dir=$$(find apps -name "$*" -type d); \
	if [ -f "$$app_dir/src/$*.c" ]; then \
		$(CC) $(CFLAGS) $(INCLUDES) $$app_dir/src/$*.c $(LIB_TARGETS) -o $@; \
	else \
		$(CC) $(CFLAGS) $(INCLUDES) $$(find $$app_dir -name "*.c") $(LIB_TARGETS) -o $@; \
	fi



# =============================================================================
# DEVELOPMENT TARGETS  
# =============================================================================

clean:
	@rm -rf build
	@echo "✓ Cleaned"

install: all
	@sudo cp $(BUILD_DIR)/bin/* /usr/local/bin/ 2>/dev/null || true
	@sudo cp $(BUILD_DIR)/lib/* /usr/local/lib/ 2>/dev/null || true
	@echo "✓ Installed to /usr/local"

format:
	@find . -name "*.c" -o -name "*.h" | xargs clang-format -i --style=LLVM
	@echo "✓ Code formatted"

lint:
	@clang-tidy $(shell find . -name "*.c") -- $(CFLAGS) $(INCLUDES) 2>/dev/null || echo "○ clang-tidy not available"

test:
	@echo "Running tests via test runner..."
	@./scripts/run_tests.sh -m $(MODE)
	@echo "✓ Tests completed"

profile:
	@$(MAKE) MODE=profile all
	@echo "✓ Profile build ready - run apps to generate profiling data"

benchmark: MODE=release
benchmark: all
	@echo "Benchmarking ARM64 optimized binaries..."
	@for app in $(BUILD_DIR)/bin/*; do echo "$$app:"; time $$app --version 2>/dev/null || time $$app </dev/null || true; done

run-%: $(BUILD_DIR)/bin/%
	@./$<

sysinfo:
	@echo "System: $$(uname -srm)"
ifeq ($(UNAME_S),Darwin)
	@echo "CPU: $$(sysctl -n machdep.cpu.brand_string 2>/dev/null || echo 'Unknown')"
	@echo "Cores: $$(sysctl -n hw.ncpu 2>/dev/null || echo 'Unknown') ($$(sysctl -n hw.perflevel0.physicalcpu 2>/dev/null || echo '?') P + $$(sysctl -n hw.perflevel1.physicalcpu 2>/dev/null || echo '?') E)"
	@echo "Memory: $$(($$(sysctl -n hw.memsize 2>/dev/null || echo 0) / 1024 / 1024 / 1024))GB"
else
	@echo "CPU: $$(grep 'model name' /proc/cpuinfo 2>/dev/null | head -n1 | cut -d: -f2 | sed 's/^[ ]*//' || echo 'Unknown')"
	@echo "Cores: $$(nproc 2>/dev/null || echo 'Unknown')"
	@echo "Memory: $$(free -h 2>/dev/null | awk '/^Mem:/ {print $$2}' || echo 'Unknown')"
endif
	@echo "Compiler: $$($(CC) --version | head -n1)"
	@echo "Mode: $(MODE)"

help:
	@echo "ARM64 Apple Silicon C Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       Build everything (release mode)"
	@echo "  libs      Build libraries only"  
	@echo "  apps      Build applications only"
	@echo "  tests     Build and run tests"
	@echo "  clean     Clean build artifacts"
	@echo "  install   Install to /usr/local"
	@echo "  profile   Build with profiling"
	@echo "  benchmark Performance benchmark"
	@echo ""
	@echo "Modes:"
	@echo "  make MODE=release  Maximum performance (default)"
	@echo "  make MODE=debug    Debug with sanitizers"
	@echo "  make MODE=profile  Profiling enabled"
	@echo ""
	@echo "Apps: $(APPS)"
	@echo "Run:  make run-<app>"

# Colors for pretty output
GREEN := \033[32m
YELLOW := \033[33m
NC := \033[0m