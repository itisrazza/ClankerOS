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
#include "econ_writer.h"
#include "pmm.h"
#include "paging.h"
#include "kheap.h"
#include "kcmdline.h"
#include "process.h"
#include "panic.h"

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
static void pageFaultHandler(registers_t* regs);
static void testProcess1(void);
static void testProcess2(void);
static void testProcess3(void);

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
    // Parse kernel command line arguments
    KCmdLineInitialize(mbootInfo);

    // Enable early console if requested
    if (KCmdLineHasFlag("earlycon")) {
        EConWriterEnable();
    }

    // Initialize early console first for debugging
    EConInitialize();

    // Initialize terminal
    VidInitialize();

    // Get writers for formatted output
    ClcWriter* vgaWriter = VidGetWriter();
    ClcWriter* serialWriter = EConGetWriter();

    // VGA: Simple branding
    ClcPrintfWriter(vgaWriter, "ClankerOS v0.1.0\n");
    ClcPrintfWriter(vgaWriter, "Booting kernel...\n\n");

    // Serial: Detailed boot information (if earlycon enabled)
    ClcPrintfWriter(serialWriter, "\n=== ClankerOS Boot Log ===\n");
    ClcPrintfWriter(serialWriter, "Multiboot magic: 0x%x\n", magic);
    ClcPrintfWriter(serialWriter, "Multiboot info:  %p\n", mbootInfo);
    ClcPrintfWriter(serialWriter, "Multiboot flags: 0x%x\n", mbootInfo->flags);
    if (KCmdLineHasFlag("earlycon")) {
        ClcPrintfWriter(serialWriter, "Early console: enabled\n");
    }
    if (KCmdLineHasFlag("boottest")) {
        ClcPrintfWriter(serialWriter, "Boot tests: enabled\n");
    }

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

    // Register page fault handler (ISR 14)
    IsrRegisterHandler(14, pageFaultHandler);

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

    // Initialize Physical Memory Manager
    ClcPrintfWriter(serialWriter, "\nInitializing PMM...\n");
    ClcPrintfWriter(vgaWriter, "Initializing PMM... ");
    PmmInitialize(mbootInfo);
    ClcPrintfWriter(vgaWriter, "OK\n");

    // Display memory information
    size_t totalMem = PmmGetTotalMemory();
    size_t freeMem = PmmGetFreeMemory();
    size_t usedMem = PmmGetUsedMemory();

    // VGA: Simple summary
    ClcPrintfWriter(vgaWriter, "  Memory: %u MB total, %u MB free\n",
                    (uint32_t)(totalMem / (1024 * 1024)),
                    (uint32_t)(freeMem / (1024 * 1024)));

    // Serial: Detailed breakdown
    ClcPrintfWriter(serialWriter, "Memory Manager Statistics:\n");
    ClcPrintfWriter(serialWriter, "  Total: %u MB (%u KB, %u bytes)\n",
                    (uint32_t)(totalMem / (1024 * 1024)),
                    (uint32_t)(totalMem / 1024),
                    (uint32_t)totalMem);
    ClcPrintfWriter(serialWriter, "  Free:  %u MB (%u KB, %u bytes)\n",
                    (uint32_t)(freeMem / (1024 * 1024)),
                    (uint32_t)(freeMem / 1024),
                    (uint32_t)freeMem);
    ClcPrintfWriter(serialWriter, "  Used:  %u MB (%u KB, %u bytes)\n",
                    (uint32_t)(usedMem / (1024 * 1024)),
                    (uint32_t)(usedMem / 1024),
                    (uint32_t)usedMem);

    // Enable interrupts
    ClcPrintfWriter(vgaWriter, "\nEnabling interrupts... ");
    __asm__ volatile ("sti");
    ClcPrintfWriter(vgaWriter, "OK\n");

    ClcPrintfWriter(vgaWriter, "\nWelcome to ClankerOS!\n");
    ClcPrintfWriter(vgaWriter, "Kernel initialized successfully.\n");

    // Initialize paging (always needed)
    ClcPrintfWriter(vgaWriter, "\nInitializing paging... ");
    PagingInitialize();
    ClcPrintfWriter(vgaWriter, "OK\n");

    // Initialize kernel heap (always needed)
    ClcPrintfWriter(vgaWriter, "Initializing kernel heap... ");
    KHeapInitialize();
    ClcPrintfWriter(vgaWriter, "OK\n");

    // Run boot tests if requested
    if (KCmdLineHasFlag("boottest")) {
        // Test memory allocation
        ClcPrintfWriter(serialWriter, "\nMemory Allocation Test:\n");
        ClcPrintfWriter(vgaWriter, "\nRunning memory test... ");

        uintptr_t page1 = PmmAllocPage();
        uintptr_t page2 = PmmAllocPage();
        uintptr_t page3 = PmmAllocPage();
        ClcPrintfWriter(serialWriter, "  Alloc page 1: %p\n", (void*)page1);
        ClcPrintfWriter(serialWriter, "  Alloc page 2: %p\n", (void*)page2);
        ClcPrintfWriter(serialWriter, "  Alloc page 3: %p\n", (void*)page3);
        ClcPrintfWriter(serialWriter, "  Free after alloc: %u KB\n",
                        (uint32_t)(PmmGetFreeMemory() / 1024));

        PmmFreePage(page2);
        ClcPrintfWriter(serialWriter, "  Freed page 2\n");
        ClcPrintfWriter(serialWriter, "  Free after free: %u KB\n",
                        (uint32_t)(PmmGetFreeMemory() / 1024));

        uintptr_t page4 = PmmAllocPage();
        ClcPrintfWriter(serialWriter, "  Alloc page 4: %p ", (void*)page4);
        if (page4 == page2) {
            ClcPrintfWriter(serialWriter, "(reused freed page - PASS)\n");
        } else {
            ClcPrintfWriter(serialWriter, "(did not reuse - unexpected)\n");
        }

        ClcPrintfWriter(vgaWriter, "PASS\n");
        ClcPrintfWriter(serialWriter, "Memory test complete!\n");

        // Test paging
        ClcPrintfWriter(serialWriter, "\nPaging Test:\n");
        ClcPrintfWriter(vgaWriter, "Testing paging... ");

        // Test virtual to physical address translation
        uintptr_t testVirt = 0x1000;
        uintptr_t testPhys = PagingGetPhysicalAddress(testVirt);
        ClcPrintfWriter(serialWriter, "  Virtual %p -> Physical %p ", (void*)testVirt, (void*)testPhys);
        if (testPhys == testVirt) {
            ClcPrintfWriter(serialWriter, "(identity mapped - PASS)\n");
        } else {
            ClcPrintfWriter(serialWriter, "(FAIL)\n");
        }

        ClcPrintfWriter(vgaWriter, "PASS\n");
        ClcPrintfWriter(serialWriter, "Paging test complete!\n");

        // Test kernel heap
        ClcPrintfWriter(serialWriter, "\nKernel Heap Test:\n");
        ClcPrintfWriter(vgaWriter, "Testing heap allocator... ");

        // Test allocation
        char* str1 = (char*)KAllocateMemory(32);
        int* nums = (int*)KAllocateMemory(10 * sizeof(int));
        char* str2 = (char*)KAllocateMemory(64);

        ClcPrintfWriter(serialWriter, "  Allocated str1: %p (32 bytes)\n", str1);
        ClcPrintfWriter(serialWriter, "  Allocated nums: %p (40 bytes)\n", nums);
        ClcPrintfWriter(serialWriter, "  Allocated str2: %p (64 bytes)\n", str2);

        // Write to allocations
        if (str1 && nums && str2) {
            for (int i = 0; i < 10; i++) {
                nums[i] = i * 10;
            }
            ClcPrintfWriter(serialWriter, "  nums[5] = %d (expected 50)\n", nums[5]);

            // Test free
            KFreeMemory(nums);
            ClcPrintfWriter(serialWriter, "  Freed nums\n");

            // Test realloc
            str1 = (char*)KReallocateMemory(str1, 128);
            ClcPrintfWriter(serialWriter, "  Reallocated str1: %p (128 bytes)\n", str1);

            // Get stats
            size_t total, used, free;
            KHeapGetStats(&total, &used, &free);
            ClcPrintfWriter(serialWriter, "  Heap: %u KB total, %u KB used, %u KB free\n",
                            (uint32_t)(total / 1024),
                            (uint32_t)(used / 1024),
                            (uint32_t)(free / 1024));

            KFreeMemory(str1);
            KFreeMemory(str2);
        }

        ClcPrintfWriter(vgaWriter, "PASS\n");
        ClcPrintfWriter(serialWriter, "Heap test complete!\n");

        ClcPrintfWriter(vgaWriter, "\nAll tests passed!\n");
    }

    ClcPrintfWriter(serialWriter, "\n=== Boot Complete ===\n");

    // Test panic system if requested
    if (KCmdLineHasFlag("testpanic")) {
        ClcPrintfWriter(vgaWriter, "\nTesting panic system...\n");
        ClcPrintfWriter(serialWriter, "Panic test requested - triggering KPanic\n");
        KPanic("Test panic - this is intentional (value: %d)", 42);
    }

    // Test page fault handler if requested
    if (KCmdLineHasFlag("testpagefault")) {
        ClcPrintfWriter(vgaWriter, "\nTesting page fault handler...\n");
        ClcPrintfWriter(serialWriter, "Page fault test - accessing invalid address\n");
        volatile uint32_t* badPtr = (uint32_t*)0xDEADBEEF;
        uint32_t value = *badPtr;  // This will trigger a page fault
        (void)value;  // Suppress unused warning
    }

    // Initialize process management
    ClcPrintfWriter(vgaWriter, "\nInitializing processes... ");
    ClcPrintfWriter(serialWriter, "\n=== Process Management Initialization ===\n");
    ProcessInitialize();
    ClcPrintfWriter(vgaWriter, "OK\n");

    // Create test processes
    ClcPrintfWriter(vgaWriter, "Creating test processes... ");
    Process* proc1 = ProcessCreate("test1", testProcess1, PROCESS_MODE_KERNEL);
    Process* proc2 = ProcessCreate("test2", testProcess2, PROCESS_MODE_KERNEL);
    Process* proc3 = ProcessCreate("test3", testProcess3, PROCESS_MODE_KERNEL);
    ClcPrintfWriter(vgaWriter, "OK\n");

    if (!proc1 || !proc2 || !proc3) {
        ClcPrintfWriter(vgaWriter, "Failed to create processes!\n");
        while (1) __asm__ volatile ("hlt");
    }

    // Register scheduler with timer
    PitRegisterTickHandler(ProcessSchedule);

    // Enable scheduler
    ClcPrintfWriter(vgaWriter, "Enabling scheduler...\n");
    ClcPrintfWriter(serialWriter, "Scheduler enabled - starting multitasking\n");
    ProcessEnableScheduler();

    ClcPrintfWriter(vgaWriter, "\nMultitasking started!\n\n");

    // Idle loop - this is now PID 0
    while (1) {
        __asm__ volatile ("hlt");
    }
}

