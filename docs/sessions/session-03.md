# Session 3: Hardware Interrupts - IRQ, PIC, and PIT

**Date:** 2025-12-12
**Focus:** Hardware interrupt infrastructure, PIC initialization, timer implementation

## Overview

This session completed the low-level interrupt handling infrastructure by implementing IRQ handlers, the Programmable Interrupt Controller (PIC), and the Programmable Interval Timer (PIT). This enables ClankerOS to receive and handle hardware interrupts from devices like the keyboard, timer, and other peripherals.

## Key Accomplishments

### 1. x86 I/O Port Access (x86.h)

Created a header for x86-specific instruction wrappers per human architect's request.

#### Design Decision: Use Instruction Names Directly

**Human feedback:** "For simple wrappers around assembly instructions, I'd like the instruction itself to be used. I'd also like for these sort of instruction stand-in functions to be in a file called x86.h."

Created `kernel/include/x86.h` with:
- `outb(port, value)` - NOT `IoWriteByte()`
- `inb(port)` - NOT `IoReadByte()`
- `io_wait()` - Short delay via port 0x80

```c
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
```

**Rationale:** More transparent, matches actual x86 instructions, easier for low-level programmers to understand.

### 2. IRQ Handler Infrastructure

#### IRQ Numbering and Remapping

**Problem:** Default IRQ mapping conflicts with CPU exceptions
- CPU exceptions: 0-31
- Default IRQs: 0-15 (conflicts!)

**Solution:** Remap IRQs to interrupts 32-47
- IRQ 0 (timer) → Interrupt 32
- IRQ 1 (keyboard) → Interrupt 33
- IRQ 2 (cascade) → Interrupt 34
- ... through IRQ 15 → Interrupt 47

#### Assembly Stubs (irq_stubs.s)

Created 16 IRQ stubs following the pattern:
```assembly
.macro IRQ num, intNum
.global irq\num
irq\num:
    cli
    pushl $0                    /* Dummy error code */
    pushl $\intNum              /* Interrupt number */
    jmp irqCommonStub
.endm

IRQ 0, 32
IRQ 1, 33
...
IRQ 15, 47

irqCommonStub:
    /* Save all registers */
    pushl %eax
    pushl %ecx
    pushl %edx
    pushl %ebx
    pushl %esp
    pushl %ebp
    pushl %esi
    pushl %edi

    call irqHandler

    /* Restore all registers */
    popl %edi
    popl %esi
    popl %ebp
    popl %esp
    popl %ebx
    popl %edx
    popl %ecx
    popl %eax

    addl $8, %esp    /* Clean up error code and int number */
    sti
    iret
```

#### C Handler Registration (irq.c, irq.h)

**Dispatch mechanism:**
```c
static IrqHandlerFunc irqHandlers[16];

void irqHandler(uint32_t intNum, uint32_t errorCode)
{
    uint8_t irq = (uint8_t)(intNum - 32);

    if (irq < 16 && irqHandlers[irq] != NULL) {
        irqHandlers[irq]();  // Call registered handler
    }

    PicSendEoi(irq);  // Send End-Of-Interrupt to PIC
}
```

**Public API:**
- `IrqInitialize()` - Install all 16 IRQ stubs into IDT
- `IrqRegisterHandler(irq, handler)` - Register C callback for IRQ
- `IrqUnregisterHandler(irq)` - Remove handler

### 3. PIC (8259 Programmable Interrupt Controller)

#### Hardware Configuration

**Dual PIC setup:**
- Master PIC: ports 0x20 (command), 0x21 (data)
  - Handles IRQ 0-7
- Slave PIC: ports 0xA0 (command), 0xA1 (data)
  - Handles IRQ 8-15
  - Cascaded to master via IRQ 2

#### Initialization Sequence (ICW)

```c
void PicInitialize(void)
{
    // ICW1: Start initialization
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // ICW2: Set vector offsets (remap to 32-47)
    outb(PIC1_DATA, 32);  // Master: IRQ 0-7  -> INT 32-39
    io_wait();
    outb(PIC2_DATA, 40);  // Slave:  IRQ 8-15 -> INT 40-47
    io_wait();

    // ICW3: Configure cascade
    outb(PIC1_DATA, 4);   // Master has slave at IRQ2
    io_wait();
    outb(PIC2_DATA, 2);   // Slave cascade identity
    io_wait();

    // ICW4: Set 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Mask all interrupts initially
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}
```

#### End-Of-Interrupt (EOI) Handling

```c
void PicSendEoi(uint8_t irq)
{
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);  // Send to slave
    }
    outb(PIC1_COMMAND, PIC_EOI);      // Always send to master
}
```

**Critical detail:** For IRQs 8-15, EOI must be sent to BOTH slave and master.

#### IRQ Masking

```c
void PicSetMask(uint8_t irq)      // Disable IRQ
void PicClearMask(uint8_t irq)    // Enable IRQ
```

Individual IRQ lines can be masked/unmasked via the data ports.

### 4. PIT (Programmable Interval Timer)

