/* idt.c - Interrupt Descriptor Table implementation */

#include <stdint.h>
#include <stddef.h>
#include "idt.h"

/* IDT table */
static struct idt_entry idtEntries[IDT_ENTRIES];
static struct idt_ptr idtPointer;

/* External assembly function to load IDT */
extern void idtFlush(uint32_t);

/*
 * IdtSetGate - Set an IDT entry
 */
void IdtSetGate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags)
{
    idtEntries[num].baseLow = base & 0xFFFF;
    idtEntries[num].baseHigh = (base >> 16) & 0xFFFF;
    idtEntries[num].selector = selector;
    idtEntries[num].always0 = 0;
    idtEntries[num].flags = flags;
}

/*
 * IdtInitialize - Initialize the Interrupt Descriptor Table
 *
 * Sets up the IDT structure and loads it into the CPU.
 * Individual interrupt handlers are registered separately.
 */
void IdtInitialize(void)
{
    idtPointer.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtPointer.base = (uint32_t)&idtEntries;

    // Clear all IDT entries
    for (int i = 0; i < IDT_ENTRIES; i++) {
        IdtSetGate(i, 0, 0, 0);
    }

    // Load the IDT
    idtFlush((uint32_t)&idtPointer);
}
