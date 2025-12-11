/* gdt.c - Global Descriptor Table implementation */

#include <stdint.h>
#include <stddef.h>
#include "gdt.h"
#include "early_console.h"

/* Number of GDT entries */
#define GDT_ENTRIES 5

/* GDT table */
static struct gdt_entry gdtEntries[GDT_ENTRIES];
static struct gdt_ptr gdtPointer;

/* External assembly function to load GDT */
extern void gdtFlush(uint32_t);

/*
 * gdtSetGate - Set a GDT entry
 */
static void gdtSetGate(int32_t num, uint32_t base, uint32_t limit,
                       uint8_t access, uint8_t gran)
{
    gdtEntries[num].baseLow = (base & 0xFFFF);
    gdtEntries[num].baseMiddle = (base >> 16) & 0xFF;
    gdtEntries[num].baseHigh = (base >> 24) & 0xFF;

    gdtEntries[num].limitLow = (limit & 0xFFFF);
    gdtEntries[num].granularity = (limit >> 16) & 0x0F;
    gdtEntries[num].granularity |= gran & 0xF0;

    gdtEntries[num].access = access;
}

/*
 * GdtInitialize - Initialize the Global Descriptor Table
 *
 * Sets up a flat memory model with separate code and data segments
 * for both kernel (ring 0) and user mode (ring 3).
 */
void GdtInitialize(void)
{
    gdtPointer.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gdtPointer.base = (uint32_t)&gdtEntries;

    // Null segment (required)
    gdtSetGate(0, 0, 0, 0, 0);

    // Kernel code segment
    gdtSetGate(1, 0, 0xFFFFFFFF,
               GDT_ACCESS_PRESENT | GDT_ACCESS_DESCRIPTOR | GDT_ACCESS_PRIV_RING0 |
               GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
               GDT_GRAN_4K | GDT_GRAN_32BIT);

    // Kernel data segment
    gdtSetGate(2, 0, 0xFFFFFFFF,
               GDT_ACCESS_PRESENT | GDT_ACCESS_DESCRIPTOR | GDT_ACCESS_PRIV_RING0 |
               GDT_ACCESS_RW,
               GDT_GRAN_4K | GDT_GRAN_32BIT);

    // User code segment
    gdtSetGate(3, 0, 0xFFFFFFFF,
               GDT_ACCESS_PRESENT | GDT_ACCESS_DESCRIPTOR | GDT_ACCESS_PRIV_RING3 |
               GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
               GDT_GRAN_4K | GDT_GRAN_32BIT);

    // User data segment
    gdtSetGate(4, 0, 0xFFFFFFFF,
               GDT_ACCESS_PRESENT | GDT_ACCESS_DESCRIPTOR | GDT_ACCESS_PRIV_RING3 |
               GDT_ACCESS_RW,
               GDT_GRAN_4K | GDT_GRAN_32BIT);

    // Load the GDT
    gdtFlush((uint32_t)&gdtPointer);
}