#### Timer Configuration

**Hardware:**
- Channel 0 at port 0x40 (our timer)
- Channel 1 at port 0x41 (legacy RAM refresh, unused)
- Channel 2 at port 0x42 (PC speaker)
- Command register at port 0x43

**Base frequency:** 1.193182 MHz (1193182 Hz)

#### Initialization

```c
void PitInitialize(uint32_t frequency)
{
    uint32_t divisor = PIT_BASE_FREQ / frequency;

    // Command: Channel 0, lobyte/hibyte, mode 3 (square wave), binary
    outb(PIT_COMMAND, 0x36);

    // Send divisor (low byte, then high byte)
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    // Register handler and enable IRQ0
    IrqRegisterHandler(IRQ0, pitIrqHandler);
    PicClearMask(IRQ0);

    timerFrequency = PIT_BASE_FREQ / divisor;
}
```

**Default frequency:** 100 Hz (fires every 10ms)

#### Tick Counter

```c
static volatile uint64_t timerTicks = 0;

static void pitIrqHandler(void)
{
    timerTicks++;
}

uint64_t PitGetTicks(void)
{
    return timerTicks;
}
```

**Important:** `volatile` ensures compiler doesn't optimize away reads of `timerTicks` (modified by interrupt handler).

### 5. Kernel Integration

#### Initialization Sequence in KMain()

```c
// Initialize IRQs
ClcPrintfWriter(vgaWriter, "Initializing IRQs... ");
IrqInitialize();
ClcPrintfWriter(vgaWriter, "OK\n");

// Initialize PIC
ClcPrintfWriter(vgaWriter, "Initializing PIC... ");
PicInitialize();
ClcPrintfWriter(vgaWriter, "OK\n");

// Initialize PIT timer (100 Hz)
ClcPrintfWriter(vgaWriter, "Initializing PIT... ");
PitInitialize(100);
ClcPrintfWriter(vgaWriter, "OK (100 Hz)\n");

// Enable interrupts
ClcPrintfWriter(vgaWriter, "Enabling interrupts... ");
__asm__ volatile ("sti");
ClcPrintfWriter(vgaWriter, "OK\n");
```

#### Timer Test

Added a 3-second timer test to verify interrupts work:

```c
ClcPrintfWriter(vgaWriter, "\nTimer test (watching ticks for 3 seconds):\n");
uint64_t startTicks = PitGetTicks();
uint64_t lastTicks = startTicks;

while (PitGetTicks() < startTicks + 300) {  // 300 ticks = 3 seconds at 100 Hz
    uint64_t currentTicks = PitGetTicks();
    if (currentTicks != lastTicks) {
        ClcPrintfWriter(vgaWriter, "Ticks: %u\n", (uint32_t)currentTicks);
        lastTicks = currentTicks;
    }
}

ClcPrintfWriter(vgaWriter, "Timer test complete!\n");
```

**Output:** Shows tick counter incrementing every 10ms, demonstrating working hardware interrupts.

### 6. VGA Text Wrapping Bug Fix

**Problem:** Text went off screen instead of wrapping

**Root cause:** `VidPutChar()` checked row bounds when wrapping at end of line, but NOT when processing newline character.

**Buggy code:**
```c
if (c == '\n') {
    terminalColumn = 0;
    terminalRow++;  // Can exceed VGA_HEIGHT!
    return;
}
```

**Fixed code:**
```c
if (c == '\n') {
    terminalColumn = 0;
    if (++terminalRow == VGA_HEIGHT) {
        terminalRow = 0;  // Wrap to top
    }
    return;
}
```

**Current behavior:** Simple wraparound to top of screen (scrolling to be implemented later).

## Architecture Flow

### Complete Interrupt Path (IRQ0 Timer Example)

1. **PIT fires** → Sends IRQ 0 to PIC
2. **PIC remaps** → IRQ 0 → Interrupt 32
3. **CPU jumps** → IDT entry 32 → `irq0` stub
4. **Assembly stub** → Saves registers → calls `irqHandler(32, 0)`
5. **C dispatcher** → Looks up `irqHandlers[0]` → calls `pitIrqHandler()`
6. **Timer handler** → Increments `timerTicks`
7. **Returns to dispatcher** → Calls `PicSendEoi(0)`
8. **Assembly stub** → Restores registers → `iret`
9. **CPU resumes** → Original code continues

## Technical Details

### Why io_wait()?

Between PIC commands, we call `io_wait()` which does `outb` to port 0x80:
```c
static inline void io_wait(void)
{
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}
```

**Reason:** Older hardware needs time between I/O operations. Port 0x80 is traditionally used for delays (POST code port).

### Why cli/sti in IRQ Stubs?

```assembly
irq0:
    cli              # Disable interrupts
    pushl $0
    pushl $32
    jmp irqCommonStub

irqCommonStub:
    # ... save registers ...
    call irqHandler
    # ... restore registers ...
    sti              # Re-enable interrupts
    iret
```

