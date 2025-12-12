# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ClankerOS is an experimental operating system built collaboratively by a human architect and Claude (AI). The human provides architectural direction and design decisions; Claude implements the code. This is a learning project exploring how well LLMs can handle low-level systems programming.

- **Target**: x86 (i386) primary, ARM planned
- **Bootloader**: Multiboot protocol (GRUB/Limine compatible)
- **Kernel**: Hybrid design
- **Languages**: C for kernel, C++ for userspace applications
- **Build System**: GNU Make

## Build Commands

```bash
# First-time setup: Build cross-compiler toolchain
./scripts/build-toolchain.sh
export PATH="$PWD/toolchain/bin:$PATH"

# Build kernel
make

# Run in QEMU (three options)
make run              # Basic VGA output (for human use)
make run-serial       # Serial output to stdio (for human debugging)
make debug            # With GDB server (-s -S)

# Clean build artifacts
make clean

# Documentation website
make website          # Generate website in website/
make serve-website    # Serve at http://localhost:8000
```

## Running and Testing (for Claude Code)

**IMPORTANT**: When Claude Code runs the kernel for testing, it should **always**:

1. **Use QEMU directly** (not `make run-serial`):
   ```bash
   qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio -append "earlycon"
   ```

2. **Always include the `earlycon` kernel parameter** to enable serial output. Without this, no output will be visible.

3. **Use serial output (`-serial stdio`)** to capture kernel logs and diagnostic information.

4. **Optionally add VGA display** for visual inspection (allows human to see VGA output):
   ```bash
   qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio -append "earlycon"
   # The display window will show VGA output automatically
   ```

5. **Pass additional kernel parameters** via `-append` as needed:
   ```bash
   # Test panic system
   qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio -append "earlycon testpanic"

   # Test page fault handler
   qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio -append "earlycon testpagefault"
   ```

**Why this matters**: The `make run-serial` target is designed for human developers and doesn't include the `earlycon` parameter that enables serial console output. Claude Code needs to see serial output to verify kernel functionality, so it must call QEMU directly with appropriate parameters.

## Architecture

### Directory Structure

```
kernel/
├── arch/i386/        # x86-specific: boot.s, GDT, IDT, ISR, IRQ, PIC, PIT
├── core/             # Architecture-independent: main.c
└── include/          # Public kernel headers

libraries/
└── libclankercommon/ # Shared library (printf, writer interface)
    ├── include/clc/  # Headers with Clc prefix
    └── src/          # Implementation

programs/             # Userspace programs (future)
scripts/              # Build scripts and utilities
docs/                 # Session notes and coding style
```

### Key Design Patterns

#### Writer Interface (Rust-inspired)

The codebase uses a trait-object-like pattern for output abstraction:

```c
// Generic writer interface (libraries/libclankercommon/include/clc/writer.h)
typedef struct {
    void (*putchar)(void* data, char c);
} ClcWriterVTable;

typedef struct {
    void* data;                      // Fat pointer data
    const ClcWriterVTable* vtable;   // Fat pointer vtable
} ClcWriter;
```

Example usage:
```c
ClcWriter* vgaWriter = VidGetWriter();    // Get VGA writer
ClcWriter* serialWriter = EConGetWriter(); // Get serial writer
ClcPrintfWriter(vgaWriter, "Booting %s\n", "ClankerOS");
```

This allows printf to work with any output device without coupling.

#### Interrupt Architecture

Hardware interrupts follow this flow:
1. **Assembly stub** (irq_stubs.s): Saves CPU state, pushes interrupt number
2. **Common handler** (irq.c): Dispatches to registered C callback
3. **Send EOI**: Acknowledges interrupt to PIC

IRQs 0-15 are remapped to interrupts 32-47 to avoid conflicts with CPU exceptions (0-31).

### Naming Conventions (Critical)

Follow these strictly per [docs/CODING_STYLE.md](docs/CODING_STYLE.md):

