/* gdt.h - Global Descriptor Table definitions */
#ifndef GDT_H
#define GDT_H

#include <stdint.h>

/*
 * GDT Entry structure
 * Describes a memory segment
 */
struct gdt_entry {
    uint16_t limitLow;      // Lower 16 bits of limit
    uint16_t baseLow;       // Lower 16 bits of base
    uint8_t baseMiddle;     // Next 8 bits of base
    uint8_t access;         // Access flags
    uint8_t granularity;    // Granularity and upper limit
    uint8_t baseHigh;       // Last 8 bits of base
} __attribute__((packed));

/*
 * GDT Pointer structure
 * Used to load the GDT with LGDT instruction
 */
struct gdt_ptr {
    uint16_t limit;         // Size of GDT - 1
    uint32_t base;          // Address of first GDT entry
} __attribute__((packed));

/* Access byte flags */
#define GDT_ACCESS_PRESENT      0x80  // Segment is present
#define GDT_ACCESS_PRIV_RING0   0x00  // Ring 0 (kernel)
#define GDT_ACCESS_PRIV_RING3   0x60  // Ring 3 (user)
#define GDT_ACCESS_DESCRIPTOR   0x10  // Descriptor type (1 = code/data)
#define GDT_ACCESS_EXECUTABLE   0x08  // Code segment
#define GDT_ACCESS_RW           0x02  // Readable (code) / Writable (data)
#define GDT_ACCESS_ACCESSED     0x01  // Accessed bit

/* Granularity byte flags */
#define GDT_GRAN_4K             0x80  // 4KB granularity
#define GDT_GRAN_32BIT          0x40  // 32-bit mode
#define GDT_GRAN_LIMIT_MASK     0x0F  // Upper 4 bits of limit

/* Public functions */
void GdtInitialize(void);

#endif /* GDT_H */
