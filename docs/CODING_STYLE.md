# ClankerOS Coding Style Guide

## Brace Style

Use K&R (Kernighan & Ritchie) brace style:

```c
// Good
void FunctionName(void)
{
    if (condition) {
        DoSomething();
    } else {
        DoSomethingElse();
    }
}

// Bad
void FunctionName(void) {
    // ...
}
```

## Naming Conventions

### Symbol Naming
- **Public symbols** (accessible outside translation unit): PascalCase with prefix
- **Local variables**: camelCase (starts with lowercase)
- **Static globals**: camelCase (starts with lowercase)

### Namespace Prefixes

Use 1-2 letter prefixes to namespace public symbols. Prefer full names over acronyms.

| Prefix | Module | Example |
|--------|--------|---------|
| `K` | Kernel core | `KMain`, `KPanic` |
| `Vid` | Video/Display | `VidInitialize`, `VidPutChar` |
| `Mem` | Memory management | `MemAllocate`, `MemFree` |
| `Int` | Interrupts | `IntInitialize`, `IntRegister` |
| `Pic` | PIC (8259) | `PicInitialize`, `PicSendEOI` |
| `Pit` | PIT Timer | `PitInitialize`, `PitSetFrequency` |
| `Kbd` | Keyboard | `KbdInitialize`, `KbdGetChar` |
| `Gdt` | Global Descriptor Table | `GdtInitialize`, `GdtInstall` |
| `Idt` | Interrupt Descriptor Table | `IdtInitialize`, `IdtSetGate` |
| `Cl` | libclanker/libclankerk | `ClPrintFormatted`, `ClAllocateMemory` |
| `Cui` | libcui (GUI library) | `CuiCreateWindow`, `CuiDrawRectangle` |

**Note**: Shortening identifiers is acceptable when they become too long (e.g., `VidInit` vs `VidInitialize`).

### Examples

```c
// Public kernel function
void KMain(uint32_t magic, multiboot_info_t* mbootInfo);

// Public video functions
void VidInitialize(void);
void VidPutChar(char c);
void VidWriteString(const char* str);

// Static globals (camelCase)
static size_t terminalRow;
static size_t terminalColumn;
static uint16_t* terminalBuffer;

// Local variables (camelCase)
void SomeFunction(void)
{
    size_t index;
    uint8_t colorValue;
    const char* messageText;
}

// Private helper function (static, camelCase)
static uint16_t vgaEntry(unsigned char c, uint8_t color)
{
    uint16_t entryValue = c | (color << 8);
    return entryValue;
}
```

## File Organization

### Header Guards
Use `#ifndef` style with uppercase filename:
```c
#ifndef MULTIBOOT_H
#define MULTIBOOT_H
// ...
#endif /* MULTIBOOT_H */
```

### Include Order
1. Standard library headers (if any)
2. Project headers
3. Local headers

```c
#include <stdint.h>
#include <stddef.h>

#include "kernel.h"
#include "multiboot.h"
```

## Comments

- Use `//` for single-line comments
- Use `/* */` for multi-line comments and file headers
- Document all public functions

```c
/*
 * KMain - Kernel entry point
 *
 * Called by boot.s after stack setup
 */
void KMain(uint32_t magic, multiboot_info_t* mbootInfo)
{
    // Initialize subsystems
    VidInitialize();
    // ...
}
```

## Formatting

- **Indentation**: 4 spaces (no tabs)
- **Line length**: Aim for 80 characters, 100 max
- **Pointer style**: `type* name` (asterisk with type)

## Types

- Prefer fixed-width types: `uint32_t`, `int16_t`, etc.
- Use `size_t` for sizes/counts
- Use `bool` from `<stdbool.h>` for booleans

## Constants

- `#define` constants: UPPER_CASE_SNAKE
- `enum` values: UPPER_CASE_SNAKE
- `const` variables: camelCase for locals, PascalCase with prefix for public

```c
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_WHITE = 15,
};

// Local constant
const size_t vidWidth = 80;

// Public constant (if exported)
const size_t VidDefaultWidth = 80;
```

## Function Ordering

Within a file:
1. Includes
2. Defines and macros
3. Type definitions
4. Static (private) variables
5. Static (private) function declarations
6. Public function implementations
7. Static (private) function implementations
