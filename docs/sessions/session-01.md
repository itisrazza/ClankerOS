# Session 1: Foundation and First Boot

**Date**: 2025-12-11
**Status**: ✅ Complete
**Milestone**: Boot & Basic Output

## Overview

First session establishing the ClankerOS project from an empty directory to a bootable kernel running in QEMU. This session focused on project architecture, build system setup, and getting the most minimal kernel to display output.

## Accomplishments

### Project Structure
- Created top-level directory organization:
  - `kernel/` - Kernel source code with arch-specific subdirs
  - `toolchain/` - Cross-compiler installation target
  - `programs/` - Userspace programs (hello world stub)
  - `libraries/` - System libraries (libclanker, libclankerk, libcui)
  - `scripts/` - Build and utility scripts
  - `docs/` - Documentation

### Build System
- Implemented `Makefile.common` for shared toolchain configuration
  - Auto-detects project root
  - Supports `CLANKEROS_TOOLCHAIN` environment variable
  - Falls back to system PATH if toolchain not found
  - All Makefiles can run standalone
- Created root Makefile with targets: build, clean, run, debug
- Set up kernel Makefile with proper i386 cross-compilation flags
- Fixed GCC include path issue for freestanding headers

### Toolchain
- Wrote `build-toolchain.sh` to build i386-elf cross-compiler
  - Downloads and builds binutils 2.41
  - Downloads and builds GCC 13.2.0 with C/C++ support
  - Installs to local `toolchain/` directory
  - Shellcheck clean

### Utilities
- `bin2c.py` - Converts binary files to C header arrays (MicroPython compatible)
- `mkdisk.py` - Disk image creator (stubbed for now, using QEMU multiboot)

### Kernel Implementation
- **Multiboot compliance**: Proper header and boot stub
- **Boot sequence**:
  - `boot.s` - Assembly entry point with stack setup
  - Calls `kernel_main()` with multiboot info
- **VGA text mode driver**:
  - 80x25 text display at 0xB8000
  - Color support via VGA attributes
  - Character output with newline handling
- **Linker script**: Memory layout starting at 1MB physical

### Documentation
- README.md explaining the AI-built OS experiment
- SETUP.md with build instructions and project info
- TODO.md with comprehensive roadmap
- MIT License
- Session logs (this document)

## Technical Details

### Architecture Decisions
- **Target**: i386 (32-bit x86) as primary platform
- **Bootloader**: Multiboot protocol for GRUB/Limine/QEMU compatibility
- **Kernel Design**: Hybrid kernel architecture
- **Languages**: C for kernel, C++ support for userspace
- **Build System**: GNU Make

### Key Files Created
```
clankeros/
├── Makefile.common              # Shared build configuration
├── Makefile                     # Root build system
├── kernel/
│   ├── Makefile                # Kernel build
│   ├── arch/i386/
│   │   ├── boot.s              # Multiboot entry point
│   │   └── linker.ld           # Memory layout
│   ├── core/
│   │   └── main.c              # Kernel entry and VGA driver
│   └── include/
│       └── multiboot.h         # Multiboot spec definitions
├── scripts/
│   ├── build-toolchain.sh      # Toolchain builder
│   ├── bin2c.py                # Binary to C converter
│   └── mkdisk.py               # Disk image creator (stub)
└── docs/
    └── sessions/
        └── session-01.md       # This file
```

## Challenges & Solutions

### Challenge 1: Makefile Environment Setup
**Problem**: How to make Makefiles work standalone while sharing toolchain config?

**Solution**: Created `Makefile.common` that:
- Auto-detects project root by walking up directories
- Respects environment variable overrides
- Provides sensible fallbacks
- Can be included from any depth

### Challenge 2: Missing Standard Headers
**Problem**: Using `-nostdinc` prevented access to `stdint.h` and `stddef.h`

**Solution**:
- Removed `-nostdinc` flag
- Added GCC's built-in include directory explicitly: `-I$(GCC_INCLUDE)`
- This gives us freestanding headers without full libc

### Challenge 3: Toolchain Location Flexibility
**Problem**: Users might have cross-compiler in different locations

**Solution**: Priority chain:
1. `CLANKEROS_TOOLCHAIN` environment variable
2. Local `toolchain/` directory
3. System PATH (`i386-elf-gcc`)

## Test Results

### Build Test
```bash
$ make kernel
make[1]: Entering directory '/home/razz/src/razza/clankeros/kernel'
i386-elf-gcc ... -c core/main.c -o core/main.o
i386-elf-ld ... -o clankeros.bin arch/i386/boot.o core/main.o
make[1]: Leaving directory '/home/razz/src/razza/clankeros/kernel'
```
✅ Clean build with no errors

### QEMU Test
```bash
$ make run
qemu-system-i386 -kernel kernel/clankeros.bin
```

Output in QEMU window:
```
ClankerOS v0.1.0
Booting kernel...

Welcome to ClankerOS!
```
✅ Successful boot and text output

## Lines of Code
- **C Code**: ~120 lines (main.c)
- **Assembly**: ~40 lines (boot.s)
- **Makefiles**: ~150 lines
- **Scripts**: ~200 lines
- **Headers**: ~50 lines
- **Total**: ~560 lines of code

## Next Session Goals

Based on TODO.md, the next milestone is **Core Infrastructure**:

1. **GDT (Global Descriptor Table)** - Set up protected mode segments
2. **IDT (Interrupt Descriptor Table)** - Enable interrupt handling
3. **ISRs** - CPU exception handlers (divide by zero, page fault, etc.)
4. **IRQ Handlers** - Hardware interrupts
5. **PIC Setup** - Program the 8259 Programmable Interrupt Controller
6. **Basic Timer** - PIT (Programmable Interval Timer) for scheduling

With interrupts working, we can then move to:
- Keyboard input
- Memory management
- System calls

## Notes & Observations

- The build system came together cleanly with good separation of concerns
- VGA text mode is surprisingly straightforward
- Multiboot makes testing in QEMU very easy (no bootloader needed)
- The project structure scales well - easy to add new modules
- MicroPython compatibility constraint on scripts was painless

## References

- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
- [OSDev Wiki](https://wiki.osdev.org/)
- [Intel x86 Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

---

**Session Duration**: ~1 hour
**Commits**: Initial project setup
**Status**: Ready for Session 2
