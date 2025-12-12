/* pic.h - 8259 Programmable Interrupt Controller */
#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/*
 * PicInitialize - Initialize the 8259 PIC
 *
 * Remaps the PIC to use interrupts 32-47 instead of 0-15 (which conflict
 * with CPU exceptions). Disables all IRQs initially.
 */
void PicInitialize(void);

/*
 * PicSendEoi - Send End-Of-Interrupt signal to PIC
 *
 * Must be called at the end of each IRQ handler to acknowledge
 * the interrupt.
 *
 * Parameters:
 *   irq - IRQ number (0-15)
 */
void PicSendEoi(uint8_t irq);

/*
 * PicSetMask - Disable an IRQ line
 *
 * Parameters:
 *   irq - IRQ number (0-15)
 */
void PicSetMask(uint8_t irq);

/*
 * PicClearMask - Enable an IRQ line
 *
 * Parameters:
 *   irq - IRQ number (0-15)
 */
void PicClearMask(uint8_t irq);

#endif /* PIC_H */
