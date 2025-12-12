# Session 5: Process Management and Kernel Panic System

**Date**: 2025-12-13
**Focus**: Implementing process management with preemptive multitasking and a robust kernel panic system

## Summary

This session implemented two major subsystems:

1. **Process Management** - Complete process management system with context switching, round-robin scheduling, and preemptive multitasking
2. **Kernel Panic System** - Robust panic handler with minimal dependencies for reliable error reporting even in corrupted system states

## Major Accomplishments

### 1. Process Management Implementation

Implemented a complete process management system with interrupt-based context switching.

#### Files Created
- `kernel/include/process.h` - Process management API and data structures
- `kernel/core/process.c` - Process management implementation

#### Key Features
- **Process Control Block (PCB)** with full CPU context
- **Process states**: READY, RUNNING, BLOCKED, TERMINATED
- **Process modes**: KERNEL (ring 0), USER (ring 3) - user mode deferred for now
- **Round-robin scheduler** with ready queue
- **Preemptive multitasking** via PIT timer integration (100 Hz)
- **Context switching** leveraging existing IRQ framework
- **Process lifecycle management**: creation, scheduling, blocking, termination

#### Process Structure
```c
typedef struct Process {
    uint32_t pid;
    char name[32];
    ProcessState state;
    ProcessMode mode;
    CpuContext context;        // Full register state
    uintptr_t kernelStack;     // 8KB stack
    uintptr_t userStack;       // For future user mode
    PageDirectory* pageDirectory;
    uint32_t timeslice;
    uint32_t priority;
    struct Process* next;      // Ready queue linkage
} Process;
```

#### API Functions
- `ProcessInitialize()` - Initialize process system with idle process (PID 0)
- `ProcessCreate(name, entryPoint, mode)` - Create new process
- `ProcessSchedule(regs)` - Timer-driven scheduler
- `ProcessYield()` - Voluntary CPU yield
- `ProcessBlock()` / `ProcessUnblock()` - Block/unblock processes
- `ProcessExit()` - Terminate current process
- `ProcessGetCurrent()` - Get current process

#### Design Decisions

**Interrupt-based Context Switching**:
- Leverages existing IRQ framework instead of complex assembly
- Scheduler runs on timer interrupts (IRQ0 at 100 Hz)
- Full CPU context saved/restored via `registers_t` structure
- Simpler and more maintainable than hand-written assembly

**Initial Process Setup**:
- PID 0 "idle" process represents boot context
- Test processes created as kernel-mode only
- Stack includes fake interrupt frame for proper context switching
- Entry point wrapper enables interrupts and calls actual function

**Process Exit Handling**:
- Process marks itself as TERMINATED
- Halts in infinite loop waiting for timer interrupt
- Scheduler sees TERMINATED state and doesn't re-queue
- Avoids complexity of software interrupts (int $0x81 caused GPF)

### 2. Enhanced IRQ System for Register-Aware Handlers

Modified IRQ infrastructure to support handlers that need register state.

#### Files Modified
- `kernel/include/irq.h` - Added `IrqHandlerRegFunc` typedef
- `kernel/arch/i386/irq.c` - Dual handler table implementation
- `kernel/arch/i386/irq_stubs.s` - Full context saving matching ISR pattern

#### Key Changes
- **Dual handler tables**: Simple handlers (`IrqHandlerFunc`) and register-aware (`IrqHandlerRegFunc`)
- **New API**: `IrqRegisterHandlerWithRegs(irq, handler)` for scheduler and other context-modifying handlers
- **Assembly stub changes**: IRQ stubs now save full CPU state (matching ISR pattern) and pass `registers_t*` to C handler

### 3. PIT Timer Integration with Scheduler

Modified PIT to support scheduler callbacks.

#### Files Modified
- `kernel/include/pit.h` - Added `PitTickHandler` typedef
- `kernel/arch/i386/pit.c` - Tick handler registration and callback

#### Key Features
- Timer handler receives full register state
- Scheduler registered via `PitRegisterTickHandler(ProcessSchedule)`
- Timer ticks at 100 Hz providing ~10ms timeslices
- Clean separation: PIT manages hardware, scheduler manages processes

### 4. Kernel Panic System

Implemented a robust kernel panic system with minimal dependencies to ensure reliable operation even when the system is corrupted.

#### Files Created
- `kernel/include/panic.h` - Panic macros and function declarations
- `kernel/core/panic.c` - Panic implementation

