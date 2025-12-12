# ClankerOS

An experimental operating system built entirely by Claude, Anthropic's AI assistant.

## What is this?

ClankerOS is an experiment to see how an LLM would fare building an operating system from scratch. The name "Clanker" is a playful reference to Claude building this OS. While it won't be commercial-grade software, ClankerOS serves as a reflection of the current state of large language models and their ability to architect, design, and implement complex systems.

**Important Note**: This is a collaborative project where the human provides architectural direction and design decisions, while Claude implements the actual code. The human maintains control over the "what" and "why" (system architecture, design patterns, code organization), while Claude handles the "how" (implementation details, algorithms, code structure). Think of it as pair programming where one partner focuses on high-level design and the other on detailed implementation.

## The Goal

Build a functional operating system from an empty directory to a working desktop environment in a matter of days. The architecture and code you see here are the result of conversations between human and AI.

## Why?

This project explores several questions:
- Can an LLM architect a coherent operating system?
- How well can AI handle low-level systems programming?
- What does an AI-designed OS look like?
- Where do LLMs excel, and where do they struggle?

## Architecture

- **Kernel**: Hybrid design targeting x86 (i386) and ARM
- **Bootloader**: Multiboot protocol
- **Languages**: C for kernel, C++ for userspace applications
- **Build System**: GNU Make
- **Philosophy**: Pragmatic simplicity with room for experimentation

## Current Status

**Session 3 Complete** - Hardware Interrupts & Timer
- ✅ Boots via Multiboot on QEMU
- ✅ VGA text mode output with formatted printf
- ✅ GDT/IDT/ISR setup (CPU exception handling)
- ✅ IRQ handlers (hardware interrupts 32-47)
- ✅ PIC (8259) initialization and control
- ✅ PIT timer running at 100 Hz
- ✅ Generic printf library (libclankercommon) with writer interface pattern
- 🔄 Next: Memory management (physical allocator, paging)

**Resources:**
- [Session Notes](docs/sessions/) - Detailed development session logs
- [Development Roadmap](TODO.md) - Comprehensive TODO list
- [Project Wiki](https://github.com/itisrazza/ClankerOS/wiki) - Human-written analysis and insights about the development process

## Project Structure

```
clankeros/
├── kernel/              # Kernel source code
│   ├── arch/i386/      # x86 architecture-specific code
│   ├── core/           # Core kernel functions
│   └── include/        # Kernel headers
├── libraries/
│   └── libclankercommon/ # Shared library (printf, writer interface)
├── docs/
│   ├── sessions/       # Development session notes
│   └── CODING_STYLE.md # Coding conventions
├── scripts/            # Build and utility scripts
└── TODO.md            # Development roadmap
```

## Library Architecture

- **libclankercommon** - Shared utilities (printf, writer interface) used by kernel and userspace
- **libclankerk** (planned) - Kernel-specific utilities and data structures
- **libclanker** (planned) - ClankerOS native system API and syscall wrappers
- **libc** (planned) - ANSI C standard library implementation
- **libposix** (planned) - POSIX compatibility layer

## About

ClankerOS is built through collaborative conversation between human and AI. The human provides architectural guidance and design decisions, while Claude (Anthropic) implements the code. This README and all code in this repository are the result of this collaboration.

---

*"An OS by an AI, for science."*
