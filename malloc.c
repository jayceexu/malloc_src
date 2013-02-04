#include "malloc.h"
#include "memlib.h"

// Global variables
void * heap_listp;
void * free_listp;

#define FREE_HEAD		((void*)free_listp)
#define FREE_TAIL		((void*)free_listp + DSIZE)

#define PREV_FREEP(bp)		((void*)bp)
#define NEXT_FREEP(bp)		((void*)bp + DSIZE)

// Merge adjacent free blocks by boundary tag
// pb: block pointer
void * coalesce(void *bp);
// Extend heap to words of WORD size
void * extend_heap(size_t words);
// Find a block with size bytes, use first-fit
void * find_fit(size_t size);
void insert_free_block(void * bp);
void remove_free_block(void * bp);
// Remove bp from free block list
// Split if possible
void place(void *bp, size_t asize);


/*
 * @brief Initialize prologue, epilogue. To the purpose of avoid edge
 *        checking during coalescing.
 *
 * begin                                                          end
 * heap                                                           heap  
 *  -------------------------------------------------------------------   
 * |  pad   | hdr(8:a) | ftr(8:a) | zero or more usr blks |  hdr(0:a)  |
 *  -------------------------------------------------------------------
 *          |<----  prologue  --->|                       |<-epilogue->|
 *
 * The prologue SHOULD be 8 bytes. Otherwise the PREV_BLKP will be error.
 **/
int mm_init()
{
    mem_init();
    heap_listp = mem_sbrk(4*DSIZE);
    if (NULL == heap_listp) {
        return -1;
    }
    free_listp = heap_listp;
    
    // Set free list, begin & end
    SET8(heap_listp, 0);
    SET8(heap_listp + DSIZE, 0);

    heap_listp += 2 * DSIZE;
    
    // Set alignment padding
    SET(heap_listp, 0);
    // Set prologue
    SET(heap_listp + WSIZE, PACK(OVERHEAD, 1)); // set hdr
    SET(heap_listp + DSIZE, PACK(OVERHEAD, 1)); // set ftr
    // Set epilogue
    SET(heap_listp + WSIZE + DSIZE, PACK(0, 1));
    // Move heap pointer to footer
    heap_listp += DSIZE;

    // Allocated a free block of CHUNKSIZE bytes
    if (NULL == extend_heap(CHUNKSIZE/WSIZE)) {
        return -1;
    }
    return 0;
}


void * mm_malloc(size_t size)
{
    // size_t won't be negative
    if (size == 0) {  
        return NULL;
    }
    
    size_t asize = 0; // Store adjust size
    if (size <= 2*DSIZE) {
        asize = OVERHEAD + 2*DSIZE; // 2*Dsize is for prev/next pointer in free block 
    } else {
        asize = 2*DSIZE * ((OVERHEAD + size + 2*DSIZE -1) / 2*DSIZE);
    }
    void * bp;
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        
        return bp;
    }

    // No fitable free block. Allocate more space
    size_t extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL) {
        return NULL;
    }
    
    place(bp, extendsize);
    
    return bp;
    
}

void * mm_calloc(size_t n, size_t size)
{
    size_t bytes = n*size;
    if (bytes == 0) {
        return NULL;
    }
    char * ptr = (char*)mm_malloc(bytes);
    if (NULL == ptr) {
        return NULL;
    }

    for (size_t i = 0; i < bytes; ++i) {
        *(ptr + i) = 0;
    }
    return ptr;
}

void * mm_realloc(void * ptr, size_t size)
{
    if (NULL == ptr) {
        return mm_malloc(size);
    }
    if (0 == size) {
        mm_free(ptr);
        return NULL;
    }
    
    size_t old_size = GET_SIZE(HDRP(ptr)) - OVERHEAD;
    if (size <= old_size) {
        // Old space is big enough
        return ptr;
    }

    char * new_mem = (char*)mem_malloc(size);
    if (NULL == new_mem) {
        return NULL;
    }
    // Should only copy old_size bytes
    memcpy(new_mem, ptr, old_size);
    mm_free(ptr);
    return new_mem;
}