- **Public functions**: PascalCase with module prefix
  - `VidInitialize()`, `PitGetTicks()`, `MemAllocate()`
  - Kernel core uses `K` prefix: `KMain()`, `KPanic()`
  - libclankercommon uses `Clc` prefix: `ClcPrintfWriter()`

- **Local variables & static globals**: camelCase
  - `terminalRow`, `timerTicks`, `colorValue`

- **Private static functions**: camelCase
  - `static uint16_t vgaEntry(char c, uint8_t color)`

- **Constants & macros**: UPPER_SNAKE_CASE
  - `VGA_WIDTH`, `PIT_BASE_FREQ`, `IRQ0`

**Module prefixes**:
- `Vid` = Video/VGA, `Mem` = Memory, `Pic` = PIC controller, `Pit` = Timer
- `Gdt`/`Idt` = Descriptor tables, `Isr`/`Irq` = Interrupts
- `ECon` = Early console (serial), `Clc` = libclankercommon

### Brace Style

K&R style (Kernighan & Ritchie):
```c
void FunctionName(void)
{
    if (condition) {
        DoSomething();
    } else {
        DoSomethingElse();
    }
}
```

Opening brace on same line for control structures, new line for functions.

### x86 I/O Port Access

Use direct instruction names in [kernel/include/x86.h](kernel/include/x86.h):
```c
outb(PIC1_COMMAND, 0x20);    // NOT IoWriteByte()
uint8_t value = inb(port);   // NOT IoReadByte()
io_wait();                   // Short delay via port 0x80
```

This was explicitly requested by the human architect - instruction wrappers should use actual instruction names.

## Current Implementation Status

**Completed** (as of Session 3):
- Multiboot boot via GRUB
- VGA text mode with scrolling/wrapping
- GDT (Global Descriptor Table)
- IDT (Interrupt Descriptor Table)
- ISR (Interrupt Service Routines) for CPU exceptions
- IRQ handlers for hardware interrupts
- PIC (8259) initialization and control
- PIT (Programmable Interval Timer) at 100 Hz
- Early console (COM1 serial port)
- Generic printf library with writer interface

**Next Milestone** (TODO.md): Memory Management
- Physical memory allocator (bitmap or stack-based)
- Parse multiboot memory map
- Virtual memory (paging)
- Kernel heap (kmalloc/kfree)

## Important Technical Details

### Multiboot Information

The bootloader passes `multiboot_info_t*` to `KMain()`. Key fields:
- `mmap_addr` / `mmap_length`: Physical memory map
- `mem_lower` / `mem_upper`: Available RAM
- See [kernel/include/multiboot.h](kernel/include/multiboot.h)

### Interrupt Numbering

- **0-31**: CPU exceptions (divide by zero, page fault, etc.)
- **32-47**: Hardware IRQs 0-15 (remapped by PIC)
  - IRQ0 (32) = PIT timer
  - IRQ1 (33) = Keyboard
  - IRQ2 (34) = PIC cascade
  - etc.

### Memory Layout

Currently using identity mapping (virtual = physical). Paging not yet enabled.

- Kernel loaded at ~1MB by bootloader
- VGA buffer at 0xB8000
- Stack set up by boot.s

## Development Workflow

1. **Read** [TODO.md](TODO.md) for current milestone and task list
2. **Follow** coding style in [docs/CODING_STYLE.md](docs/CODING_STYLE.md)
3. **Consult** session notes in `docs/sessions/` for context on past decisions
4. **Test** changes with `make run-serial` to see serial output
5. **Update** TODO.md and README.md when completing milestones
6. **Copy** Claude conversation data before commits: `scripts/copy-claude-data.sh`

## Project Philosophy

- **Pragmatic simplicity**: Don't over-engineer. Build what's needed.
- **Human-directed architecture**: The human decides design patterns and structure; Claude implements.
- **K&R style**: Clean, readable C with consistent naming conventions.
- **Learning-focused**: This is an educational project to explore LLM capabilities in systems programming.

Avoid backwards-compatibility hacks, premature abstractions, or unnecessary complexity. If something is unused, delete it completely.
