#include "malloc.h"
#include "fdt.h"
#include "mbox.h"
#include "exception.h"
#include "uart.h"

extern char _heap_start;
extern char _end;
static char* top = &_heap_start;

uint32_t arm_memory_base;
uint32_t arm_memory_size;

static frame_t *frame_array;
static list_head_t free_list[MAX_ORDER + 1];           // 4k * 2^0 ~ 4k * 2^10
static list_head_t chunk_list[MAX_CHUNK_ORDER + 1];    // 32, 64, 128, 256, 512

int DEBUG = 0;

static unsigned long long align_up(unsigned long long n, unsigned long long align)
{
    return (n + align - 1) & (~(align - 1));
}

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
    for (i = PAGE_COUNT-1; i>=0; i--)
    {
        INIT_LIST_HEAD ( &frame_array[i].listhead );
        frame_array[i].frame_index = i;
        if ( i % (1 << MAX_ORDER) == 0) {
            frame_array[i].frame_order = MAX_ORDER;
            list_add ( &frame_array[i].listhead, &free_list[MAX_ORDER] );
        }
        else {
            frame_array[i].frame_order = 0;
        }
        frame_array[i].in_use = 0;
        frame_array[i].chunk_order = -1;
    }

    /* chunk list init */
    for (int i = 0; i <= MAX_CHUNK_ORDER; i++)
    {
        INIT_LIST_HEAD ( &chunk_list[i] );
    }

    get_arm_memory_info(&arm_memory_base, &arm_memory_size);

    uart_puts("\n");
    uart_puts("\033[2J\033[H");
    dtb_reserve_mem();
    uart_getc(ECHO_OFF);
    uart_puts("kernel img:\n");
    memory_reserve(0x80000, (uint64_t)&_end);
    uart_getc(ECHO_OFF);
    uart_puts("startup malloc:\n");
    memory_reserve((uint64_t)&_heap_start, (uint64_t)top);
    uart_getc(ECHO_OFF);
    uart_puts("CPIO:\n");
    memory_reserve((uint64_t)CPIO_BASE, (uint64_t)CPIO_END);
    uart_getc(ECHO_OFF);
    uart_puts("Kernel Stack:\n");
    memory_reserve(arm_memory_size - KSTACK_SIZE, arm_memory_size);
}

void *alloc_pages(unsigned long long size)
{
    /* check allocate order */
    if (size > (PAGE_SIZE<<MAX_ORDER)) {
        uart_puts("\n\t!!!!! Exceed Max Order !!!!!\n");
        return (void *) ((uint64_t)arm_memory_size + 3);
    }
    int alloc_order = 0;
    for (int i = 0; i<=MAX_ORDER; i++)
    {
        if (size <= ( PAGE_SIZE << i)) {
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
        uart_puts("\n\t!!!!! Out of memory !!!!!\n");
        return (void *) 0x1;
    }

    /* split buddies */
    frame_t *curr = (frame_t *) free_list[curr_order].next;
    list_del_entry( &curr->listhead );

    if (DEBUG >= 1) {
        uart_puts("\tRelease redundant memory:\n");
        uart_puts("\t\tRelease start\n");
    }
    
    for (int i = curr_order; i > alloc_order; i--) {
        curr->frame_order--;
        frame_t *frame_buddy = &frame_array[ curr->frame_index ^ (1 << curr->frame_order)];
        frame_buddy->frame_order = curr->frame_order;
        list_add(&frame_buddy->listhead, &free_list[frame_buddy->frame_order]);

        if (DEBUG >= 1) {
            uart_puts("\t\tindex = ");
            uart_int((unsigned long long)frame_buddy->frame_index);
            uart_puts(",\torder = ");
            uart_int((unsigned long long)frame_buddy->frame_order);
            uart_puts("\n");
        }
    }
    if (DEBUG >= 1) {
        uart_puts("\t\tRelease end\n\n");
    }

    /* allocate */
    list_del_entry( &curr->listhead );
    curr->frame_order = alloc_order;
    curr->in_use = 1;

    if (DEBUG >= 2) {
        uart_puts("frame\taddress: 0x");
        uart_hex((unsigned long long)&curr->listhead);
        uart_puts("\tindex: ");
        uart_int(curr->frame_index);
        uart_puts("\torder: ");
        uart_int(curr->frame_order);
        uart_puts("\n");
    }

    return (void *) PAGE_BASE + (curr->frame_index << 12);
}

void free_pages(void *ptr)
{
    // uart_hex((unsigned long long)&frame_array[(unsigned long long)(ptr-PAGE_BASE)>>12]);
    frame_t *curr = &frame_array[(unsigned long long)(ptr-PAGE_BASE)>>12];

    if (DEBUG >= 2) {
        uart_puts("frame\taddress: 0x");
        uart_hex((unsigned long long)&curr->listhead);
        uart_puts("\tindex: ");
        uart_int(curr->frame_index);
        uart_puts("\torder: ");
        uart_int(curr->frame_order);
        uart_puts("\n");
    }

    if (DEBUG >= 1) {
        uart_puts("\n\tCoalesce memory:\n");
        uart_puts("\tCoalesce start\n");
    }

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
            if (DEBUG >= 1) {
                uart_puts("\t\tindex = ");
                uart_int((unsigned long long)frame_buddy->frame_index);
                uart_puts(",\torder = ");
                uart_int((unsigned long long)frame_buddy->frame_order);
                uart_puts("\n\t\tindex = ");
                uart_int((unsigned long long)curr->frame_index);
                uart_puts(",\torder = ");
                uart_int((unsigned long long)curr->frame_order);
                uart_puts("\n");
            }
            
            /* reset curr */
            curr->frame_order = 0;
            curr->in_use = 0;
            
            curr = frame_buddy;
        }
        else {
            if (DEBUG >= 1) {
                uart_puts("\t\tindex = ");
                uart_int((unsigned long long)curr->frame_index);
                uart_puts(",\torder = ");
                uart_int((unsigned long long)curr->frame_order);
                uart_puts("\n\t\tindex = ");
                uart_int((unsigned long long)frame_buddy->frame_index);
                uart_puts(",\torder = ");
                uart_int((unsigned long long)frame_buddy->frame_order);
                uart_puts("\n");
            }

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

        if (DEBUG >= 1) {
            uart_puts("\t\t\t-> index = ");
            uart_int((unsigned long long)curr->frame_index);
            uart_puts(",\torder = ");
            uart_int((unsigned long long)curr->frame_order);
            uart_puts("\n");
        }
    }
    if (DEBUG >= 1) {
        uart_puts("\tCoalesce end\n\n");
    }

    /* add coalesced memory to free list */
    list_add(&curr->listhead, &free_list[curr->frame_order]);
}

