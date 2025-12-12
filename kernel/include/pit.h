/* pit.h - Programmable Interval Timer (8253/8254) */
#ifndef PIT_H
#define PIT_H

#include <stdint.h>

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

#endif /* PIT_H */
