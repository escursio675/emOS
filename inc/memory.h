/**
 * memory.h
 *
 * Kernel heap: kmalloc()/kfree() over one fixed region.
 * First-fit allocator, single address space, no paging, no MPU.
 */
#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

/* Heap size in bytes. Must be a multiple of 8. */
#ifndef KHEAP_SIZE
#define KHEAP_SIZE 4096U
#endif

/* Merge adjacent free blocks on kfree().
 * Your scope doc lists coalescing under future scope, so it is OFF by
 * default. Set to 1 (e.g. -DKMALLOC_COALESCE=1) to enable it. */
#ifndef KMALLOC_COALESCE
#define KMALLOC_COALESCE 0
#endif

typedef struct {
    size_t total_bytes;   /* heap size */
    size_t used_bytes;    /* bytes in allocated blocks, headers included */
    size_t free_bytes;    /* bytes in free blocks, headers included */
    size_t largest_free;  /* largest request kmalloc() could satisfy now */
    size_t block_count;   /* allocated + free blocks */
} heap_stats_t;           /* used_bytes + free_bytes == total_bytes */

/* Set up the heap as one big free block. Call once from kernel_init(). */
void kmalloc_init(void);

/* Returns an 8-byte aligned block of at least `size` bytes, or NULL if
 * size is 0 or no free block is large enough (out of memory). */
void *kmalloc(size_t size);

/* Releases a block from kmalloc(). NULL, wild pointers and double frees
 * are ignored. */
void kfree(void *ptr);

/* Snapshot of the heap for the shell's `mem` command. */
void kmalloc_stats(heap_stats_t *out);

#endif /* MEMORY_H */