#include "malloc.h"
#include "memlib.h"

// Global variables
void * heap_listp;

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
    heap_listp = mem_sbrk(2*DSIZE);
    if (NULL == heap_listp) {
        return -1;
    }
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

// @brief Merge adjacent free blocks by boundary tag
// @param pb: block pointer
void * coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        // needn't merge
        return bp;
        
    } else if (!prev_alloc && next_alloc) {
        // Merge previous block
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        SET(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        SET(FTRP(bp), PACK(size, 0));
        return PREV_BLKP(bp);
        
    } else if (prev_alloc && !next_alloc) {
        // Merge next block
        size +=  GET_SIZE(HDRP(NEXT_BLKP(bp)));
        SET(HDRP(bp), PACK(size, 0));
        SET(FTRP(bp), PACK(size, 0));
        return bp;
        
    } else {
        // Merge previous && next blocks
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))
                + GET_SIZE(HDRP(NEXT_BLKP(bp)));

        SET(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        SET(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        return PREV_BLKP(bp);
    }
}


void * mm_malloc(size_t size)
{
    
    
}

void * mm_free(void * ptr)
{

}


void * mm_calloc(size_t n, size_t size)
{


}

void * mm_realloc(void * ptr, size_t size)
{

}


