# Session 4: Memory Management and Kernel Command Line Arguments

**Date**: December 12, 2024
**Focus**: Implemented complete memory management subsystem and kernel command line argument parsing

## Overview

This session completed the memory management milestone (Phase 1 from IPC architecture) and added kernel command line argument support for runtime configuration. The kernel now has physical memory management, virtual memory with paging, a kernel heap allocator, and support for `earlycon` and `boottest` command line flags.

## What Was Implemented

### 1. Physical Memory Manager (PMM)

**Files**:
- `kernel/include/pmm.h` - PMM interface
- `kernel/core/pmm.c` - Bitmap-based page allocator

**Key Features**:
- Bitmap allocator for 4KB pages
- Parses multiboot memory map to identify available RAM
- Marks reserved regions (first 1MB, kernel, bitmap itself) as used
- Functions: `PmmInitialize()`, `PmmAllocPage()`, `PmmFreePage()`, `PmmGetTotalMemory()`, `PmmGetFreeMemory()`, `PmmGetUsedMemory()`

**Implementation Details**:
```c
// Bitmap operations
#define BITMAP_INDEX(page) ((page) / 32)
#define BITMAP_OFFSET(page) ((page) % 32)
#define BITMAP_SET(page) (pageBitmap[BITMAP_INDEX(page)] |= (1 << BITMAP_OFFSET(page)))
#define BITMAP_CLEAR(page) (pageBitmap[BITMAP_INDEX(page)] &= ~(1 << BITMAP_OFFSET(page)))
#define BITMAP_TEST(page) (pageBitmap[BITMAP_INDEX(page)] & (1 << BITMAP_OFFSET(page)))
```

**Testing**: Allocates pages, frees them, verifies reuse of freed pages. With 128 MB RAM: ~126 MB free, ~1 MB used (kernel + bitmap).

### 2. Paging (Virtual Memory)

**Files**:
- `kernel/include/paging.h` - Paging structures and interface
- `kernel/core/paging.c` - Page directory/table management

**Key Features**:
- Identity mapping for first 4MB (kernel space)
- Page directory and page table structures (4KB aligned)
- Functions: `PagingInitialize()`, `PagingMapPage()`, `PagingUnmapPage()`, `PagingGetPhysicalAddress()`, `PagingInvalidatePage()`, `PagingSwitchDirectory()`
- Page fault handler (ISR 14) with detailed error reporting

**Implementation Details**:
- Page directory allocated from PMM
- Each page directory entry can map 4MB via 1024 page table entries
- CR3 register points to page directory
- CR0.PG bit enables paging
- Page fault handler decodes error code and CR2 (fault address)

**Testing**: Verifies identity mapping (virtual address 0x1000 → physical 0x1000).

### 3. Kernel Heap Allocator

**Files**:
- `kernel/include/kheap.h` - Heap interface
- `kernel/core/kheap.c` - Dynamic memory allocator

**Key Features**:
- Heap starts at 5MB, initial size 1MB, max 256MB
- First-fit allocation algorithm
- Block splitting when allocating
- Block merging when freeing (coalescing adjacent free blocks)
- Functions: `KHeapInitialize()`, `KAllocateMemory()`, `KFreeMemory()`, `KReallocateMemory()`, `KHeapGetStats()`

**Implementation Details**:
```c
typedef struct BlockHeader {
    size_t size;
    bool free;
    struct BlockHeader* next;
} BlockHeader;
```

**Testing**:
- Allocates multiple blocks (32 bytes, 40 bytes, 64 bytes)
- Writes to allocated memory and verifies data
- Frees blocks and reallocates
- Tests `KReallocateMemory()` expanding a 32-byte block to 128 bytes

### 4. Early Console Writer Updates

**Files**:
- `kernel/include/econ_writer.h` - Updated interface
- `kernel/arch/i386/econ_writer.c` - Conditional output implementation

**Key Features**:
- Added `EConWriterEnable()` function
- Serial output is no-op until explicitly enabled
- Allows kernel to boot silently unless `earlycon` flag is passed

**Implementation**:
```c
static bool econEnabled = false;

static void econWriterPutChar(void* data, char c)
{
    (void)data;

    if (!econEnabled) {
        return;  // Early return pattern
    }

    EConPutChar(c);
}
```

### 5. String Utilities (libclankercommon)

**Files**:
- `libraries/libclankercommon/include/clc/string.h` - String utility interface
- `libraries/libclankercommon/src/string.c` - Implementation
- `libraries/libclankercommon/Makefile` - Updated to include string.c

**Functions**:
- `ClcStrLen()` - Get string length
- `ClcStrCopy()` - Copy string with max length
- `ClcStrEqual()` - Compare strings for equality
- `ClcStrStartsWith()` - Check if string starts with prefix
- `ClcStrCompare()` - Lexicographic comparison

