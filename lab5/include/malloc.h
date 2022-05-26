#ifndef MALLOC_H
#define MALLOC_H

#include "compiler.h"
#include "list.h"

#define KSTACK_SIZE             0x10000000

// 0x1000 0000 -> 0x2000 0000
// #define PAGE_BASE               0x10000000
// #define PAGE_SIZE               0x1000
// #define PAGE_COUNT              0x10000     // 65536 page
#define PAGE_BASE               0x0
#define PAGE_SIZE               0x1000
#define PAGE_COUNT              0x3C000     // 245760 page
#define MAX_ORDER               10             // 4k * 2^0 ~ 4k * 2^10

// 32 * 2^0 ~ 32 * 2^4
#define CHUNK_SIZE              0x20        // size of list_head_t
#define MAX_CHUNK_ORDER         4

/**
 * @param frame_order
 * Only used by leading buddy of a contiguous buddy chain.
 * number of contiguous frames of size 2^frame_order.
 * @param in_use
 * 0 not in use
 * 1 in use
 * @param chunk_order
 * -1 not used for chunk
 * >0 chunk order
 */
typedef struct frame
{
    list_head_t listhead;
    int frame_index;
    int frame_order;
    int in_use;
    int chunk_order;
    int chunk_count;
} frame_t;

void kmalloc_init();
void *alloc_pages(unsigned long long size);
void free_pages(void *ptr);
void *alloc_chunk(unsigned long long size);
void free_chunk(void *ptr);
void *kmalloc(uint64_t size);
void kfree(void *ptr);

void memory_reserve(unsigned long long start, unsigned long long end);

void show_free_list(int free_list_order);

void* malloc(unsigned int size);
void free(void* ptr);

#endif