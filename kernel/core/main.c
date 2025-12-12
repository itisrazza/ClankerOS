/* main.c - ClankerOS kernel entry point */

#include <stdint.h>
#include <stddef.h>
#include "multiboot.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "pic.h"
#include "pit.h"
#include "early_console.h"
#include "clc/printf.h"
#include "vid_writer.h"

/* VGA text mode buffer */
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

/* VGA color codes */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

/* Terminal state */
static size_t terminalRow;
static size_t terminalColumn;
static uint8_t terminalColor;
static uint16_t* terminalBuffer;

/* Private function declarations */
static inline uint8_t vgaEntryColor(enum vga_color fg, enum vga_color bg);
static inline uint16_t vgaEntry(unsigned char uc, uint8_t color);

/*
 * VidInitialize - Initialize VGA text mode display
 */
void VidInitialize(void)
{
    terminalRow = 0;
    terminalColumn = 0;
    terminalColor = vgaEntryColor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminalBuffer = (uint16_t*)VGA_MEMORY;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminalBuffer[index] = vgaEntry(' ', terminalColor);
        }
    }
}

/*
 * VidPutChar - Write a single character to the display
 */
void VidPutChar(char c)
{
    if (c == '\n') {
        terminalColumn = 0;
        if (++terminalRow == VGA_HEIGHT) {
            terminalRow = 0;  // Wrap to top (will scroll later)
        }
        return;
    }

    const size_t index = terminalRow * VGA_WIDTH + terminalColumn;
    terminalBuffer[index] = vgaEntry(c, terminalColor);

    if (++terminalColumn == VGA_WIDTH) {
        terminalColumn = 0;
        if (++terminalRow == VGA_HEIGHT) {
            terminalRow = 0;  // Wrap to top (will scroll later)
        }
    }
}

/*
 * VidWriteString - Write a null-terminated string to the display
 */
void VidWriteString(const char* data)
{
    size_t i = 0;
    while (data[i]) {
        VidPutChar(data[i]);
        i++;
    }
}

/*
 * KMain - Kernel entry point
 *
 * Called by boot.s after stack setup with multiboot information
 */
void KMain(uint32_t magic, multiboot_info_t* mbootInfo)
{
    // Initialize early console first for debugging
    EConInitialize();

    // Initialize terminal
    VidInitialize();

    // Get VGA writer for formatted output
    ClcWriter* vgaWriter = VidGetWriter();

    ClcPrintfWriter(vgaWriter, "ClankerOS v0.1.0\n");
    ClcPrintfWriter(vgaWriter, "Booting kernel...\n\n");

    // Initialize GDT
    ClcPrintfWriter(vgaWriter, "Initializing GDT... ");
    GdtInitialize();
    ClcPrintfWriter(vgaWriter, "OK\n");

    // Initialize IDT
    ClcPrintfWriter(vgaWriter, "Initializing IDT... ");
    IdtInitialize();
    ClcPrintfWriter(vgaWriter, "OK\n");

    // Initialize ISRs
    ClcPrintfWriter(vgaWriter, "Initializing ISRs... ");
    IsrInitialize();
    ClcPrintfWriter(vgaWriter, "OK\n");

    // Initialize IRQs
    ClcPrintfWriter(vgaWriter, "Initializing IRQs... ");
    IrqInitialize();
    ClcPrintfWriter(vgaWriter, "OK\n");

    // Initialize PIC
    ClcPrintfWriter(vgaWriter, "Initializing PIC... ");
    PicInitialize();
    ClcPrintfWriter(vgaWriter, "OK\n");

    // Initialize PIT timer (100 Hz)
    ClcPrintfWriter(vgaWriter, "Initializing PIT... ");
    PitInitialize(100);
    ClcPrintfWriter(vgaWriter, "OK (100 Hz)\n");

    // Enable interrupts
    ClcPrintfWriter(vgaWriter, "Enabling interrupts... ");
    __asm__ volatile ("sti");
    ClcPrintfWriter(vgaWriter, "OK\n");

    ClcPrintfWriter(vgaWriter, "\nWelcome to ClankerOS!\n");
    ClcPrintfWriter(vgaWriter, "Kernel initialized successfully.\n");

    // Test formatted output
    ClcPrintfWriter(vgaWriter, "\nMultiboot magic: 0x%x\n", magic);
    ClcPrintfWriter(vgaWriter, "Multiboot info at: %p\n", mbootInfo);

    // Display timer ticks for a few seconds
    ClcPrintfWriter(vgaWriter, "\nTimer test (watching ticks for 3 seconds):\n");
    uint64_t startTicks = PitGetTicks();
    uint64_t lastTicks = startTicks;
    while (PitGetTicks() < startTicks + 300) {  // 3 seconds at 100 Hz
        uint64_t currentTicks = PitGetTicks();
        if (currentTicks != lastTicks) {
            ClcPrintfWriter(vgaWriter, "Ticks: %u\n", (uint32_t)currentTicks);
            lastTicks = currentTicks;
        }
    }
    ClcPrintfWriter(vgaWriter, "Timer test complete!\n");

    // Halt - we'll add more functionality later
    while (1) {
        __asm__ volatile ("hlt");
    }
}

/* Private function implementations */

static inline uint8_t vgaEntryColor(enum vga_color fg, enum vga_color bg)
{
    return fg | bg << 4;
}

static inline uint16_t vgaEntry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}
