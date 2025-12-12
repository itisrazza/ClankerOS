/* pmm.c - Physical Memory Manager Implementation */

#include "pmm.h"
#include "multiboot.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Bitmap for tracking physical pages */
static uint32_t* pageBitmap = NULL;
static size_t bitmapSize = 0;       // Size in uint32_t entries
static size_t totalPages = 0;
static size_t freePages = 0;
static size_t usedPages = 0;

/* Kernel end symbol (defined in linker script) */
extern uint32_t kernelEnd;

/* Bitmap operations */
#define BITMAP_INDEX(page) ((page) / 32)
#define BITMAP_OFFSET(page) ((page) % 32)
#define BITMAP_SET(page) (pageBitmap[BITMAP_INDEX(page)] |= (1 << BITMAP_OFFSET(page)))
#define BITMAP_CLEAR(page) (pageBitmap[BITMAP_INDEX(page)] &= ~(1 << BITMAP_OFFSET(page)))
#define BITMAP_TEST(page) (pageBitmap[BITMAP_INDEX(page)] & (1 << BITMAP_OFFSET(page)))

/*
 * pageToAddress - Convert page number to physical address
 */
static inline uintptr_t pageToAddress(size_t page)
{
    return page * PAGE_SIZE;
}

/*
 * addressToPage - Convert physical address to page number
 */
static inline size_t addressToPage(uintptr_t addr)
{
    return addr / PAGE_SIZE;
}

/*
 * markPageUsed - Mark a page as used in the bitmap
 */
static void markPageUsed(size_t page)
{
    if (page >= totalPages) {
        return;
    }

    if (!BITMAP_TEST(page)) {
        BITMAP_SET(page);
        usedPages++;
        freePages--;
    }
}

/*
 * markPageFree - Mark a page as free in the bitmap
 */
static void markPageFree(size_t page)
{
    if (page >= totalPages) {
        return;
    }

    if (BITMAP_TEST(page)) {
        BITMAP_CLEAR(page);
        freePages++;
        usedPages--;
    }
}

/*
 * markRegionUsed - Mark a memory region as used
 */
static void markRegionUsed(uintptr_t start, size_t length)
{
    size_t startPage = addressToPage(start);
    size_t endPage = addressToPage(start + length - 1);

    for (size_t page = startPage; page <= endPage; page++) {
        markPageUsed(page);
    }
}

/*
 * markRegionFree - Mark a memory region as free
 */
static void markRegionFree(uintptr_t start, size_t length)
{
    size_t startPage = addressToPage(start);
    size_t endPage = addressToPage(start + length - 1);

    for (size_t page = startPage; page <= endPage; page++) {
        markPageFree(page);
    }
}

/*
 * PmmInitialize - Initialize physical memory manager
 */
void PmmInitialize(multiboot_info_t* mbootInfo)
{
    // Check if memory map is available
    if (!(mbootInfo->flags & (1 << 6))) {
        // No memory map, fall back to mem_lower/mem_upper
        // Total memory = mem_lower (KB) + mem_upper (KB)
        totalPages = ((mbootInfo->mem_lower + mbootInfo->mem_upper) * 1024) / PAGE_SIZE;
    } else {
        // Parse memory map to find highest address
        uint32_t mmapAddr = mbootInfo->mmap_addr;
        uint32_t mmapEnd = mmapAddr + mbootInfo->mmap_length;
        uintptr_t highestAddr = 0;

        while (mmapAddr < mmapEnd) {
            multiboot_mmap_entry_t* entry = (multiboot_mmap_entry_t*)mmapAddr;
            uintptr_t regionEnd = (uintptr_t)(entry->addr + entry->len);

            if (regionEnd > highestAddr) {
                highestAddr = regionEnd;
            }

            // Move to next entry (entry->size doesn't include the size field itself)
            mmapAddr += entry->size + sizeof(entry->size);
        }

        totalPages = addressToPage(highestAddr);
    }

    // Calculate bitmap size (1 bit per page, packed into uint32_t)
    bitmapSize = (totalPages + 31) / 32;  // Round up

    // Place bitmap right after kernel end (align to 4 bytes)
    pageBitmap = (uint32_t*)(((uintptr_t)&kernelEnd + 3) & ~3);

    // Initialize bitmap - mark all pages as used initially
    for (size_t i = 0; i < bitmapSize; i++) {
        pageBitmap[i] = 0xFFFFFFFF;  // All bits set = all used
    }
    usedPages = totalPages;
    freePages = 0;

    // Now parse memory map and mark available regions as free
    if (mbootInfo->flags & (1 << 6)) {
        uint32_t mmapAddr = mbootInfo->mmap_addr;
        uint32_t mmapEnd = mmapAddr + mbootInfo->mmap_length;

        while (mmapAddr < mmapEnd) {
            multiboot_mmap_entry_t* entry = (multiboot_mmap_entry_t*)mmapAddr;

            // Only mark AVAILABLE memory as free
            if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                markRegionFree((uintptr_t)entry->addr, (size_t)entry->len);
            }

            mmapAddr += entry->size + sizeof(entry->size);
        }
    } else {
        // Simple case: mark everything above 1MB as free
        // (First 1MB is reserved for BIOS, VGA, etc.)
        markRegionFree(0x100000, (mbootInfo->mem_upper * 1024));
    }

    // Mark kernel memory as used (from 1MB to end of bitmap)
    uintptr_t kernelStart = 0x100000;  // Kernel loaded at 1MB
    uintptr_t bitmapEnd = (uintptr_t)pageBitmap + (bitmapSize * sizeof(uint32_t));
    markRegionUsed(kernelStart, bitmapEnd - kernelStart);

    // Mark low memory (0-1MB) as used
    markRegionUsed(0, 0x100000);
}

/*
 * PmmAllocPage - Allocate a single physical page
 */
uintptr_t PmmAllocPage(void)
{
    // Find first free page
    for (size_t i = 0; i < bitmapSize; i++) {
        if (pageBitmap[i] != 0xFFFFFFFF) {
            // This uint32_t has at least one free page
            for (size_t bit = 0; bit < 32; bit++) {
                size_t page = i * 32 + bit;
                if (page >= totalPages) {
                    return 0;  // Out of bounds
                }

                if (!BITMAP_TEST(page)) {
                    // Found free page
                    markPageUsed(page);
                    return pageToAddress(page);
                }
            }
        }
    }

    // Out of memory
    return 0;
}

/*
 * PmmFreePage - Free a physical page
 */
void PmmFreePage(uintptr_t addr)
{
    // Ensure address is page-aligned
    if (addr & (PAGE_SIZE - 1)) {
        return;  // Not page-aligned
    }

    size_t page = addressToPage(addr);
    markPageFree(page);
}

/*
 * PmmGetTotalMemory - Get total physical memory in bytes
 */
size_t PmmGetTotalMemory(void)
{
    return totalPages * PAGE_SIZE;
}

/*
 * PmmGetFreeMemory - Get free physical memory in bytes
 */
size_t PmmGetFreeMemory(void)
{
    return freePages * PAGE_SIZE;
}

/*
 * PmmGetUsedMemory - Get used physical memory in bytes
 */
size_t PmmGetUsedMemory(void)
{
    return usedPages * PAGE_SIZE;
}
