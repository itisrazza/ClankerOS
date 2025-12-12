/* pit.h - Programmable Interval Timer (8253/8254) */
#ifndef PIT_H
#define PIT_H

#include <stdint.h>
#include "isr.h"

/*
 * PitTickHandler - Handler function called on each timer tick
 *
 * Receives the CPU register state at the time of the interrupt.
 */
typedef void (*PitTickHandler)(registers_t* regs);

/*
 * PitInitialize - Initialize the PIT
 *
 * Sets up the PIT to generate timer interrupts at the specified frequency.
 *
 * Parameters:
 *   frequency - Desired timer frequency in Hz (e.g., 100 for 100 Hz)
 */
void PitInitialize(uint32_t frequency);

/*
 * PitGetTicks - Get the number of timer ticks since boot
 *
 * Returns: Number of timer interrupts that have occurred
 */
uint64_t PitGetTicks(void);

/*
 * PitGetFrequency - Get the current timer frequency
 *
 * Returns: Timer frequency in Hz
 */
uint32_t PitGetFrequency(void);

/*
 * PitRegisterTickHandler - Register a handler to be called on each timer tick
 *
 * Parameters:
 *   handler - Function to call on each tick (receives register state)
 */
void PitRegisterTickHandler(PitTickHandler handler);

#endif /* PIT_H */
