/*
 * mm-implicit.c - an empty malloc package
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 *
 * @id : 201902745
 * @name : 장한
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif


/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)

#define WSIZE 4 // 1개의 word 사이즈
#define DSIZE 8 // 2배의 word 사이즈
#define CHUNKSIZE (1 << 12) // heap을 늘릴 때 얼만큼 늘릴거냐? 4kb 분량.
#define OVERHEAD 8 // 
#define MAX(x, y) ((x) > (y) ? (x) : (y)) // 주어진 값 중 어느 것이 더 클지 정하는 MAX 함수
// size를 pack하고 개별 word 안의 bit를 할당 (size와 alloc을 비트연산), 헤더에서 써야하기 때문에 만emfa.
#define PACK(size, alloc) ((size) | (alloc)) // alloc : 가용여부 (ex. 000) / size : block size를 의미. =>합치면 온전한 주소가 나온다
/* address p위치에 words를 read와 write를 한다. */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))


static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void *coalesce(void * bp);
static void place(void *bp, size_t asize);
void *malloc (size_t size);
void free (void *bp);
void *realloc(void *oldptr, size_t size);
void *calloc (size_t nmemb, size_t size);
// static int in_heap(const void *p);
// static int aligned(const void *p);
void mm_checkheap(int verbose);

static char * heap_listp = 0;
static char * next_fit;

/*
 * Initialize: return -1 on error, 0 on success.
 */
int mm_init(void) {

    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == NULL) // 초기 empty heap 생성
        return -1;                      // heap_listp = 새로 생성되는 heap 영역의 시작
    
    PUT(heap_listp, 0); // heap의 시작부분 할당 /* Alignment padding */
    PUT(heap_listp + WSIZE, PACK(OVERHEAD, 1)); /* Prologue header */
    PUT(heap_listp + DSIZE, PACK(OVERHEAD, 1)); /* Prologue footer */
    PUT(heap_listp + WSIZE + DSIZE, PACK(0, 1)); /* Epilogue header */ // when find func, note endpoint
    heap_listp += DSIZE;

    /* Extend the empty heap with a free block of CHUNKSIZE bytes*/
    if(extend_heap(CHUNKSIZE / WSIZE) == NULL) // CHUNKSIZE 바이트의 free block 생성
        return -1;
    next_fit = heap_listp;
    return 0;
}


/*
 * malloc
 */
void *malloc (size_t size)
{    
    size_t asize; /* Adjusted block size */
    size_t extendsize; /* Amout to extend heap if no fit*/
    char *bp;

    /* Ignore spurious requests */
    // if(size == 0){
    //     return NULL;
    // }
    /* Adjust block size to include overhead and alignment reqs. */
    // 할당할 크기가 DSIZE 보다 작은 경우, align을 위해 블록 크기를 DSIZE+OVERHEAD로 설정
    if (size <= DSIZE)
        asize = 2*DSIZE;
    else // 그 이외의 경우는 적당하게 align을 맞춰 블록 크기 설정
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    
    // Search the free list for a fit
    if((bp = find_fit(asize)) != NULL) // find_fit 함수로 적당한 블록 탐색
    {
        place(bp, asize);// 위에서 찾은 block에 가용블록 표시 및 사이즈
        return bp;
    }
    // find_fit 함수로 블록을 못 찾은 경우, 남은 Heap이 없으므로 extend_heap 호출
    // extend_heap 호출 후 블록 할당
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;

    place(bp, asize);
    return bp;
}

/*
 * free
 */
void free (void *bp) {
    if(!bp)
        return;
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    next_fit = coalesce(bp);
}

static void *find_fit(size_t asize)
{
    /* next-fit search */
    char *bp;

    //start the search from the beginning of the heap
    for (bp = next_fit; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }
    return NULL;/* No fit */
    // char *bp = next_fit;
    // // Search from next_fit to the end of the heap
    // for (next_fit = bp; GET_SIZE(HDRP(next_fit)) > 0; next_fit = NEXT_BLKP(next_fit))
    // {
    //     if (!GET_ALLOC(HDRP(next_fit)) && (asize <= GET_SIZE(HDRP(next_fit))))
    //     {
    //         // If a fit is found, return the address the of block pointer
    //         return next_fit;
    //     }
    // }
    // // If no fit is found by the end of the heap, start the search from the
    // // beginning of the heap to the original next_fit location
    // for (next_fit = heap_listp; next_fit < bp; next_fit = NEXT_BLKP(next_fit))
    // {
    //     if (!GET_ALLOC(HDRP(next_fit)) && (asize <= GET_SIZE(HDRP(next_fit))))
    //     {
    //         return next_fit;
    //     }
    // }
    // return NULL; /* No fit */

}

static void place(void *bp, size_t asize) {

    size_t csize = GET_SIZE(HDRP(bp));
    if((csize - asize) >= (2*DSIZE)) 
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
    }
     else
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

static void *extend_heap(size_t words) {
    char * bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    PUT(HDRP(bp), PACK(size, 0));       /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));       /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));/* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

static void *coalesce(void * bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        return bp;
    }
    else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

/*
 * realloc - you may want to look at mm-naive.c
 */

void *realloc(void *oldptr, size_t size)
{
  size_t oldsize;
  void *newptr;

  /* If size == 0 then this is just free, and we return NULL. */
  if(size == 0) {
    free(oldptr);
    return 0;
  }

  /* If oldptr is NULL, then this is just malloc. */
  if(oldptr == NULL) {
    return malloc(size);
  }

  newptr = malloc(size);

  /* If realloc() fails the original block is left untouched  */
  if(!newptr) {
    return 0;
  }

  /* Copy the old data. */
  oldsize = GET_SIZE(HDRP(oldptr));
  if(size < oldsize) oldsize = size;
  memcpy(newptr, oldptr, oldsize);

  /* Free the old block. */
  free(oldptr);

  return newptr;
}


/*
 * calloc - you may want to look at mm-naive.c
 * This function is not tested by mdriver, but it is
 * needed to run the traces.
 */
void *calloc (size_t nmemb, size_t size)
{
  size_t bytes = nmemb * size;
  void *newptr;

  newptr = malloc(bytes);
  memset(newptr, 0, bytes);

  return newptr;
}


/*
 * Return whether the pointer is in the heap.
 * May be useful for debugging.
 */
// static int in_heap(const void *p) {
//     return p < mem_heap_hi() && p >= mem_heap_lo();
// }

// /*
//  * Return whether the pointer is aligned.
//  * May be useful for debugging.
//  */
// static int aligned(const void *p) {
//     return (size_t)ALIGN(p) == (size_t)p;
// }

/*
 * mm_checkheap
 */
void mm_checkheap(int verbose) {
}
