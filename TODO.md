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
- [ ] Global Descriptor Table (GDT) setup
- [ ] Interrupt Descriptor Table (IDT) setup
- [ ] Interrupt Service Routines (ISRs) for CPU exceptions
- [ ] Interrupt Request (IRQ) handlers
- [ ] Programmable Interrupt Controller (PIC) initialization
- [ ] System timer (PIT) implementation

### Memory Management
- [ ] Physical memory manager (bitmap/stack allocator)
- [ ] Virtual memory manager (paging)
- [ ] Kernel heap allocator (kmalloc/kfree)
- [ ] Memory mapping utilities
- [ ] Parse multiboot memory map

### Input/Output
- [ ] Serial port driver (for debugging)
- [ ] Keyboard driver (PS/2)
- [ ] Improved VGA driver (scrolling, colors, cursor)
- [ ] Basic printf/logging infrastructure

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

## Userspace
- [ ] C standard library implementation (libclanker)
- [ ] System call wrappers
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

**Milestone 1: Boot & Basic Output** ✅ COMPLETE
- Successfully boots via Multiboot
- VGA text output working
- Build system functional

**Next Milestone: Core Infrastructure**
- GDT/IDT setup
- Interrupt handling
- Memory management basics
