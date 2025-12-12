/* pic.c - 8259 Programmable Interrupt Controller */

#include <stdint.h>
#include "pic.h"
#include "x86.h"

/* PIC I/O ports */
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

/* PIC commands */
#define PIC_EOI         0x20    /* End-of-interrupt command */

/* Initialization Command Words (ICW) */
#define ICW1_ICW4       0x01    /* ICW4 needed */
#define ICW1_SINGLE     0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04    /* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08    /* Level triggered mode */
#define ICW1_INIT       0x10    /* Initialization required */

#define ICW4_8086       0x01    /* 8086/88 mode */
#define ICW4_AUTO       0x02    /* Auto EOI */
#define ICW4_BUF_SLAVE  0x08    /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C    /* Buffered mode/master */
#define ICW4_SFNM       0x10    /* Special fully nested mode */

/*
 * PicInitialize - Initialize the 8259 PIC
 */
void PicInitialize(void)
{
    /* Save masks */
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    /* Start initialization sequence (in cascade mode) */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    /* ICW2: Set vector offsets */
    outb(PIC1_DATA, 32);  /* Master PIC: IRQ 0-7  -> INT 32-39 */
    io_wait();
    outb(PIC2_DATA, 40);  /* Slave PIC:  IRQ 8-15 -> INT 40-47 */
    io_wait();

    /* ICW3: Tell Master PIC there's a Slave PIC at IRQ2 (0000 0100) */
    outb(PIC1_DATA, 4);
    io_wait();
    /* Tell Slave PIC its cascade identity (0000 0010) */
    outb(PIC2_DATA, 2);
    io_wait();

    /* ICW4: Set 8086 mode */
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    /* Restore saved masks (or mask all interrupts) */
    outb(PIC1_DATA, 0xFF);  /* Mask all interrupts initially */
    outb(PIC2_DATA, 0xFF);

    (void)mask1;  /* Avoid unused variable warning */
    (void)mask2;
}

/*
 * PicSendEoi - Send End-Of-Interrupt signal to PIC
 */
void PicSendEoi(uint8_t irq)
{
    /* If IRQ came from slave PIC, send EOI to slave */
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    /* Always send EOI to master PIC */
    outb(PIC1_COMMAND, PIC_EOI);
}

/*
 * PicSetMask - Disable an IRQ line
 */
void PicSetMask(uint8_t irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) | (1 << irq);
    outb(port, value);
}

/*
 * PicClearMask - Enable an IRQ line
 */
void PicClearMask(uint8_t irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}
