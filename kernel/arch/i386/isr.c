/* isr.c - Interrupt Service Routines implementation */

#include <stdint.h>
#include <stddef.h>
#include "isr.h"
#include "idt.h"
#include "panic.h"

/* ISR handler array */
static isr_t interruptHandlers[256];

/* Exception messages */
static const char* exceptionMessages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

/* Forward declaration */
extern void VidWriteString(const char* str);

/*
 * IsrRegisterHandler - Register a custom ISR handler
 */
void IsrRegisterHandler(uint8_t n, isr_t handler)
{
    interruptHandlers[n] = handler;
}

/*
 * isrHandler - Common ISR handler called from assembly
 */
void isrHandler(registers_t* regs)
{
    // Call custom handler if registered
    if (interruptHandlers[regs->intNo] != 0) {
        isr_t handler = interruptHandlers[regs->intNo];
        handler(regs);
    } else {
        // Default handler: panic with exception info
        const char* exceptionName = "Unknown Interrupt";
        if (regs->intNo < 32) {
            exceptionName = exceptionMessages[regs->intNo];
        }

        KPanicRegs(regs, "Unhandled CPU Exception: %s (INT %u)",
                   exceptionName, regs->intNo);
    }
}

/*
 * IsrInitialize - Initialize ISRs
 *
 * Registers all CPU exception handlers (0-31) in the IDT
 */
void IsrInitialize(void)
{
    // Clear handler array
    for (int i = 0; i < 256; i++) {
        interruptHandlers[i] = 0;
    }

    // Register ISRs 0-31 (CPU exceptions)
    // Using 0x08 (kernel code segment) and present flag
    uint8_t flags = IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32;

    IdtSetGate(0, (uint32_t)isr0, 0x08, flags);
    IdtSetGate(1, (uint32_t)isr1, 0x08, flags);
    IdtSetGate(2, (uint32_t)isr2, 0x08, flags);
    IdtSetGate(3, (uint32_t)isr3, 0x08, flags);
    IdtSetGate(4, (uint32_t)isr4, 0x08, flags);
    IdtSetGate(5, (uint32_t)isr5, 0x08, flags);
    IdtSetGate(6, (uint32_t)isr6, 0x08, flags);
    IdtSetGate(7, (uint32_t)isr7, 0x08, flags);
    IdtSetGate(8, (uint32_t)isr8, 0x08, flags);
    IdtSetGate(9, (uint32_t)isr9, 0x08, flags);
    IdtSetGate(10, (uint32_t)isr10, 0x08, flags);
    IdtSetGate(11, (uint32_t)isr11, 0x08, flags);
    IdtSetGate(12, (uint32_t)isr12, 0x08, flags);
    IdtSetGate(13, (uint32_t)isr13, 0x08, flags);
    IdtSetGate(14, (uint32_t)isr14, 0x08, flags);
    IdtSetGate(15, (uint32_t)isr15, 0x08, flags);
    IdtSetGate(16, (uint32_t)isr16, 0x08, flags);
    IdtSetGate(17, (uint32_t)isr17, 0x08, flags);
    IdtSetGate(18, (uint32_t)isr18, 0x08, flags);
    IdtSetGate(19, (uint32_t)isr19, 0x08, flags);
    IdtSetGate(20, (uint32_t)isr20, 0x08, flags);
    IdtSetGate(21, (uint32_t)isr21, 0x08, flags);
    IdtSetGate(22, (uint32_t)isr22, 0x08, flags);
    IdtSetGate(23, (uint32_t)isr23, 0x08, flags);
    IdtSetGate(24, (uint32_t)isr24, 0x08, flags);
    IdtSetGate(25, (uint32_t)isr25, 0x08, flags);
    IdtSetGate(26, (uint32_t)isr26, 0x08, flags);
    IdtSetGate(27, (uint32_t)isr27, 0x08, flags);
    IdtSetGate(28, (uint32_t)isr28, 0x08, flags);
    IdtSetGate(29, (uint32_t)isr29, 0x08, flags);
    IdtSetGate(30, (uint32_t)isr30, 0x08, flags);
    IdtSetGate(31, (uint32_t)isr31, 0x08, flags);
}
