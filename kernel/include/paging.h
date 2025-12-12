/* paging.h - Virtual Memory Management (Paging) */
#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stdbool.h>

/* Page directory/table entry flags */
#define PAGE_PRESENT    0x001  // Page is present in memory
#define PAGE_WRITE      0x002  // Page is writable
#define PAGE_USER       0x004  // Page is accessible from user mode
#define PAGE_WRITETHROUGH 0x008  // Write-through caching
#define PAGE_NOCACHE    0x010  // Disable caching
#define PAGE_ACCESSED   0x020  // Page has been accessed
#define PAGE_DIRTY      0x040  // Page has been written to (only in PTE)
#define PAGE_SIZE_4MB   0x080  // 4MB page (only in PDE with PSE)
#define PAGE_GLOBAL     0x100  // Global page (not flushed from TLB)

/* Page directory and page table sizes */
#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE     1024

/* Page directory entry */
typedef uint32_t PageDirectoryEntry;

/* Page table entry */
typedef uint32_t PageTableEntry;

/* Page directory structure */
typedef struct {
    PageDirectoryEntry entries[PAGE_DIRECTORY_SIZE];
} __attribute__((aligned(4096))) PageDirectory;

/* Page table structure */
typedef struct {
    PageTableEntry entries[PAGE_TABLE_SIZE];
} __attribute__((aligned(4096))) PageTable;

/*
 * PagingInitialize - Initialize paging with identity mapping
 *
 * Sets up a page directory and page tables for the kernel with identity
 * mapping (virtual address = physical address). Enables paging.
 */
void PagingInitialize(void);

/*
 * PagingMapPage - Map a virtual page to a physical page
 *
 * @virtualAddr: Virtual address to map (page-aligned)
 * @physicalAddr: Physical address to map to (page-aligned)
 * @flags: Page flags (PAGE_PRESENT | PAGE_WRITE, etc.)
 * @return: true on success, false on failure
 */
bool PagingMapPage(uintptr_t virtualAddr, uintptr_t physicalAddr, uint32_t flags);

/*
 * PagingUnmapPage - Unmap a virtual page
 *
 * @virtualAddr: Virtual address to unmap (page-aligned)
 */
void PagingUnmapPage(uintptr_t virtualAddr);

/*
 * PagingGetPhysicalAddress - Get physical address for virtual address
 *
 * @virtualAddr: Virtual address to translate
 * @return: Physical address, or 0 if not mapped
 */
uintptr_t PagingGetPhysicalAddress(uintptr_t virtualAddr);

/*
 * PagingInvalidatePage - Invalidate TLB entry for a page
 *
 * @virtualAddr: Virtual address of page to invalidate
 */
void PagingInvalidatePage(uintptr_t virtualAddr);

/*
 * PagingGetCurrentDirectory - Get current page directory
 *
 * @return: Pointer to current page directory
 */
PageDirectory* PagingGetCurrentDirectory(void);

/*
 * PagingSwitchDirectory - Switch to a different page directory
 *
 * @pageDir: Physical address of page directory to switch to
 */
void PagingSwitchDirectory(uintptr_t pageDir);

#endif /* PAGING_H */
