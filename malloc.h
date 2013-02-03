// Copyright (C) xujaycee@gmail.com

// A implementation of malloc library
// Quote cmu <cs:app>
// In use of boundary tags coalescing

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12) // default heap size for initialization
#define OVERHEAD 8

#define MAX(x,y) 		((x)>(y) ? (x) : (y))


#define PACK(size, alloc) 	((size) | (alloc))

// Get and Set a word in address p
#define GET(p) 			(*(size_t*)(p))
#define SET(p, val) 		(*(size_t*)(p) = val)
// GET8 and SET8 is for portable on 32-bit & 64-bit machines
#define GET8(p)      		(*(unsigned long *)(p))
#define SET8(p, val) 		(*(unsigned long *)(p) = (unsigned long)(val))

// Get the size and allocated fields
#define GET_SIZE(p) 		(GET(p) & ~0x7)
#define GET_ALLOC(p)		(GET(p) & 0x1)

// Get the header & footer pointer according to block pointer
#define HDRP(bp)		((void*)bp - WSIZE)
#define FTRP(bp)		((void*)bp + GET_SIZE(HDRP(bp)) - DSIZE)

// Get the next & previous block pointer according to block pointer
#define NEXT_BLKP(bp) 		((void*)bp + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) 		((void*)bp - GET_SIZE((void*)bp - DSIZE))

int mm_init();

// DIY version of glibc malloc family
void * mm_malloc(size_t size);
void * mm_calloc(size_t n, size_t size);
void * mm_realloc(void * ptr, size_t size);
void mm_free(void * ptr);

