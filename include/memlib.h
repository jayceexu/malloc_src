//  Copyright (C) xujaycee@gmail.com

// This is the project for practising
// To acquaint the knowlege of malloc mechanism
// Header for basic lib of heap allocated family

#ifndef MEMLIB_H
#define MEMLIB_H

void mem_init();
void mem_deinit();
void * mem_sbrk(int incr);
void mem_reset_brk();

// Get the first allocated heap byte 
void *mem_heap_low();

// Get the last allocated heap byte
void *mem_heap_high();


size_t mem_getheapsize();
size_t mem_getpagesize();

#endif
