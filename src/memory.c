/**
 * memory.c
 *
 * First-fit kernel heap.
 *
 * Layout: the heap is one static array. It is carved into blocks that
 * always tile it exactly, each block being [header][payload]. Blocks are
 * linked in address order, so the "free list" is just a walk of this list.
 *
 *   heap_area
 *   | hdr | payload (used) | hdr | payload (free) | hdr | payload (used) |
 */
#include <stdint.h>
#include <stddef.h>
#include "../inc/memory.h"

#define KMALLOC_ALIGN  8U
#define BLOCK_MAGIC    0xA110C8EDU

/* may_alias: the heap is a byte array that we also view as headers; this
 * stops GCC (-Os enables strict aliasing) from reordering those accesses. */
typedef struct block_header block_header_t;

struct __attribute__((may_alias)) block_header {
    size_t          size;     /* payload bytes, multiple of KMALLOC_ALIGN */
    block_header_t *next;     /* next block in address order, NULL if last */
    uint32_t        is_free;  /* 1 = free, 0 = allocated */
    uint32_t        magic;    /* BLOCK_MAGIC, lets kfree() reject bad pointers */
};

#define HDR_SIZE  (sizeof(struct block_header))

/* Keeping every block start 8-byte aligned needs these to hold. */
typedef char check_hdr_aligned[(sizeof(struct block_header) % KMALLOC_ALIGN == 0U) ? 1 : -1];
typedef char check_heap_aligned[((KHEAP_SIZE % KMALLOC_ALIGN) == 0U) ? 1 : -1];

static uint8_t heap_area[KHEAP_SIZE] __attribute__((aligned(KMALLOC_ALIGN)));
static block_header_t *heap_head = NULL;

static size_t align_up(size_t n)
{
    return (n + (KMALLOC_ALIGN - 1U)) & ~(size_t)(KMALLOC_ALIGN - 1U);
}

void kmalloc_init(void)
{
    heap_head = (block_header_t *)heap_area;
    heap_head->size    = KHEAP_SIZE - HDR_SIZE;
    heap_head->next    = NULL;
    heap_head->is_free = 1U;
    heap_head->magic   = BLOCK_MAGIC;
}

/* Cut `b` down to `size` payload bytes and turn the tail into a new free
 * block, but only if the tail can hold a header plus a minimal payload. */
static void split_block(block_header_t *b, size_t size)
{
    block_header_t *tail;

    if (b->size < size + HDR_SIZE + KMALLOC_ALIGN) {
        return;   /* too small to split: caller gets the whole block */
    }

    tail = (block_header_t *)((uint8_t *)(b + 1) + size);
    tail->size    = b->size - size - HDR_SIZE;
    tail->next    = b->next;
    tail->is_free = 1U;
    tail->magic   = BLOCK_MAGIC;

    b->size = size;
    b->next = tail;
}

void *kmalloc(size_t size)
{
    block_header_t *b;

    if (heap_head == NULL || size == 0U || size > KHEAP_SIZE) {
        return NULL;
    }
    size = align_up(size);

    /* First fit: take the first free block that is big enough. */
    for (b = heap_head; b != NULL; b = b->next) {
        if (b->is_free && b->size >= size) {
            split_block(b, size);
            b->is_free = 0U;
            return (void *)(b + 1);
        }
    }

    return NULL;   /* out of memory */
}

#if KMALLOC_COALESCE
/* Merge every run of adjacent free blocks into one. */
static void coalesce(void)
{
    block_header_t *b = heap_head;

    while (b != NULL && b->next != NULL) {
        if (b->is_free && b->next->is_free) {
            b->next->magic = 0U;   /* absorbed header is no longer a block */
            b->size += HDR_SIZE + b->next->size;
            b->next  = b->next->next;
        } else {
            b = b->next;
        }
    }
}
#endif

void kfree(void *ptr)
{
    block_header_t *b;
    uintptr_t p  = (uintptr_t)ptr;
    uintptr_t lo = (uintptr_t)heap_area + HDR_SIZE;
    uintptr_t hi = (uintptr_t)heap_area + KHEAP_SIZE;

    if (ptr == NULL || heap_head == NULL) {
        return;
    }
    if (p < lo || p >= hi || (p & (KMALLOC_ALIGN - 1U)) != 0U) {
        return;   /* not a pointer into the heap */
    }

    b = (block_header_t *)ptr - 1;
    if (b->magic != BLOCK_MAGIC || b->is_free) {
        return;   /* not a block start, or already freed */
    }

    b->is_free = 1U;

#if KMALLOC_COALESCE
    coalesce();
#endif
}

void kmalloc_stats(heap_stats_t *out)
{
    block_header_t *b;

    out->total_bytes  = KHEAP_SIZE;
    out->used_bytes   = 0U;
    out->free_bytes   = 0U;
    out->largest_free = 0U;
    out->block_count  = 0U;

    for (b = heap_head; b != NULL; b = b->next) {
        size_t span = HDR_SIZE + b->size;

        out->block_count++;
        if (b->is_free) {
            out->free_bytes += span;
            if (b->size > out->largest_free) {
                out->largest_free = b->size;
            }
        } else {
            out->used_bytes += span;
        }
    }
}