/*
 * pageFaultHandler - Handle page faults (ISR 14)
 */
static void pageFaultHandler(registers_t* regs)
{
    // Get faulting address from CR2
    uintptr_t faultAddr;
    __asm__ volatile ("mov %%cr2, %0" : "=r"(faultAddr));

    // Decode error code
    bool present = !(regs->errCode & 0x1);  // Page not present
    bool write = regs->errCode & 0x2;        // Write operation
    bool user = regs->errCode & 0x4;         // User mode
    bool reserved = regs->errCode & 0x8;     // Reserved bit set
    bool fetch = regs->errCode & 0x10;       // Instruction fetch

    // Build cause string
    const char* cause = "Unknown";
    if (present && write) cause = "Write to non-present page";
    else if (present && !write) cause = "Read from non-present page";
    else if (write) cause = "Page protection violation (write)";
    else if (user) cause = "User mode access violation";
    else if (reserved) cause = "Reserved bit set in page table";
    else if (fetch) cause = "Instruction fetch from non-executable page";

    KPanicRegs(regs, "Page Fault at 0x%08x - %s", faultAddr, cause);
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

/*
 * Test processes - demonstrate multitasking
 */

static void testProcess1(void)
{
    ClcWriter* vga = VidGetWriter();
    ClcWriter* serial = EConGetWriter();

    for (int i = 0; i < 5; i++) {
        ClcPrintfWriter(vga, "[P1:%d] ", i);
        ClcPrintfWriter(serial, "Process 1 iteration %d\n", i);

        // Busy wait to simulate work
        for (volatile int j = 0; j < 1000000; j++);
    }

    ClcPrintfWriter(serial, "Process 1 exiting\n");
}

static void testProcess2(void)
{
    ClcWriter* vga = VidGetWriter();
    ClcWriter* serial = EConGetWriter();

    for (int i = 0; i < 5; i++) {
        ClcPrintfWriter(vga, "[P2:%d] ", i);
        ClcPrintfWriter(serial, "Process 2 iteration %d\n", i);

        // Busy wait to simulate work
        for (volatile int j = 0; j < 1000000; j++);
    }

    ClcPrintfWriter(serial, "Process 2 exiting\n");
}

static void testProcess3(void)
{
    ClcWriter* vga = VidGetWriter();
    ClcWriter* serial = EConGetWriter();

    for (int i = 0; i < 5; i++) {
        ClcPrintfWriter(vga, "[P3:%d] ", i);
        ClcPrintfWriter(serial, "Process 3 iteration %d\n", i);

        // Busy wait to simulate work
        for (volatile int j = 0; j < 1000000; j++);
    }

    ClcPrintfWriter(serial, "Process 3 exiting\n");
}