#### Files Modified
- `kernel/arch/i386/isr.c` - Use KPanicRegs for unhandled exceptions
- `kernel/core/main.c` - Added page fault handler using KPanicRegs, test flags

#### Key Features
- **Minimal dependencies**: No memory allocations, no writer interface
- **Direct hardware access**: `VidWriteString()`, `VidPutChar()`, `EConPutChar()` only
- **Stack-based formatting**: 12-byte buffer for number conversion
- **Simple format parser**: Supports `%s`, `%d`, `%u`, `%x`, `%08x` (width specifiers)
- **Two variants**:
  - `KPanic(fmt, ...)` - Basic panic with message
  - `KPanicRegs(regs, fmt, ...)` - Panic with full CPU register dump
- **Macro-based**: `KPanic()` automatically captures `__FILE__` and `__LINE__`

#### Implementation Highlights

**Stack-only operations**:
```c
static void writeHex(char* buf, uint32_t value, int width);
static void writeDecimal(char* buf, int value);
static void serialWriteString(const char* str);
```

**Format string parser**:
- Handles width specifiers by skipping digits after `%`
- No dynamic allocation or complex formatting
- Falls back to literal character output for unknown specifiers

**Critical design**:
- Disables interrupts immediately (`cli`)
- Outputs to both VGA and serial
- Halts forever in infinite loop (`hlt`)
- Never returns or re-enables interrupts

#### Test Flags
- `testpanic` - Test basic KPanic functionality
- `testpagefault` - Test KPanicRegs with page fault (accesses 0xDEADBEEF)

### 5. Documentation Updates

#### CLAUDE.md
Added "Running and Testing (for Claude Code)" section:
- Always use QEMU directly (not `make run-serial`)
- Always include `earlycon` kernel parameter
- Explains why this matters for AI testing

#### IPC_ARCHITECTURE.md
Refined block device namespace design:
- Changed from Linux-style names (`sda`, `mmcblk`) to interface-based addressing
- Format: `block:<interface>:<address>`
- Examples: `block:floppy:a`, `block:ide:0.0.0`, `block:usb:001.008`
- Each interface defines its own addressing scheme

#### KERNEL_PARAMETERS.md (New)
Comprehensive documentation of kernel parameters:

**Implemented**:
- `earlycon` - Enable early console (COM1 serial)
- `testpanic` - Test panic system
- `testpagefault` - Test page fault handler

**Planned**:
- `root=<block_device>` - Root filesystem device
- `nographics` - Disable graphics, stay in text mode
- `init=<program_path>` - Alternate initial process

Includes usage examples, API reference, and testing instructions.

## Testing Performed

### Process Management Tests
```bash
# Normal boot with process management
qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio -append "earlycon"
```

**Output**:
```
=== Process Management Initialization ===
Initializing process management...
Process management initialized (PID 0: idle)
Created process PID 1: test1 (kernel mode)
Created process PID 2: test2 (kernel mode)
Created process PID 3: test3 (kernel mode)
Scheduler enabled - starting multitasking
Process 1 (test1) exiting
Process 2 (test2) exiting
Process 3 (test3) exiting
```

All three test processes executed successfully with preemptive multitasking.

### Panic System Tests

**Basic Panic Test**:
```bash
qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio -append "earlycon testpanic"
```

**Output**:
```
================================================================================
!!!                          KERNEL PANIC                                   !!!
================================================================================
Location: core/main.c:339
Message: Test panic - this is intentional (value: 42)

System halted. CPU in halt state.
================================================================================
```

**Page Fault Test**:
```bash
qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio -append "earlycon testpagefault"
```

**Output**:
```
================================================================================
!!!                          KERNEL PANIC                                   !!!
================================================================================
Location: core/main.c:410
Message: Page Fault at 0xdeadbeef - Read from non-present page

CPU Register Dump:
  EIP: 0x001011d1  CS:  0x0008  EFLAGS: 0x00010246
  EAX: 0x0000002c  EBX: 0x00106008  ECX: 0x001028f8  EDX: 0x00000000
  ESP: 0x0010af94  EBP: 0x0010aff0  ESI: 0x0010448e  EDI: 0x00106010
  DS:  0x0010  SS:  0x4d58
  INT: 14  ERR: 0x00000000

System halted. CPU in halt state.
================================================================================
```

## Issues Encountered and Resolved

### Issue 1: General Protection Fault on Process Exit

**Problem**: ProcessExit called ProcessYield which used `int $0x81`, but interrupt 0x81 was never registered in the IDT, causing GPF.

