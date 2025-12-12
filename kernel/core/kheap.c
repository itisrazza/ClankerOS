/* kheap.c - Kernel Heap Allocator Implementation */

#include "kheap.h"
#include "paging.h"
#include "pmm.h"
#include "early_console.h"
#include "clc/printf.h"
#include "econ_writer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Heap configuration */
#define HEAP_START      0x00500000  // Start heap at 5MB
#define HEAP_INITIAL    0x00100000  // Initial heap size: 1MB
#define HEAP_MAX        0x10000000  // Maximum heap size: 256MB

/* Block header structure */
typedef struct BlockHeader {
    size_t size;                   // Size of block (excluding header)
    bool free;                     // Is block free?
    struct BlockHeader* next;      // Next block in list
} BlockHeader;

/* Heap state */
static uintptr_t heapStart = HEAP_START;
static uintptr_t heapEnd = HEAP_START;
static uintptr_t heapMax = HEAP_MAX;
static BlockHeader* firstBlock = NULL;

/* Statistics */
static size_t totalSize = 0;
static size_t usedSize = 0;
static size_t freeSize = 0;

/* Alignment */
#define ALIGN_UP(n, align) (((n) + (align) - 1) & ~((align) - 1))
#define BLOCK_ALIGN 16

/*
 * heapExpand - Expand the heap by allocating more pages
 */
static bool heapExpand(size_t increment)
{
    // Align increment to page boundary
    increment = ALIGN_UP(increment, PAGE_SIZE);

    // Check if we would exceed maximum heap size
    if (heapEnd + increment > heapMax) {
        return false;
    }

    // Allocate and map pages
    for (uintptr_t addr = heapEnd; addr < heapEnd + increment; addr += PAGE_SIZE) {
        uintptr_t physPage = PmmAllocPage();
        if (physPage == 0) {
            return false;  // Out of physical memory
        }

        if (!PagingMapPage(addr, physPage, PAGE_PRESENT | PAGE_WRITE)) {
            PmmFreePage(physPage);
            return false;
        }
    }

    // Create new free block at end of heap
    BlockHeader* newBlock = (BlockHeader*)heapEnd;
    newBlock->size = increment - sizeof(BlockHeader);
    newBlock->free = true;
    newBlock->next = NULL;

    // Add to block list
    if (firstBlock == NULL) {
        firstBlock = newBlock;
    } else {
        // Find last block
        BlockHeader* current = firstBlock;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newBlock;
    }

    heapEnd += increment;
    totalSize += increment - sizeof(BlockHeader);
    freeSize += increment - sizeof(BlockHeader);

    return true;
}

/*
 * heapMergeBlocks - Merge adjacent free blocks
 */
static void heapMergeBlocks(void)
{
    BlockHeader* current = firstBlock;

    while (current != NULL && current->next != NULL) {
        if (current->free && current->next->free) {
            // Check if blocks are adjacent
            uintptr_t currentEnd = (uintptr_t)current + sizeof(BlockHeader) + current->size;
            uintptr_t nextStart = (uintptr_t)current->next;

            if (currentEnd == nextStart) {
                // Merge blocks
                current->size += sizeof(BlockHeader) + current->next->size;
                current->next = current->next->next;
                continue;  // Check again with same block
            }
        }
        current = current->next;
    }
}

/*
 * KHeapInitialize - Initialize the kernel heap
 */
void KHeapInitialize(void)
{
    ClcWriter* serial = EConGetWriter();

    ClcPrintfWriter(serial, "\nInitializing kernel heap...\n");
    ClcPrintfWriter(serial, "  Heap range: %p - %p\n", (void*)heapStart, (void*)heapMax);

    // Expand heap with initial size
    if (!heapExpand(HEAP_INITIAL)) {
        ClcPrintfWriter(serial, "ERROR: Failed to initialize heap\n");
        return;
    }

    ClcPrintfWriter(serial, "  Initial size: %u KB\n", HEAP_INITIAL / 1024);
    ClcPrintfWriter(serial, "Kernel heap initialized\n");
}

/*
 * KAllocateMemory - Allocate memory from kernel heap
 */
void* KAllocateMemory(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    // Align size
    size = ALIGN_UP(size, BLOCK_ALIGN);

    // Find first free block that fits (first-fit algorithm)
    BlockHeader* current = firstBlock;
    while (current != NULL) {
        if (current->free && current->size >= size) {
            // Found suitable block

            // Split block if remainder is large enough
            if (current->size >= size + sizeof(BlockHeader) + BLOCK_ALIGN) {
                // Create new block for remainder
                BlockHeader* newBlock = (BlockHeader*)((uintptr_t)current + sizeof(BlockHeader) + size);
                newBlock->size = current->size - size - sizeof(BlockHeader);
                newBlock->free = true;
                newBlock->next = current->next;

                current->size = size;
                current->next = newBlock;

                freeSize -= size + sizeof(BlockHeader);
            } else {
                // Use entire block
                freeSize -= current->size;
            }

            current->free = false;
            usedSize += current->size;

            return (void*)((uintptr_t)current + sizeof(BlockHeader));
        }
        current = current->next;
    }

    // No suitable block found, try expanding heap
    size_t expandSize = ALIGN_UP(size + sizeof(BlockHeader), PAGE_SIZE);
    if (expandSize < PAGE_SIZE * 4) {
        expandSize = PAGE_SIZE * 4;  // Expand by at least 4 pages
    }

    if (!heapExpand(expandSize)) {
        return NULL;  // Out of memory
    }

    // Try allocation again (should succeed now)
    return KAllocateMemory(size);
}

/*
 * KFreeMemory - Free memory allocated from kernel heap
 */
void KFreeMemory(void* ptr)
{
    if (ptr == NULL) {
        return;
    }

    // Get block header
    BlockHeader* block = (BlockHeader*)((uintptr_t)ptr - sizeof(BlockHeader));

    // Mark as free
    block->free = true;
    usedSize -= block->size;
    freeSize += block->size;

    // Merge adjacent free blocks
    heapMergeBlocks();
}

/*
 * KReallocateMemory - Reallocate memory
 */
void* KReallocateMemory(void* ptr, size_t size)
{
    // If ptr is NULL, behave like malloc
    if (ptr == NULL) {
        return KAllocateMemory(size);
    }

    // If size is 0, behave like free
    if (size == 0) {
        KFreeMemory(ptr);
        return NULL;
    }

    // Get current block
    BlockHeader* block = (BlockHeader*)((uintptr_t)ptr - sizeof(BlockHeader));

    // If new size fits in current block, keep it
    if (block->size >= size) {
        return ptr;
    }

    // Allocate new block
    void* newPtr = KAllocateMemory(size);
    if (newPtr == NULL) {
        return NULL;
    }

    // Copy data
    size_t copySize = block->size < size ? block->size : size;
    uint8_t* src = (uint8_t*)ptr;
    uint8_t* dst = (uint8_t*)newPtr;
    for (size_t i = 0; i < copySize; i++) {
        dst[i] = src[i];
    }

    // Free old block
    KFreeMemory(ptr);

    return newPtr;
}

/*
 * KHeapGetStats - Get heap statistics
 */
void KHeapGetStats(size_t* totalSizeOut, size_t* usedSizeOut, size_t* freeSizeOut)
{
    if (totalSizeOut) *totalSizeOut = totalSize;
    if (usedSizeOut) *usedSizeOut = usedSize;
    if (freeSizeOut) *freeSizeOut = freeSize;
}
