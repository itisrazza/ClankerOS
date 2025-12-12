/* irq.c - Hardware interrupt request handlers */

#include <stddef.h>
#include <stdint.h>
#include "irq.h"
#include "idt.h"
#include "pic.h"

/* IRQ handler table */
static IrqHandlerFunc irqHandlers[16];

/* External assembly IRQ stubs */
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

/*
 * IrqInitialize - Initialize IRQ infrastructure
 */
void IrqInitialize(void)
{
    /* Clear handler table */
    for (int i = 0; i < 16; i++) {
        irqHandlers[i] = NULL;
    }

    /* Install IRQ handlers in IDT (interrupts 32-47) */
    IdtSetGate(32, (uint32_t)irq0, 0x08, 0x8E);
    IdtSetGate(33, (uint32_t)irq1, 0x08, 0x8E);
    IdtSetGate(34, (uint32_t)irq2, 0x08, 0x8E);
    IdtSetGate(35, (uint32_t)irq3, 0x08, 0x8E);
    IdtSetGate(36, (uint32_t)irq4, 0x08, 0x8E);
    IdtSetGate(37, (uint32_t)irq5, 0x08, 0x8E);
    IdtSetGate(38, (uint32_t)irq6, 0x08, 0x8E);
    IdtSetGate(39, (uint32_t)irq7, 0x08, 0x8E);
    IdtSetGate(40, (uint32_t)irq8, 0x08, 0x8E);
    IdtSetGate(41, (uint32_t)irq9, 0x08, 0x8E);
    IdtSetGate(42, (uint32_t)irq10, 0x08, 0x8E);
    IdtSetGate(43, (uint32_t)irq11, 0x08, 0x8E);
    IdtSetGate(44, (uint32_t)irq12, 0x08, 0x8E);
    IdtSetGate(45, (uint32_t)irq13, 0x08, 0x8E);
    IdtSetGate(46, (uint32_t)irq14, 0x08, 0x8E);
    IdtSetGate(47, (uint32_t)irq15, 0x08, 0x8E);
}

/*
 * IrqRegisterHandler - Register a handler for an IRQ
 */
void IrqRegisterHandler(uint8_t irq, IrqHandlerFunc handler)
{
    if (irq < 16) {
        irqHandlers[irq] = handler;
    }
}

/*
 * IrqUnregisterHandler - Unregister a handler for an IRQ
 */
void IrqUnregisterHandler(uint8_t irq)
{
    if (irq < 16) {
        irqHandlers[irq] = NULL;
    }
}

/*
 * irqHandler - Common C IRQ handler
 *
 * Called from assembly stub. Dispatches to registered handler
 * and sends EOI to PIC.
 */
void irqHandler(uint32_t intNum, uint32_t errorCode)
{
    (void)errorCode; /* Unused for IRQs */

    /* Convert interrupt number to IRQ number (32-47 -> 0-15) */
    uint8_t irq = (uint8_t)(intNum - 32);

    /* Call registered handler if present */
    if (irq < 16 && irqHandlers[irq] != NULL) {
        irqHandlers[irq]();
    }

    /* Send End-Of-Interrupt to PIC */
    PicSendEoi(irq);
}
