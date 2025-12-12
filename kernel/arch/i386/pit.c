/* pit.c - Programmable Interval Timer (8253/8254) */

#include <stdint.h>
#include <stddef.h>
#include "pit.h"
#include "irq.h"
#include "isr.h"
#include "pic.h"
#include "x86.h"

/* PIT I/O ports */
#define PIT_CHANNEL0    0x40    /* Channel 0 data port (R/W) */
#define PIT_CHANNEL1    0x41    /* Channel 1 data port (R/W) */
#define PIT_CHANNEL2    0x42    /* Channel 2 data port (R/W) */
#define PIT_COMMAND     0x43    /* Mode/Command register (W) */

/* PIT frequency (1.193182 MHz) */
#define PIT_BASE_FREQ   1193182

/* Timer state */
static volatile uint64_t timerTicks = 0;
static uint32_t timerFrequency = 0;
static PitTickHandler tickHandler = NULL;

/*
 * pitIrqHandler - Timer interrupt handler (with register state)
 */
static void pitIrqHandler(registers_t* regs)
{
    timerTicks++;

    /* Call registered tick handler if present */
    if (tickHandler) {
        tickHandler(regs);
    }
}

/*
 * PitInitialize - Initialize the PIT
 */
void PitInitialize(uint32_t frequency)
{
    /* Calculate divisor for desired frequency */
    uint32_t divisor = PIT_BASE_FREQ / frequency;

    /* Make sure divisor is in valid range (1-65535) */
    if (divisor < 1) {
        divisor = 1;
    }
    if (divisor > 65535) {
        divisor = 65535;
    }

    /* Store actual frequency */
    timerFrequency = PIT_BASE_FREQ / divisor;

    /* Send command byte:
     * Bits 6-7: Channel 0
     * Bits 4-5: Access mode (lobyte/hibyte)
     * Bits 1-3: Mode 3 (square wave generator)
     * Bit 0:    Binary mode (not BCD)
     */
    outb(PIT_COMMAND, 0x36);

    /* Send frequency divisor (low byte, then high byte) */
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    /* Register IRQ handler for IRQ0 (timer) with register state */
    IrqRegisterHandlerWithRegs(IRQ0, pitIrqHandler);

    /* Enable IRQ0 (unmask it in PIC) */
    PicClearMask(IRQ0);
}

/*
 * PitRegisterTickHandler - Register a handler to be called on each tick
 */
void PitRegisterTickHandler(PitTickHandler handler)
{
    tickHandler = handler;
}

/*
 * PitGetTicks - Get the number of timer ticks since boot
 */
uint64_t PitGetTicks(void)
{
    return timerTicks;
}

/*
 * PitGetFrequency - Get the current timer frequency
 */
uint32_t PitGetFrequency(void)
{
    return timerFrequency;
}
