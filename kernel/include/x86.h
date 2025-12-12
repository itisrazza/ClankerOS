/* x86.h - x86 instruction wrappers */
#ifndef X86_H
#define X86_H

#include <stdint.h>

/*
 * outb - Write a byte to an I/O port
 */
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/*
 * inb - Read a byte from an I/O port
 */
static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/*
 * io_wait - Short I/O delay using port 0x80
 */
static inline void io_wait(void)
{
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

#endif /* X86_H */
