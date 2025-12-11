/* early_console.c - Early boot console (COM1 serial port) */

#include <stdint.h>
#include "early_console.h"

/* Port I/O helpers */
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/*
 * EConInitialize - Initialize COM1 serial port for early output
 *
 * Sets up 38400 baud, 8N1 (8 data bits, no parity, 1 stop bit)
 */
void EConInitialize(void)
{
    outb(COM1_PORT + 1, 0x00);    // Disable interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear, 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

/*
 * econIsTransmitEmpty - Check if transmit buffer is empty
 */
static int econIsTransmitEmpty(void)
{
    return inb(COM1_PORT + 5) & 0x20;
}

/*
 * EConPutChar - Write a single character to COM1
 */
void EConPutChar(char c)
{
    // Convert \n to \r\n for proper terminal display
    if (c == '\n') {
        while (econIsTransmitEmpty() == 0);
        outb(COM1_PORT, '\r');
    }

    while (econIsTransmitEmpty() == 0);
    outb(COM1_PORT, c);
}

/*
 * EConWriteString - Write a null-terminated string to COM1
 */
void EConWriteString(const char* str)
{
    while (*str) {
        EConPutChar(*str);
        str++;
    }
}
