#include "malloc.h"

extern char _heap_start;
static char* top = &_heap_start;

static unsigned long align_up(unsigned long n, unsigned long align)
{
    return (n + align - 1) & (~(align - 1));
}

void* malloc(unsigned int size) {
    char* r = top+0x10;
    top += align_up(size+0x10, 0x10);
    return r;
}