**Rationale**: Moved string utilities to common library for reuse between kernel and userspace.

### 6. Kernel Command Line Parser

**Files**:
- `kernel/include/kcmdline.h` - Parser interface
- `kernel/core/kcmdline.c` - Token-based parser implementation

**Key Features**:
- Parses multiboot command line into flags and key=value pairs
- Functions: `KCmdLineInitialize()`, `KCmdLineHasFlag()`, `KCmdLineGetValue()`
- Supports space-separated tokens
- Handles both boolean flags (e.g., `earlycon`) and key=value pairs (e.g., `console=ttyS0`)

**Implementation**:
```c
void KCmdLineInitialize(multiboot_info_t* mbootInfo)
{
    // Check if command line is available (bit 2 in flags)
    if (!(mbootInfo->flags & (1 << 2))) {
        cmdLineValid = false;
        return;
    }

    // Copy command line to local buffer
    const char* bootCmdLine = (const char*)mbootInfo->cmdline;
    ClcStrCopy(cmdLine, bootCmdLine, CMDLINE_MAX_LEN);
    cmdLineValid = true;
}
```

### 7. Updated main.c

**Changes**:
- Added `#include "kcmdline.h"`
- Parse command line at start of `KMain()`:
  ```c
  KCmdLineInitialize(mbootInfo);

  if (KCmdLineHasFlag("earlycon")) {
      EConWriterEnable();
  }
  ```
- Report enabled flags to serial console
- Wrapped all boot tests in `if (KCmdLineHasFlag("boottest"))` conditional
- Paging and heap initialization always run (required for kernel operation)

### 8. Linker Script Update

**File**: `kernel/arch/i386/linker.ld`

**Change**: Added `kernelEnd` symbol export so PMM knows where kernel ends in memory:
```ld
/* Kernel end marker */
kernelEnd = .;
```

## Output Strategy

The kernel now uses two output channels:

1. **VGA Console** (always enabled):
   - Simple progress messages
   - Example: "Initializing PMM... OK"

2. **Serial Console** (COM1):
   - Controlled by `earlycon` flag
   - Detailed diagnostics and statistics
   - Example: Memory statistics with bytes, KB, and MB
   - Boot test output

## Command Line Flags

### `earlycon`
- **Purpose**: Enable early console serial output for debugging
- **Effect**: `EConWriterEnable()` is called, serial writer outputs to COM1
- **Without flag**: Serial writer is no-op (silent)

### `boottest`
- **Purpose**: Run boot-time memory tests
- **Tests**:
  1. **PMM Test**: Allocate pages, free page, verify reuse
  2. **Paging Test**: Verify identity mapping
  3. **Heap Test**: Allocate, free, reallocate, verify data

## Testing Results

All tests pass successfully:

```bash
# Test 1: No flags (silent boot)
qemu-system-i386 -kernel clankeros.bin -m 128M -append "" -serial stdio
# Result: No serial output, no tests run ✅

# Test 2: Early console only
qemu-system-i386 -kernel clankeros.bin -m 128M -append "earlycon" -serial stdio
# Result: Serial output enabled, detailed diagnostics, no tests ✅

# Test 3: Boot tests only
qemu-system-i386 -kernel clankeros.bin -m 128M -append "boottest" -serial stdio
# Result: Tests run silently (no serial output) ✅

# Test 4: Both flags
qemu-system-i386 -kernel clankeros.bin -m 128M -append "earlycon boottest" -serial stdio
# Result: Serial output + all tests run and pass ✅
```

**Sample output with both flags**:
```
=== ClankerOS Boot Log ===
Multiboot magic: 0x2badb002
Multiboot info:  0x9500
Multiboot flags: 0x24f
Early console: enabled
Boot tests: enabled

Initializing PMM...
Memory Manager Statistics:
  Total: 128 MB (131072 KB, 134217728 bytes)
  Free:  126 MB (129876 KB, 132993024 bytes)
  Used:  1 MB (1196 KB, 1224704 bytes)

[... paging and heap initialization ...]

Memory Allocation Test:
  Alloc page 1: 0x20e000
  Alloc page 2: 0x20f000
  Alloc page 3: 0x210000
  Free after alloc: 128828 KB
  Freed page 2
  Free after free: 128832 KB
  Alloc page 4: 0x20f000 (reused freed page - PASS)
Memory test complete!

Paging Test:
  Virtual 0x1000 -> Physical 0x1000 (identity mapped - PASS)
Paging test complete!

Kernel Heap Test:
  Allocated str1: 0x50000c (32 bytes)
  Allocated nums: 0x500038 (40 bytes)
  Allocated str2: 0x500074 (64 bytes)
  nums[5] = 50 (expected 50)
  Freed nums
  Reallocated str1: 0x5000c0 (128 bytes)
  Heap: 1023 KB total, 0 KB used, 1023 KB free
Heap test complete!

=== Boot Complete ===
```