void mm_free(void * bp)
{
    if (bp == NULL) {
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    SET(HDRP(bp), PACK(size, 0));
    SET(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}


void * extend_heap(size_t words)
{
    // Adjust for double-word alignment
    size_t bytes = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    void * ptr = mem_sbrk(bytes);
    if (NULL == ptr) {
        return -1;
    }
    SET(HDRP(ptr), PACK(bytes, 0));  // Set header
    SET(FTRP(ptr), PACK(bytes, 0));  // Set footer
    
    SET(FTRP(ptr) + WSIZE, PACK(0, 1)); // Set epilogue

    // Merge free block
    return coalesce(ptr);
}

void * coalesce(void *bp)
{
    if (bp == NULL) {
        return NULL;
    }
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        // needn't merge
        insert_free_block(bp);
        return bp;
        
    } else if (!prev_alloc && next_alloc) {
        // Merge previous block
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        remove_free_block(PREV_BLKP(bp));
        
        SET(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        SET(FTRP(bp), PACK(size, 0));
        
        insert_free_block(PREV_BLKP(bp));
        return PREV_BLKP(bp);
        
    } else if (prev_alloc && !next_alloc) {
        // Merge next block
        size +=  GET_SIZE(HDRP(NEXT_BLKP(bp)));
        remove_free_block(NEXT_BLKP(bp));
        
        SET(HDRP(bp), PACK(size, 0));
        SET(FTRP(bp), PACK(size, 0));

        insert_free_block(bp);
        return bp;
        
    } else {
        // Merge previous && next blocks
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))
                + GET_SIZE(HDRP(NEXT_BLKP(bp)));

        remove_free_block(PREV_BLKP(bp));
        remove_free_block(NEXT_BLKP(bp));
        
        SET(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        SET(FTRP(NEXT_BLKP(bp)), PACK(size, 0));

        insert_free_block(PREV_BLKP(bp));
        return PREV_BLKP(bp);
    }
}


void * find_fit(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    void * bp;
    for (bp = (void*)GET8(FREE_HEAD);
         bp != NULL;
         bp = (void*)GET8(NEXT_FREEP(bp))) {
        
        if (size <= GET_SIZE(HDRP(bp))) {
            return bp;
        }
    }
    return NULL;
}

void insert_free_block(void * bp)
{
    if (NULL == bp) {
        return;
    }

    if (NULL == GET8(FREE_HEAD)) {
        // If free list is empty
        SET8(FREE_HEAD, bp);
        SET8(PREV_FREEP(bp), NULL);
    } else {
        // Appending free block to the end
        SET8(NEXT_FREEP(GET8(FREE_TAIL)), bp);
        SET8(PREV_FREEP(bp), GET8(FREE_TAIL));

    }
    SET8(PREE_TAIL, bp);
    SET8(NEXT_FREEP(bp), NULL);

    return;
}

void remove_free_block(void * bp)
{
    if (NULL == bp) {
        return;
    }

    if (GET8(PREV_FREEP(bp)) == NULL) {
        // If the previous pointer to NULL
        if (GET8(NEXT_FREEP(bp) == NULL)) {
            // If it's the only block
            SET8(FREE_HEAD, NULL);
            SET8(FREE_TAIL, NULL);
            return ;
        }
        SET8(FREE_HEAD, GET8(NEXT_FREEP(bp)));
        SET8(PREV_FREEP(GET8(NEXT_FREEP(bp))), NULL);
        
    } else if (GET8(NEXT_FREEP(bp)) == NULL) {
        SET8(FREE_TAIL, GET8(PREV_FREEP(bp)));
        SET8(NEXT_FREEP(GET8(PREV_FREEP(bp))), NULL);

    } else {
        // In the middle
        SET8(NEXT_FREEP(GET8(PREV_FREEP(bp))), GET8(NEXT_FREEP(bp)));
        SET8(PREV_FREEP(GET8(NEXT_FREEP(bp))), GET8(PREV_FREEP(bp)));
    }
    return ;
}

// bp: point to the start of payload
void place(void * bp, size_t asize)
{
    if (bp == NULL) {
        return;
    }
    remove_free_block(bp);

    size_t old_size = GET_SIZE(HDRP(bp));
    
    if (old_size - asize >= 2*DSIZE + OVERHEAD) {
        // remainder part is big enough to split
        SET(HDRP(bp), PACK(asize, 1));
        SET(FTRP(bp), PACK(asize, 1));
        
        SET(HDRP(NEXT_BLKP(bp)), PACK(old_size - asize, 0));
        SET(FTRP(NEXT_BLKP(bp)), PACK(old_size - asize, 0));
        insert_free_block(NEXT_BLKP(bp));
        
    } else {
        SET(HDRP(bp), PACK(old_size, 1));
        SET(FTRP(bp), PACK(old_size, 1));
    }
}
