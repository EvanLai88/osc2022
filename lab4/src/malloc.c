#include "malloc.h"
#include "uart.h"

extern char _heap_start;
static char* top = &_heap_start;

static frame_t *frame_array;
static list_head_t free_list[MAX_ORDER + 1];           // 4k * 2^0 ~ 4k * 2^10
static list_head_t chunk_list[MAX_CHUNK_ORDER + 1];    // 32, 64, 128, 256, 512

void kmalloc_init()
{
    /* free list init */
    for (int i = 0; i <= MAX_ORDER; i++)
    {
        INIT_LIST_HEAD ( &free_list[i] );
    }
    
    /* frame array init */
    frame_array = malloc(PAGE_COUNT * sizeof(frame_t));
    int i = 0;
    for (i = 0; i < PAGE_COUNT; i++)
    {
        INIT_LIST_HEAD ( &frame_array[i].listhead );
        frame_array[i].frame_index = i;
        if ( i % (1 << MAX_ORDER) == 0) {
            frame_array[i].frame_order = MAX_ORDER;
            list_add_tail ( &frame_array[i].listhead, &free_list[MAX_ORDER] );
        }
        else {
            frame_array[i].frame_order = 0;
            frame_array[i].in_use = 0;
        }
    }

    /* chunk list init */
    for (int i = 0; i <= MAX_CHUNK_ORDER; i++)
    {
        INIT_LIST_HEAD ( &chunk_list[i] );
    }
}

void *alloc_pages(unsigned long long size)
{
    /* check allocate order */
    if (size > (0x1000<<MAX_ORDER)) {
        uart_async_puts("\n\t!!!!! Exceed Max Order !!!!!\n");
        return (void *)3;
    }
    int alloc_order = 0;
    for (int i = 0; i<=MAX_ORDER; i++)
    {
        if (size <= ( 0x1000 << i)) {
            alloc_order = i;
            break;
        }
    }

    /* find suitable free buddies to allocate such size */
    int curr_order = alloc_order;
    for (int i = alloc_order; i<=MAX_ORDER; i++) {
        if ( ! list_empty( &free_list[i] )) {
            curr_order = i;
            break;
        }
    }

    /* check out-of-memory */
    if (list_empty( &free_list[curr_order] )) {
        uart_async_puts("\n\t!!!!! Out of memory !!!!!\n");
        return (void *) 0x1;
    }

    /* split buddies */
    frame_t *curr = (frame_t *) free_list[curr_order].next;
    list_del_entry( &curr->listhead );
    uart_async_puts("Release redundant memory:\n");
    for (int i = curr_order; i > alloc_order; i--) {
        curr->frame_order--;
        frame_t *frame_buddy = &frame_array[ curr->frame_index ^ (1 << curr->frame_order)];
        frame_buddy->frame_order = curr->frame_order;
        list_add(&frame_buddy->listhead, &free_list[frame_buddy->frame_order]);

        uart_async_puts("\tindex = ");
        uart_async_int((unsigned long long)frame_buddy->frame_index);
        uart_async_puts(", order = ");
        uart_async_int((unsigned long long)frame_buddy->frame_order);
        uart_async_puts("\n");
    }

    /* allocate */
    list_del_entry( &curr->listhead );
    curr->frame_order = alloc_order;
    curr->in_use = 1;

    return (void *) PAGE_BASE + (curr->frame_index << 12);
}

void free_pages(void *ptr)
{
    frame_t *curr = &frame_array[(unsigned long long)(ptr-PAGE_BASE)<<12];
    uart_async_puts("\nCoalesce memory:\n");
    for (int i = curr->frame_order; i<MAX_ORDER; i++) {
        frame_t *frame_buddy = &frame_array[ curr->frame_index ^ (1 << curr->frame_order)];

        /* cannot coalesce when buddy is in use */
        if (frame_buddy->in_use == 1) {
            break;
        }

        /* remove buddy from free list */
        list_del_entry( &frame_buddy->listhead);

        /* find leading head after merge */
        if (curr->frame_index > frame_buddy->frame_index) {
            uart_async_puts("\tindex = ");
            uart_async_int((unsigned long long)frame_buddy->frame_index);
            uart_async_puts(", order = ");
            uart_async_int((unsigned long long)frame_buddy->frame_order);
            uart_async_puts("\n");

            uart_async_puts("\tindex = ");
            uart_async_int((unsigned long long)curr->frame_index);
            uart_async_puts(", order = ");
            uart_async_int((unsigned long long)curr->frame_order);
            uart_async_puts("\n");
            
            /* reset curr */
            curr->frame_order = 0;
            curr->in_use = 0;
            
            curr = frame_buddy;
        }
        else {
            uart_async_puts("\tindex = ");
            uart_async_int((unsigned long long)curr->frame_index);
            uart_async_puts(", order = ");
            uart_async_int((unsigned long long)curr->frame_order);
            uart_async_puts("\n");

            uart_async_puts("\tindex = ");
            uart_async_int((unsigned long long)frame_buddy->frame_index);
            uart_async_puts(", order = ");
            uart_async_int((unsigned long long)frame_buddy->frame_order);
            uart_async_puts("\n");

            /* reset buddy */
                /**
                 * Compiler optimization creates unaligned-write exception on Raspi board, but not on qemu.
                 * Use gcc flag -mstrict-align for aarch64 to restrict unaligned-access.
                 */
            frame_buddy->frame_order = 0;
            frame_buddy->in_use = 0;
        }

        /* update curr's order */
        curr->frame_order++;

        uart_async_puts("\t\t-> index = ");
        uart_async_int((unsigned long long)curr->frame_index);
        uart_async_puts(", order = ");
        uart_async_int((unsigned long long)curr->frame_order);
        uart_async_puts("\n");
    }

    /* add coalesced memory to free list */
    list_add(&curr->listhead, &free_list[curr->frame_order]);
}

void *alloc_chunk(unsigned long long size)
{
    return (void *)0;
}

static unsigned long align_up(unsigned long n, unsigned long align)
{
    return (n + align - 1) & (~(align - 1));
}

void show_free_list(int free_list_order)
{
    list_head_t *curr;
    uart_async_puts("Free list of order ");
    uart_async_int(free_list_order);
    uart_async_puts("\n");
    list_for_each(curr, &free_list[free_list_order])
    {
        uart_async_puts("\tindex = ");
        uart_async_int( ((frame_t *)curr)->frame_index );
        uart_async_puts("\torder = ");
        uart_async_int( ((frame_t *)curr)->frame_order );
        uart_async_puts("\n");
    }
}

void* malloc(unsigned int size) {
    char* r = top+0x10;
    top += align_up(size+0x10, 0x10);
    return r;
}

void free(void* ptr) {
    
}