- `cli` ensures we don't get interrupted while saving state
- `sti` re-enables before `iret` to allow nested interrupts
- CPU automatically clears IF flag on interrupt, but explicit `cli` is defensive

### Why Dummy Error Code?

Some CPU exceptions (like page fault) push error codes, others don't. To maintain consistent stack layout:
```assembly
pushl $0         # Dummy error code
pushl $32        # Interrupt number
```

Now all handlers have same stack frame, making `irqHandler()` simpler.

## File Structure

```
kernel/
├── arch/i386/
│   ├── irq_stubs.s      # Assembly stubs for IRQ 0-15
│   ├── irq.c            # IRQ registration and dispatch
│   ├── pic.c            # PIC driver (8259)
│   └── pit.c            # PIT timer driver (8253/8254)
├── include/
│   ├── x86.h            # inb, outb, io_wait
│   ├── irq.h            # IRQ handler interface
│   ├── pic.h            # PIC interface
│   └── pit.h            # PIT interface
└── core/
    └── main.c           # Updated with IRQ/PIC/PIT init
```

## Commits

**Main commit:** fce6a83 - Add IRQ, PIC, and PIT support for hardware interrupts

**Staged files:**
- 8 new files (x86.h, irq.h/c/s, pic.h/c, pit.h/c)
- Modified main.c (init sequence + VGA fix)
- Claude conversation data (2 .jsonl files)
- Total: 716 insertions, 7 deletions

## Build and Testing

### Build Results
```bash
$ make
i386-elf-gcc -c irq_stubs.s -o irq_stubs.o
i386-elf-gcc -c irq.c -o irq.o
i386-elf-gcc -c pic.c -o pic.o
i386-elf-gcc -c pit.c -o pit.o
i386-elf-gcc -c main.c -o main.o
i386-elf-ld -T linker.ld -o clankeros.bin <objects>
```

**Status:** ✅ Clean build, no warnings

### QEMU Test Results

```bash
$ make run-serial
Initializing GDT... OK
Initializing IDT... OK
Initializing ISRs... OK
Initializing IRQs... OK
Initializing PIC... OK
Initializing PIT... OK (100 Hz)
Enabling interrupts... OK

Timer test (watching ticks for 3 seconds):
Ticks: 1
Ticks: 2
Ticks: 3
...
Ticks: 299
Ticks: 300
Timer test complete!
```

**Observations:**
- IRQs fire correctly at 100 Hz
- No spurious interrupts
- EOI handling works (no interrupt storm)
- VGA wrapping works correctly

## Code Quality

### Coding Style Compliance
- ✅ K&R brace style throughout
- ✅ PascalCase public functions with prefixes (`IrqInitialize`, `PicSendEoi`, `PitGetTicks`)
- ✅ camelCase locals and statics (`timerTicks`, `irqHandlers`)
- ✅ UPPER_SNAKE_CASE constants (`PIC1_COMMAND`, `PIT_BASE_FREQ`, `IRQ0`)
- ✅ Instruction names for I/O (`inb`, `outb` not `IoInb`, `IoOutb`)

### Documentation
- All public functions commented
- Assembly code has explanatory comments
- Hardware details documented in comments

## Lessons Learned

### Instruction Naming Matters

Using actual instruction names (`inb`/`outb`) instead of abstracted names (`IoReadByte`/`IoWriteByte`) makes the code more transparent for low-level work. Assembly programmers instantly recognize what's happening.

### PIC EOI is Tricky

Must send EOI to both slave AND master for IRQs 8-15. Missing this causes interrupt storms (handler never finishes, keeps re-triggering).

### Volatile is Critical for IRQ Handlers

```c
static volatile uint64_t timerTicks = 0;
```

Without `volatile`, the compiler might optimize away reads in the timer test loop, assuming `timerTicks` never changes (it can't see the interrupt handler modifying it).

### io_wait() Prevents Race Conditions

Old hardware needs delays between PIC commands. Modern systems are fast enough that `io_wait()` might not be needed, but including it ensures compatibility.

## Next Milestone: Memory Management

With hardware interrupts working, we can now move to memory management:

1. **Physical memory allocator** - Parse multiboot memory map, bitmap/stack allocator
2. **Paging** - Enable virtual memory, identity mapping for kernel
3. **Kernel heap** - kmalloc/kfree for dynamic allocation

This will unlock:
- Process isolation (separate address spaces)
- Demand paging (page faults)
- Memory protection (read-only kernel code)
- Dynamic memory allocation

## Statistics

- **Files Added**: 8
- **Files Modified**: 3 (main.c, TODO.md, README.md)
- **Lines Added**: ~716
- **Lines Removed**: ~7
- **Commits**: 1
- **Build Status**: ✅ Clean
- **Boot Status**: ✅ Working
- **Timer Test**: ✅ Passing (100 Hz interrupts verified)

## References

- Intel 8259A PIC Datasheet
- Intel 8253/8254 PIT Datasheet
- OSDev Wiki: PIC, PIT, IRQs
- Intel x86 Software Developer's Manual (Interrupt Handling)
