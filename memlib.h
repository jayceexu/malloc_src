//  Copyright (C) xujaycee@gmail.com
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

// This is the project for practising
// To acquaint the knowlege of malloc mechanism
// Header for basic lib of heap allocated family

#ifndef MEMLIB_H
#define MEMLIB_H

void mem_init();
void mem_deinit();
void * mem_sbrk(int incr);
void mem_reset_brk();

// Get heap point
void *mem_heap_low();
void *mem_heap_high();


size_t mem_getheapsize();
size_t mem_getpagesize();

#endif
