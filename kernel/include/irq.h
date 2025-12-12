/* irq.h - Hardware interrupt request handlers */
#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>
#include "isr.h"

/* IRQ numbers (mapped to IDT entries 32-47) */
#define IRQ0  0   /* System timer (PIT) */
#define IRQ1  1   /* Keyboard */
#define IRQ2  2   /* Cascade (used internally by PICs) */
#define IRQ3  3   /* COM2 */
#define IRQ4  4   /* COM1 */
#define IRQ5  5   /* LPT2 */
#define IRQ6  6   /* Floppy disk */
#define IRQ7  7   /* LPT1 / spurious */
#define IRQ8  8   /* CMOS real-time clock */
#define IRQ9  9   /* Free for peripherals / legacy SCSI / NIC */
#define IRQ10 10  /* Free for peripherals / SCSI / NIC */
#define IRQ11 11  /* Free for peripherals / SCSI / NIC */
#define IRQ12 12  /* PS/2 Mouse */
#define IRQ13 13  /* FPU / Coprocessor / Inter-processor */
#define IRQ14 14  /* Primary ATA Hard Disk */
#define IRQ15 15  /* Secondary ATA Hard Disk */

/*
 * IrqHandlerFunc - IRQ handler function type
 *
 * Called when an IRQ occurs. The handler should return quickly
 * and not block.
 */
typedef void (*IrqHandlerFunc)(void);

/*
 * IrqHandlerRegFunc - IRQ handler function type with register state
 *
 * Called when an IRQ occurs with full CPU register state.
 * Used for handlers that need to modify register state (e.g., scheduler).
 */
typedef void (*IrqHandlerRegFunc)(registers_t* regs);

/*
 * IrqInitialize - Initialize IRQ infrastructure
 *
 * Sets up IRQ handlers in the IDT (entries 32-47).
 * Must be called after IdtInitialize().
 */
void IrqInitialize(void);

/*
 * IrqRegisterHandler - Register a handler for an IRQ
 *
 * Parameters:
 *   irq - IRQ number (0-15)
 *   handler - Function to call when IRQ occurs
 */
void IrqRegisterHandler(uint8_t irq, IrqHandlerFunc handler);

/*
 * IrqUnregisterHandler - Unregister a handler for an IRQ
 *
 * Parameters:
 *   irq - IRQ number (0-15)
 */
void IrqUnregisterHandler(uint8_t irq);

/*
 * IrqRegisterHandlerWithRegs - Register a handler that receives register state
 *
 * Parameters:
 *   irq - IRQ number (0-15)
 *   handler - Function to call when IRQ occurs (receives register state)
 */
void IrqRegisterHandlerWithRegs(uint8_t irq, IrqHandlerRegFunc handler);

#endif /* IRQ_H */
