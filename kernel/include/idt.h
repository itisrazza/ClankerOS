/* idt.h - Interrupt Descriptor Table definitions */
#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/*
 * IDT Entry structure
 * Describes an interrupt handler
 */
struct idt_entry {
    uint16_t baseLow;       // Lower 16 bits of handler address
    uint16_t selector;      // Kernel segment selector
    uint8_t always0;        // Always 0
    uint8_t flags;          // Flags
    uint16_t baseHigh;      // Upper 16 bits of handler address
} __attribute__((packed));

/*
 * IDT Pointer structure
 * Used to load the IDT with LIDT instruction
 */
struct idt_ptr {
    uint16_t limit;         // Size of IDT - 1
    uint32_t base;          // Address of first IDT entry
} __attribute__((packed));

/* IDT flags */
#define IDT_FLAG_PRESENT    0x80  // Interrupt is present
#define IDT_FLAG_RING0      0x00  // Ring 0 (kernel)
#define IDT_FLAG_RING3      0x60  // Ring 3 (user)
#define IDT_FLAG_GATE_32    0x0E  // 32-bit interrupt gate
#define IDT_FLAG_GATE_TRAP  0x0F  // 32-bit trap gate

/* Number of IDT entries (256 for x86) */
#define IDT_ENTRIES 256

/* Public functions */
void IdtInitialize(void);
void IdtSetGate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

#endif /* IDT_H */
