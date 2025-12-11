# ClankerOS Setup Guide

## Project Structure

```
clankeros/
├── kernel/           - Kernel source code
│   ├── arch/        - Architecture-specific code
│   │   └── i386/    - x86 32-bit implementation
│   ├── core/        - Core kernel functionality
│   └── include/     - Kernel headers
├── toolchain/       - Cross-compiler toolchain (target directory)
├── programs/        - Userspace programs
│   └── hello/       - Hello world example
├── libraries/       - System libraries
│   ├── libclanker/  - System library (userspace)
│   ├── libclankerk/ - Kernel library
│   └── libcui/      - GUI library
└── scripts/         - Build and utility scripts
    ├── build-toolchain.sh - Build i386 cross-compiler
    ├── bin2c.py          - Convert binary to C header
    └── mkdisk.py         - Create disk images (stub)
```

## Architecture

- **Target**: x86 (i386) primary, ARM planned
- **Bootloader**: Multiboot protocol (GRUB/Limine compatible)
- **Kernel Design**: Hybrid kernel
- **Languages**: C for kernel, C++ for applications
- **Build System**: GNU Make

## Building

### 1. Build Toolchain (First Time Only)

```bash
./scripts/build-toolchain.sh
export PATH="$PWD/toolchain/bin:$PATH"
```

### 2. Build Kernel

```bash
make
```

### 3. Run in QEMU

```bash
make run              # Basic run
make run-serial       # With serial output
make debug            # With GDB server
```

## Current Status

- [x] Project structure
- [x] Toolchain build script
- [x] Basic Multiboot kernel
- [x] VGA text mode output
- [ ] Interrupt handling
- [ ] Memory management
- [ ] Process management
- [ ] Virtual File System
- [ ] Device drivers
- [ ] Userspace

## Requirements

- GCC
- GNU Make
- QEMU (for testing)
- wget (for toolchain download)
