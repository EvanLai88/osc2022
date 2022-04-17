#ifndef MALLOC_H
#define MALLOC_H

#include "list.h"

// 0x1000 0000 -> 0x2000 0000
#define PAGE_BASE               0x10000000
#define PAGE_COUNT              0x10000     // 65536 page
#define MAX_ORDER               10          // 4k * 2^0 ~ 4k * 2^10
#define MAX_CHUNK_ORDER         4           // 32, 64, 128, 256, 512

/**
 * @param frame_order
 * Only used by leading buddy of a contiguous buddy chain.
 * number of contiguous frames of size 2^frame_order
 * @param in_use
 * 0 not in use
 * 1 in use
 */
typedef struct frame
{
    list_head_t listhead;
    int frame_index;
    int frame_order;
    int in_use;
} frame_t;

typedef struct chunk
{
    list_head_t listhead;
    int slot_index;
} chunk_t;

void kmalloc_init();
void *alloc_pages(unsigned long long size);
void free_pages(void *ptr);
void *alloc_chunk(unsigned long long size);
void free_chunk(void *ptr);


void show_free_list(int free_list_order);

void* malloc(unsigned int size);
void free(void* ptr);

#endif