void *alloc_chunk(unsigned long long size)
{
    /* determine allocate order */
    if (size > (CHUNK_SIZE<<MAX_CHUNK_ORDER)) {
        uart_puts("\n\t!!!!! Exceed Max chunk size of 512 !!!!!\n");
        return (void *) ((uint64_t)arm_memory_size + 4);
    }
    int alloc_order = 0;
    for (int i = 0; i<=(MAX_CHUNK_ORDER); i++)
    {
        if (size <= ( CHUNK_SIZE << i )) {
            alloc_order = i;
            break;
        }
    }

    /* check chunk availability */
    if (list_empty(&chunk_list[alloc_order]))
    {
        void *page = alloc_pages(PAGE_SIZE);
        frame_t *chunk_page = &frame_array[(unsigned long long)(page-PAGE_BASE)>>12];
        chunk_page->chunk_order = alloc_order;
        chunk_page->chunk_count = PAGE_SIZE / (CHUNK_SIZE << alloc_order);

        int CS = (CHUNK_SIZE << alloc_order);
        for (int i=PAGE_SIZE-CS; i>=0; i -= CS)
        {
            list_add ((list_head_t *)(page+i), &chunk_list[alloc_order]);
        }
    }

    /* allocate */
    void *ptr = chunk_list[alloc_order].next;
    frame_t *chunk_page = &frame_array[(unsigned long long)(ptr-PAGE_BASE)>>12];
    if (likely(chunk_page->chunk_count > 0)) {
        chunk_page->chunk_count--;
    }
    else {
        uart_puts("\n\t!!!!! Chunk_Count Error !!!!!\n");
    }
    list_del_entry((list_head_t *)ptr);

    return ptr;
}

void free_chunk(void *ptr)
{
    /* find chunk page */
    frame_t *curr = &frame_array[(unsigned long long)(ptr-PAGE_BASE)>>12];
    if (DEBUG >= 1) {
        uart_puts("\tfree chunk: 0x");
        uart_hex((unsigned long)ptr);
        uart_puts("\t");
        // uart_hex((unsigned long)curr);
        // uart_puts("\t");
        uart_int(curr->chunk_order);
        uart_puts("\t");
        uart_int(curr->chunk_count);
        uart_puts("\n");
    }

    /* free chunk */
    curr->chunk_count++;
    list_add ((list_head_t *)ptr, &chunk_list[curr->chunk_order]);

    /* free chunk page */
    // if(unlikely(curr->chunk_count == PAGE_SIZE / (CHUNK_SIZE << curr->chunk_order)))
    // {

    // }
}

void *kmalloc(uint64_t size)
{
    disable_interrupt();
    if (size <= (CHUNK_SIZE << MAX_CHUNK_ORDER)) {
        void *ptr = alloc_chunk(size);
        enable_interrupt();
        return ptr;
    }
    
    if (size <= (PAGE_SIZE << MAX_ORDER))
    {
        void *ptr = alloc_pages(size);
        enable_interrupt();
        return ptr;
    }

    uart_puts("\n\t!!!!! kmalloc error: exceed max order!!!!!!\n");
    enable_interrupt();
    return (void *) ((uint64_t)arm_memory_size + 1);
}

void kfree(void *ptr)
{
    frame_t *curr = &frame_array[(unsigned long long)(ptr-PAGE_BASE)>>12];
    disable_interrupt();
    if(curr->chunk_order == -1)
    {
        free_pages(ptr);
        enable_interrupt();
        return;
    }

    free_chunk(ptr);
    enable_interrupt();
}

