// https://k.lre.epita.fr/assignments/sbrk.html

#include "sbrk.h"

static u8 *heap_start;
static u8 *heap_end;
static u8 *heap_limit;

/*
NAME
    sbrk - change data segment size
DESCRIPTION
   sbrk() increments the program's data space by increment bytes. Calling
   sbrk() with an increment of 0 can be used to find the current location
   of the program break.
RETURN VALUE
    sbrk() return the old break value.
*/
// from https://community.silabs.com/s/article/malloc-and-sbrk?language=en_US
void *sbrk(ssize_t increment)
{
    u8 *prev_heap_end = heap_end;

    if (heap_end + increment >= heap_limit)
    {
        return (void *)-1;
    }

    heap_end += increment;
    return (void *)prev_heap_end;
}

void init_sbrk(u8 *start, u8 *limit)
{
    heap_start = start;
    heap_end = start;
    heap_limit = limit;
}
