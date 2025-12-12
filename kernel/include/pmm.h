/* pmm.h - Physical Memory Manager */
#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "multiboot.h"

/* Page size (4KB) */
#define PAGE_SIZE 4096

/* Memory region types from multiboot */
#define MULTIBOOT_MEMORY_AVAILABLE        1
#define MULTIBOOT_MEMORY_RESERVED         2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS              4
#define MULTIBOOT_MEMORY_BADRAM           5

/*
 * PmmInitialize - Initialize physical memory manager
 *
 * Parses the multiboot memory map and sets up the physical page allocator.
 * Must be called before any memory allocation.
 *
 * @mbootInfo: Multiboot information structure from bootloader
 */
void PmmInitialize(multiboot_info_t* mbootInfo);

/*
 * PmmAllocPage - Allocate a single physical page (4KB)
 *
 * Returns the physical address of the allocated page, or 0 if out of memory.
 *
 * @return: Physical address of allocated page, or 0 on failure
 */
uintptr_t PmmAllocPage(void);

/*
 * PmmFreePage - Free a physical page
 *
 * @addr: Physical address of page to free (must be page-aligned)
 */
void PmmFreePage(uintptr_t addr);

/*
 * PmmGetTotalMemory - Get total physical memory in bytes
 *
 * @return: Total memory in bytes
 */
size_t PmmGetTotalMemory(void);

/*
 * PmmGetFreeMemory - Get free physical memory in bytes
 *
 * @return: Free memory in bytes
 */
size_t PmmGetFreeMemory(void);

/*
 * PmmGetUsedMemory - Get used physical memory in bytes
 *
 * @return: Used memory in bytes
 */
size_t PmmGetUsedMemory(void);

#endif /* PMM_H */
