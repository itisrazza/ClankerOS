# ClankerOS Development TODO

## Project Setup ✅

- [x] Create top-level directory structure (kernel, toolchain, programs, libraries, scripts)
- [x] Set up kernel/ directory with basic structure (arch/i386, include, core)
- [x] Create toolchain/ directory with build scripts for i386 cross-compiler
- [x] Set up programs/ directory with hello world stub
- [x] Create libraries/ structure (libclanker, libclankerk, libcui)
- [x] Add scripts/ directory with binary-to-C-header converter script
- [x] Add scripts/ directory with disk image creation script
- [x] Create root Makefile for building the entire OS
- [x] Create kernel Makefile with Multiboot support
- [x] Add multiboot header and basic boot assembly stub
- [x] Basic VGA text mode output
- [x] Successful boot in QEMU

## Core Kernel Infrastructure

### CPU & Low-Level
- [x] Global Descriptor Table (GDT) setup
- [x] Interrupt Descriptor Table (IDT) setup
- [x] Interrupt Service Routines (ISRs) for CPU exceptions
- [x] Interrupt Request (IRQ) handlers
- [x] Programmable Interrupt Controller (PIC) initialization
- [x] System timer (PIT) implementation

### Memory Management
- [ ] Physical memory manager (bitmap/stack allocator)
- [ ] Virtual memory manager (paging)
- [ ] Kernel heap allocator (kmalloc/kfree)
- [ ] Memory mapping utilities
- [ ] Parse multiboot memory map

### Input/Output
- [x] Serial port driver (for debugging) - EarlyConsole via COM1
- [ ] Keyboard driver (PS/2)
- [ ] Improved VGA driver (scrolling, colors, cursor)
- [x] Basic printf/logging infrastructure - libclankercommon with writer interface

## System Calls & Process Management
- [ ] System call interface
- [ ] Process/task structure
- [ ] Context switching
- [ ] Basic scheduler (round-robin)
- [ ] User mode transition
- [ ] ELF loader for user programs

## File System & Storage
- [ ] Virtual File System (VFS) layer
- [ ] Initial RAM disk (initrd) support
- [ ] Simple filesystem implementation (tmpfs or similar)
- [ ] Block device interface
- [ ] ATA/IDE driver (basic)

## Device Drivers
- [ ] PCI enumeration
- [ ] RTC (Real-Time Clock) driver
- [ ] Mouse driver (PS/2)

## Userspace Libraries

### Core Libraries
- [x] Common library (libclankercommon) - printf, writer interface, shared utilities
- [ ] Kernel library (libclankerk) - kernel-specific utilities and data structures
- [ ] Native system API (libclanker) - ClankerOS native API and system call wrappers
- [ ] C standard library (libc) - ANSI C standard library implementation
- [ ] POSIX compatibility (libposix) - POSIX API layer on top of libclanker

### Applications
- [ ] Basic shell
- [ ] Essential utilities (ls, cat, echo, etc.)

## GUI (libcui)
- [ ] Framebuffer support (VESA/GOP)
- [ ] Basic window manager
- [ ] Widget toolkit
- [ ] Font rendering
- [ ] Event system

## Advanced Features
- [ ] Multi-core support (SMP)
- [ ] Network stack (TCP/IP)
- [ ] USB support
- [ ] Sound driver
- [ ] ACPI support

## ARM Port
- [ ] ARM bootloader
- [ ] ARM MMU setup
- [ ] ARM interrupt handling
- [ ] ARM-specific drivers

## Architecture Notes

- **Target**: x86 (i386) primary, ARM secondary
- **Bootloader**: Multiboot protocol (GRUB/Limine compatible)
- **Toolchain**: GCC cross-compiler
- **Kernel Design**: Hybrid kernel
- **Languages**: C for kernel, C++ for applications
- **Build System**: GNU Make

## Current Status

**Milestone 1: Boot & Basic Output** ✅ COMPLETE (Session 1)
- Successfully boots via Multiboot
- VGA text output working
- Build system functional

**Milestone 2: Printf & Core Infrastructure** ✅ COMPLETE (Session 2)
- GDT/IDT/ISR setup complete
- Generic printf library (libclankercommon) with writer interface
- Early console (COM1 serial) for debugging
- Formatted kernel output with hex display
- Code organization improvements (clc/ namespace)

**Milestone 3: Interrupts & Timing** ✅ COMPLETE (Session 3)
- IRQ handler infrastructure (interrupts 32-47)
- PIC initialization (8259 chip)
- PIT timer setup (100 Hz)
- Hardware interrupts working
- x86 instruction wrappers (inb, outb, io_wait)

**Next Milestone: Memory Management**
- Physical memory manager (bitmap/stack allocator)
- Parse multiboot memory map
- Virtual memory manager (paging)
- Kernel heap allocator (kmalloc/kfree)
