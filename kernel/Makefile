# ClankerOS Kernel Makefile

# Include common configuration
include ../Makefile.common

TARGET = clankeros.bin
ARCH = i386

# Directories
ARCH_DIR = arch/$(ARCH)
CORE_DIR = core
INCLUDE_DIR = include

# Source files
ASM_SOURCES = $(wildcard $(ARCH_DIR)/*.s)
C_SOURCES = $(wildcard $(CORE_DIR)/*.c) $(wildcard $(ARCH_DIR)/*.c)

# Object files
ASM_OBJECTS = $(ASM_SOURCES:.s=.o)
C_OBJECTS = $(C_SOURCES:.c=.o)
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

# Get GCC's built-in include directory for freestanding headers
GCC_INCLUDE := $(shell $(CC) -print-file-name=include)

# Flags
CFLAGS = -std=c11 -ffreestanding -O2 -Wall -Wextra -Wpedantic
CFLAGS += -nostdlib -fno-builtin -fno-stack-protector
CFLAGS += -I$(INCLUDE_DIR) -I$(GCC_INCLUDE)
CFLAGS += -m32

ASFLAGS = --32

LDFLAGS = -T $(ARCH_DIR)/linker.ld -nostdlib -m elf_i386

# Default target
all: $(TARGET)

# Link kernel
$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Compile C sources
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble assembly sources
%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(TARGET)

# Debug info
debug:
	@echo "ASM_SOURCES: $(ASM_SOURCES)"
	@echo "C_SOURCES: $(C_SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"

.PHONY: all clean debug