## Key Design Decisions

1. **Bitmap vs Stack Allocator**: Chose bitmap for PMM because it's simple, predictable, and allows arbitrary free/alloc patterns.

2. **Identity Mapping**: First 4MB identity-mapped (virtual == physical) to avoid page fault during paging bootstrap.

3. **Heap Location**: Started at 5MB to leave plenty of space after kernel and page tables.

4. **First-Fit Algorithm**: Simple and effective for kernel heap. Could optimize to best-fit later if needed.

5. **Conditional Serial Output**: Makes kernel boot silent by default, only verbose when debugging.

6. **String Utilities in Common Library**: Reusable between kernel and future userspace programs.

7. **Module Naming**: Used `K` prefix for kernel-specific code (`KCmdLine`, `KAllocateMemory`) per coding style guide.

## Code Style Notes

- Used PascalCase with module prefixes: `PmmAllocPage()`, `PagingMapPage()`, `KHeapInitialize()`
- Local variables and static functions: camelCase
- Constants and macros: UPPER_SNAKE_CASE
- Early return pattern preferred over nested conditionals
- K&R brace style throughout

## Files Modified/Created

**New Files**:
- `kernel/include/pmm.h`
- `kernel/core/pmm.c`
- `kernel/include/paging.h`
- `kernel/core/paging.c`
- `kernel/include/kheap.h`
- `kernel/core/kheap.c`
- `kernel/include/kcmdline.h`
- `kernel/core/kcmdline.c`
- `kernel/include/econ_writer.h`
- `kernel/arch/i386/econ_writer.c`
- `libraries/libclankercommon/include/clc/string.h`
- `libraries/libclankercommon/src/string.c`

**Modified Files**:
- `kernel/core/main.c` - Added command line parsing, conditional output/tests
- `kernel/arch/i386/linker.ld` - Added `kernelEnd` symbol
- `libraries/libclankercommon/Makefile` - Added string.c to sources

## Milestone Status

**Phase 1 (Memory Management) - COMPLETE** ✅
- ✅ Physical memory manager (bitmap allocator)
- ✅ Parse multiboot memory map
- ✅ Virtual memory (paging with identity mapping)
- ✅ Page fault handler
- ✅ Kernel heap allocator

**Bonus Features**:
- ✅ Kernel command line argument parsing
- ✅ Conditional early console output
- ✅ Optional boot-time tests
- ✅ String utilities in common library

## Next Steps (for future sessions)

From the IPC architecture document, Phase 2 would be:
- Process management
- Task switching
- Scheduler
- User mode transition

However, other prerequisites might be needed first:
- Keyboard input
- Shell/terminal
- File system basics
- System calls

## Technical Notes

### Memory Layout
```
0x00000000 - 0x00100000  Reserved (1MB) - BIOS, VGA, etc.
0x00100000 - ?           Kernel code/data
?          - ?           Page bitmap
0x00500000 - 0x10000000  Kernel heap (5MB start, 256MB max)
Higher addresses          Available for paging
```

### Page Fault Error Code Decoding
- Bit 0: Page present (0 = not present, 1 = protection fault)
- Bit 1: Write operation (0 = read, 1 = write)
- Bit 2: User mode (0 = kernel, 1 = user)
- Bit 3: Reserved bit set
- Bit 4: Instruction fetch

### Multiboot Command Line
- Flag bit 2 in `multiboot_info_t.flags` indicates command line availability
- Stored at `multiboot_info_t.cmdline` (physical address)
- Space-separated tokens parsed by `KCmdLineHasFlag()` and `KCmdLineGetValue()`

## Build and Test Commands

```bash
# Build
make

# Run with early console and tests
qemu-system-i386 -kernel kernel/clankeros.bin -m 128M \
  -append "earlycon boottest" -serial stdio

# Run silent (production-like)
qemu-system-i386 -kernel kernel/clankeros.bin -m 128M
```

## Session Summary

Session 4 was highly productive, completing the entire memory management subsystem and adding kernel command line argument support. The kernel can now:
- Detect and manage physical memory from multiboot
- Allocate/free 4KB pages with bitmap allocator
- Use virtual memory with paging enabled
- Dynamically allocate memory on the kernel heap
- Parse command line arguments for runtime configuration
- Conditionally enable debugging output and tests

All code follows ClankerOS coding style, tests pass, and the implementation is clean and well-documented. Ready to move on to process management or other kernel subsystems.
