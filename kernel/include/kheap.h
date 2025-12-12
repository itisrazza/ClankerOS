/* kheap.h - Kernel Heap Allocator */
#ifndef KHEAP_H
#define KHEAP_H

#include <stddef.h>
#include <stdint.h>

/*
 * KHeapInitialize - Initialize the kernel heap
 *
 * Sets up the kernel heap allocator. Must be called after paging is enabled.
 */
void KHeapInitialize(void);

/*
 * KAllocateMemory - Allocate memory from kernel heap
 *
 * @size: Number of bytes to allocate
 * @return: Pointer to allocated memory, or NULL if out of memory
 */
void* KAllocateMemory(size_t size);

/*
 * KFreeMemory - Free memory allocated from kernel heap
 *
 * @ptr: Pointer to memory to free (must be from KAllocateMemory)
 */
void KFreeMemory(void* ptr);

/*
 * KReallocateMemory - Reallocate memory
 *
 * @ptr: Pointer to existing allocation (or NULL for new allocation)
 * @size: New size in bytes
 * @return: Pointer to reallocated memory, or NULL if out of memory
 */
void* KReallocateMemory(void* ptr, size_t size);

/*
 * KHeapGetStats - Get heap statistics
 *
 * @totalSize: Output parameter for total heap size
 * @usedSize: Output parameter for used heap size
 * @freeSize: Output parameter for free heap size
 */
void KHeapGetStats(size_t* totalSize, size_t* usedSize, size_t* freeSize);

#endif /* KHEAP_H */