/**
 * @brief Reserve Memory from `start` to `end` not includeing `end`.
 * 
 * @param start starting address of reserve memory area.
 * @param end ending address (not included) of reserve memory area.
 */
void memory_reserve(unsigned long long start, unsigned long long end)
{
    start = start - (start % PAGE_SIZE);
    end = align_up(end, PAGE_SIZE);
    uart_puts("\tstart: \t0x");
    uart_hex(start);
    uart_puts("\n");
    uart_puts("\tend: \t0x");
    uart_hex(end);
    uart_puts("\n");

    /**
     * Divide and conquer
     * Split every contiguous pages, that are not fully contained by the reserve area, in half (buddy system)
     * until thay are fully contained by the reserve area.
     */
    uart_puts("\tReserve :\n");
    for (int i=MAX_ORDER; i>=0; i--)
    {
        list_head_t *curr;
        list_for_each(curr, &free_list[i])
        {
            unsigned long long page_start = PAGE_BASE + (((frame_t *)curr)->frame_index << 12);
            unsigned long long page_end   = page_start + (PAGE_SIZE << i);

            if ( end <= page_start || page_end <= start )       // no intersection
            {
                // do nothing
                continue;
            }
            else if ( start <= page_start && page_end <= end )  // reserve fully contains pages.
            {
                ((frame_t *)curr)->in_use = 1;
                list_del_entry(curr);
                uart_puts("\t\taddress = 0x");
                uart_hex(page_start);
                uart_puts("\tindex = ");
                uart_int((unsigned long long)((frame_t *)curr)->frame_index);
                uart_puts(",\torder = ");
                uart_int((unsigned long long)((frame_t *)curr)->frame_order);
                uart_puts("\n");
                // for (int k = 0; k<=MAX_ORDER; k++)
                // {
                //     show_free_list(k);
                // }
            }
            else
            {
                // 1. pages fully contains reserve (but not equal)
                // 2. interlace    page_start < start < page_end < end
                // 3. interlace    start < page_start < end < page_end
                if (i == 0) {
                    uart_puts("\t!!!!! Reserve memory error. !!!!!\n");
                }
                frame_t *buddy = &frame_array[ ((frame_t *)curr)->frame_index ^ (1 << (i-1))];

                // uart_puts("Split buddy: \n");
                // uart_puts("\tindex = ");
                // uart_int((unsigned long long)((frame_t *)curr)->frame_index);
                // uart_puts(",\torder = ");
                // uart_int((unsigned long long)((frame_t *)curr)->frame_order);
                // uart_puts("\n");

                list_head_t *temp = curr->prev;
                list_del_entry(&(((frame_t *)curr)->listhead));

                ((frame_t *)curr)->frame_order = i-1;
                buddy->frame_order = i-1;

                if (((frame_t *)curr)->frame_index > buddy->frame_index)
                {
                    list_add(&(((frame_t *)curr)->listhead), &(free_list[i-1]));
                    list_add(&(buddy->listhead),             &(free_list[i-1]));

                    // uart_puts("\t\t->index = ");
                    // uart_int((unsigned long long)buddy->frame_index);
                    // uart_puts(",\torder = ");
                    // uart_int((unsigned long long)buddy->frame_order);
                    // uart_puts("\n\t\t->index = ");
                    // uart_int((unsigned long long)((frame_t *)curr)->frame_index);
                    // uart_puts(",\torder = ");
                    // uart_int((unsigned long long)((frame_t *)curr)->frame_order);
                    // uart_puts("\n");
                }
                else
                {
                    list_add(&(buddy->listhead),             &(free_list[i-1]));
                    list_add(&(((frame_t *)curr)->listhead), &(free_list[i-1]));

                    // uart_puts("\t\t->index = ");
                    // uart_int((unsigned long long)((frame_t *)curr)->frame_index);
                    // uart_puts(",\torder = ");
                    // uart_int((unsigned long long)((frame_t *)curr)->frame_order);
                    // uart_puts("\n\t\t->index = ");
                    // uart_int((unsigned long long)buddy->frame_index);
                    // uart_puts(",\torder = ");
                    // uart_int((unsigned long long)buddy->frame_order);
                    // uart_puts("\n");
                }

                curr = temp;
            }
        }
    }
}

void show_free_list(int free_list_order)
{
    list_head_t *curr;
    uart_puts("Free list of order ");
    uart_int(free_list_order);
    uart_puts("\n");
    list_for_each(curr, &free_list[free_list_order])
    {
        uart_puts("\tindex = ");
        uart_int( ((frame_t *)curr)->frame_index );
        uart_puts("\torder = ");
        uart_int( ((frame_t *)curr)->frame_order );
        uart_puts("\n");
    }
}

void* malloc(unsigned int size) {
    // uart_putln("simple malloc");
    char* r = top+0x10;
    top += align_up(size+0x10, 0x10);
    return r;
}

void free(void* ptr) {
    
}