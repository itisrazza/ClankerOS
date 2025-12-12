/* paging.c - Virtual Memory Management Implementation */

#include "paging.h"
#include "pmm.h"
#include "early_console.h"
#include "clc/printf.h"
#include "econ_writer.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Kernel page directory */
static PageDirectory* kernelPageDirectory = NULL;

/* Helper macros */
#define PAGE_DIRECTORY_INDEX(addr) (((addr) >> 22) & 0x3FF)
#define PAGE_TABLE_INDEX(addr)     (((addr) >> 12) & 0x3FF)
#define PAGE_ALIGN(addr)           ((addr) & ~0xFFF)
#define PAGE_GET_PHYSICAL(entry)   ((entry) & ~0xFFF)

/*
 * pagingGetPageTable - Get or create a page table for a virtual address
 */
static PageTable* pagingGetPageTable(uintptr_t virtualAddr, bool create)
{
    uint32_t pdIndex = PAGE_DIRECTORY_INDEX(virtualAddr);
    PageDirectoryEntry* pde = &kernelPageDirectory->entries[pdIndex];

    // Check if page table exists
    if (*pde & PAGE_PRESENT) {
        // Page table exists, return it
        uintptr_t tablePhys = PAGE_GET_PHYSICAL(*pde);
        return (PageTable*)tablePhys;
    }

    // Page table doesn't exist
    if (!create) {
        return NULL;
    }

    // Allocate new page table
    uintptr_t tablePhys = PmmAllocPage();
    if (tablePhys == 0) {
        return NULL;  // Out of memory
    }

    // Clear the page table
    PageTable* table = (PageTable*)tablePhys;
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        table->entries[i] = 0;
    }

    // Install page table in directory
    *pde = tablePhys | PAGE_PRESENT | PAGE_WRITE;

    return table;
}

/*
 * PagingMapPage - Map a virtual page to a physical page
 */
bool PagingMapPage(uintptr_t virtualAddr, uintptr_t physicalAddr, uint32_t flags)
{
    // Get or create page table
    PageTable* table = pagingGetPageTable(virtualAddr, true);
    if (table == NULL) {
        return false;
    }

    // Get page table entry
    uint32_t ptIndex = PAGE_TABLE_INDEX(virtualAddr);
    PageTableEntry* pte = &table->entries[ptIndex];

    // Map the page
    *pte = PAGE_ALIGN(physicalAddr) | flags;

    // Invalidate TLB entry
    PagingInvalidatePage(virtualAddr);

    return true;
}

/*
 * PagingUnmapPage - Unmap a virtual page
 */
void PagingUnmapPage(uintptr_t virtualAddr)
{
    PageTable* table = pagingGetPageTable(virtualAddr, false);
    if (table == NULL) {
        return;  // Not mapped
    }

    uint32_t ptIndex = PAGE_TABLE_INDEX(virtualAddr);
    table->entries[ptIndex] = 0;

    PagingInvalidatePage(virtualAddr);
}

/*
 * PagingGetPhysicalAddress - Get physical address for virtual address
 */
uintptr_t PagingGetPhysicalAddress(uintptr_t virtualAddr)
{
    PageTable* table = pagingGetPageTable(virtualAddr, false);
    if (table == NULL) {
        return 0;  // Not mapped
    }

    uint32_t ptIndex = PAGE_TABLE_INDEX(virtualAddr);
    PageTableEntry pte = table->entries[ptIndex];

    if (!(pte & PAGE_PRESENT)) {
        return 0;  // Not present
    }

    return PAGE_GET_PHYSICAL(pte) | (virtualAddr & 0xFFF);
}

/*
 * PagingInvalidatePage - Invalidate TLB entry for a page
 */
void PagingInvalidatePage(uintptr_t virtualAddr)
{
    __asm__ volatile ("invlpg (%0)" : : "r"(virtualAddr) : "memory");
}

/*
 * PagingGetCurrentDirectory - Get current page directory
 */
PageDirectory* PagingGetCurrentDirectory(void)
{
    return kernelPageDirectory;
}

/*
 * PagingSwitchDirectory - Switch to a different page directory
 */
void PagingSwitchDirectory(uintptr_t pageDir)
{
    __asm__ volatile ("mov %0, %%cr3" : : "r"(pageDir));
}

/*
 * PagingInitialize - Initialize paging with identity mapping
 */
void PagingInitialize(void)
{
    ClcWriter* serial = EConGetWriter();

    ClcPrintfWriter(serial, "\nInitializing paging...\n");

    // Allocate page directory
    uintptr_t pdPhys = PmmAllocPage();
    if (pdPhys == 0) {
        ClcPrintfWriter(serial, "ERROR: Failed to allocate page directory\n");
        return;
    }

    kernelPageDirectory = (PageDirectory*)pdPhys;

    // Clear page directory
    for (int i = 0; i < PAGE_DIRECTORY_SIZE; i++) {
        kernelPageDirectory->entries[i] = 0;
    }

    ClcPrintfWriter(serial, "  Page directory at: %p\n", (void*)pdPhys);

    // Identity map first 4MB (kernel + low memory)
    // This ensures kernel code/data remains accessible after paging is enabled
    ClcPrintfWriter(serial, "  Identity mapping first 4MB...\n");

    for (uintptr_t addr = 0; addr < 0x400000; addr += PAGE_SIZE) {
        if (!PagingMapPage(addr, addr, PAGE_PRESENT | PAGE_WRITE)) {
            ClcPrintfWriter(serial, "ERROR: Failed to map page at %p\n", (void*)addr);
            return;
        }
    }

    ClcPrintfWriter(serial, "  Mapped 1024 pages (4MB)\n");

    // Enable paging
    ClcPrintfWriter(serial, "  Enabling paging...\n");

    // Load page directory into CR3
    __asm__ volatile ("mov %0, %%cr3" : : "r"(pdPhys));

    // Enable paging by setting bit 31 of CR0
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  // Set PG bit
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));

    ClcPrintfWriter(serial, "  Paging enabled!\n");
}
