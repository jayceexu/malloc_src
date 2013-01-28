#include <stdio.h>
#include <unistd.h>
#include "memlib.h"
#include "config.h"

static char heap[HEAP_SIZE];
static char * mem_brk; // the first available heap address
static char * mem_max_addr; // The highest address of heap PLUS 1

void mem_init()
{
    mem_brk = heap;
    mem_max_addr = heap + HEAP_SIZE;
}

void mem_deinit()
{
    
}

void * mem_sbrk(int incr)
{
    if (incr < 0) {
        fprintf (stderr, "Invalid argument, incr=[%d] should >= 0\n",
                 incr);
        return (void*)-1;
    }
    char * old_brk = mem_brk;
    // In case of insufficient heap space
    if (incr + mem_brk >= mem_max_addr) {
        return (void*)-1;
    }
    mem_brk += incr;
    return (void*)old_brk;
}

void mem_reset_brk()
{
    mem_brk = heap;

}

// Get the first heap byte 
void *mem_heap_low()
{
    return (void*)heap;
}

// Get the last heap byte
void *mem_heap_high()
{
    return (void*)(mem_brk - 1);
}

// Return the allocated byte size
size_t mem_getheapsize()
{
    return (size_t)(mem_brk - heap);
}

size_t mem_getpagesize()
{
    return getpagesize();
}