**Solution**: Changed ProcessExit to simply mark process as TERMINATED and halt in infinite loop. Scheduler sees TERMINATED state and doesn't re-queue the process.

```c
void ProcessExit(void) {
    currentProcess->state = PROCESS_STATE_TERMINATED;
    while (1) {
        __asm__ volatile ("hlt");
    }
}
```

### Issue 2: VGA Display Corruption in Panic Handler

**Problem**: Original panic implementation used `VidGetWriter()` and `ClcPrintfWriter()` which could allocate memory or trigger complex operations, causing corruption when called during page faults.

**Solution**: Completely rewrote panic handler to use only direct hardware access with stack-based formatting:
- No memory allocations
- No writer interface calls
- Simple format string parser with stack buffers
- Direct calls to `VidWriteString()`, `VidPutChar()`, `EConPutChar()`

### Issue 3: Format String Parsing for %08x

**Problem**: Simple panic format parser treated `%08x` as literal '0' and '8' characters, then 'x' as format specifier, resulting in broken output: "Page Fault at 0x08x -"

**Solution**: Enhanced parser to skip width/precision digits:
```c
// Skip width/precision specifiers (like 08 in %08x)
while (*p >= '0' && *p <= '9') {
    p++;
}
// Now handle the actual format specifier
```

## Code Statistics

**New Files**: 4
- `kernel/include/process.h` (106 lines)
- `kernel/core/process.c` (437 lines)
- `kernel/include/panic.h` (59 lines)
- `kernel/core/panic.c` (321 lines)
- `docs/KERNEL_PARAMETERS.md` (218 lines)

**Modified Files**: 7
- `kernel/include/irq.h` - Added register-aware handler support
- `kernel/arch/i386/irq.c` - Dual handler table implementation
- `kernel/arch/i386/irq_stubs.s` - Full context saving
- `kernel/include/pit.h` - Tick handler registration
- `kernel/arch/i386/pit.c` - Scheduler callback support
- `kernel/arch/i386/isr.c` - Use KPanicRegs for exceptions
- `kernel/core/main.c` - Process management initialization, panic tests
- `CLAUDE.md` - Added testing instructions
- `docs/IPC_ARCHITECTURE.md` - Block device namespace refinement

**Total Lines Added**: ~1,200 lines

## Next Steps

### Immediate Next Milestone: User Mode and System Calls

From TODO.md, the next priority is implementing user mode support:

1. **User Mode Transition** (ring 0 → ring 3)
   - TSS (Task State Segment) for hardware task switching
   - Proper segment descriptors for user code/data
   - Stack switching on privilege level changes

2. **System Call Interface**
   - Interrupt 0x80 handler for syscalls
   - Parameter passing conventions
   - Return value handling
   - Basic syscalls: exit, yield, write

3. **User Mode Process Support**
   - Update ProcessCreate to support user mode
   - User space memory mapping
   - Test user mode processes

### Future Milestones (from TODO.md)

4. **Message Passing IPC**
   - Namespace registration and lookup
   - Synchronous message passing
   - IPC between kernel and user processes

5. **Filesystem Support**
   - VFS layer
   - Initial RAM disk
   - Basic file operations

## Lessons Learned

1. **Interrupt-based context switching is simpler**: Leveraging the existing IRQ framework avoided complex assembly and made the scheduler more maintainable.

2. **Panic handlers must be minimal**: When the system is in a bad state, every operation is suspect. Direct hardware access and stack-only operations are essential.

3. **Test flags are invaluable**: Adding `testpanic` and `testpagefault` flags made testing much easier and will help verify the system remains stable as new features are added.

4. **Documentation as you go**: Creating KERNEL_PARAMETERS.md and updating CLAUDE.md immediately helps both human users and future AI sessions understand the system.

5. **Simple format parsers are sufficient**: The panic handler doesn't need full printf capabilities - supporting basic format specifiers (`%s`, `%d`, `%x`) with width handling is enough for diagnostic output.

## Session Metrics

- **Duration**: ~2-3 hours
- **Commits**: Multiple incremental commits
- **Tests Run**: 5+ test scenarios
- **Documentation**: 3 files updated/created
- **Issues Resolved**: 3 critical bugs fixed

## References

- [OSDev Wiki - Multitasking](https://wiki.osdev.org/Multitasking)
- [OSDev Wiki - Kernel Panic](https://wiki.osdev.org/Kernel_Panic)
- Intel 64 and IA-32 Architectures Software Developer's Manual, Volume 3A (System Programming Guide)
