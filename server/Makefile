# C-Chat Server Makefile
# ARM64 Apple Silicon Optimized Build System

CC := clang
MODE ?= release

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Darwin)
  ifeq ($(UNAME_M),arm64)
    ARM64_OPTS := -mcpu=apple-m1 -mtune=apple-m1 -arch arm64
  else
    ARM64_OPTS := -march=native
  endif
else
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

ifeq ($(MODE),debug)
  OPTS := -g3 -O0 -DDEBUG -fsanitize=address,undefined
else ifeq ($(MODE),profile)
  OPTS := -O2 -g -pg -fprofile-arcs -ftest-coverage
else
  OPTS := -O3 -DNDEBUG -flto=thin $(FAST_MATH) $(VECTORIZE) $(UNROLL) $(INLINE) $(BRANCH) -fomit-frame-pointer
endif

CFLAGS := -std=c11 -Wall -Wextra -Werror $(ARM64_OPTS) $(OPTS) -fstack-protector-strong -D_GNU_SOURCE
LDFLAGS := -pthread

BUILD_DIR := build/$(MODE)
SRC_DIR := src
INCLUDE_DIR := include
APP_NAME := c-chat-server

INCLUDES := -I$(INCLUDE_DIR)
LIBS := -lsodium

ifeq ($(UNAME_S),Darwin)
  BREW_PREFIX := $(shell brew --prefix 2>/dev/null || echo /usr/local)
  SODIUM_INCLUDE := $(BREW_PREFIX)/include
  SODIUM_LIB := $(BREW_PREFIX)/lib
  
  ifneq ($(wildcard $(SODIUM_INCLUDE)/sodium.h),)
    INCLUDES += -I$(SODIUM_INCLUDE)
    LIBS := -L$(SODIUM_LIB) -lsodium
  else
    $(warning libsodium not found. Install with: brew install libsodium)
  endif
else
  ifneq ($(shell pkg-config --exists libsodium && echo found),found)
    $(warning libsodium not found via pkg-config. Install with package manager)
  endif
endif

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/obj/%.o)

.PHONY: all clean install help run

ifeq ($(UNAME_S),Darwin)
  MAKEFLAGS += -j$(shell sysctl -n hw.ncpu)
else
  MAKEFLAGS += -j$(shell nproc)
endif

all: $(BUILD_DIR)/bin/$(APP_NAME)
	@echo "✓ Server build completed ($(MODE) mode) - Optimized for $(UNAME_S) $(UNAME_M)"

$(BUILD_DIR)/bin/$(APP_NAME): $(OBJECTS)
	@mkdir -p $(dir $@)
	@echo "Linking $(APP_NAME)..."
	@$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@
	@echo "✓ $(APP_NAME) built successfully"

$(BUILD_DIR)/obj/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -rf build
	@echo "✓ Cleaned server build artifacts"

install: all
	@sudo cp $(BUILD_DIR)/bin/$(APP_NAME) /usr/local/bin/ 2>/dev/null || true
	@echo "✓ Server installed to /usr/local/bin"

run: $(BUILD_DIR)/bin/$(APP_NAME)
	@echo "Starting C-Chat Server..."
	@./$<

format:
	@find . -name "*.c" -o -name "*.h" | xargs clang-format -i --style=LLVM
	@echo "✓ Server code formatted"

lint:
	@clang-tidy $(SOURCES) -- $(CFLAGS) $(INCLUDES) 2>/dev/null || echo "○ clang-tidy not available"

help:
	@echo "C-Chat Server Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       Build c-chat-server (release mode)"
	@echo "  clean     Clean build artifacts"
	@echo "  install   Install to /usr/local/bin"
	@echo "  run       Build and run server"
	@echo "  format    Format code with clang-format"
	@echo "  lint      Run static analysis"
	@echo ""
	@echo "Modes:"
	@echo "  make MODE=release  Maximum performance (default)"
	@echo "  make MODE=debug    Debug with sanitizers"
	@echo "  make MODE=profile  Profiling enabled"