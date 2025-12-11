# Makefile for libclankercommon - ClankerOS common library

# Detect project root
PROJECT_ROOT := $(shell git rev-parse --show-toplevel 2>/dev/null || echo ../..)
include $(PROJECT_ROOT)/Makefile.common

# Directories
SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := build

# Source files
SRCS := $(SRC_DIR)/printf.c \
        $(SRC_DIR)/writers.c

OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Library output
LIB := libclankercommon.a

# Compiler flags
CFLAGS := -std=c11 -ffreestanding -O2 -Wall -Wextra -Wpedantic \
          -nostdlib -fno-builtin -fno-stack-protector \
          -I$(INCLUDE_DIR) \
          -I$(GCC_INCLUDE) \
          -m32 -g

.PHONY: all clean

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(LIB)
