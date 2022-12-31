/*
 * mm-explicit.c - an empty malloc package
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
#define HDRSIZE     4 // header size (bytes)
#define FTRSIZE     4 // footer size (bytes)
#define WSIZE       4 // word size
#define DSIZE       8 // doubleword size (bytes)
 //힙 영역을 한 번 늘릴 때 마다 늘려 줄 크기
#define CHUNKSIZE   (1<<12) // initial heap size
#define OVERHEAD    8 // overhead of header and footer (bytes)

#define MAX(x, y) ((x) > (y)?  (x) : (y))
#define MIN(x, y) ((x) < (y)?  (x) : (y))

// Pack a size and allocated bit into a word
/* 크기와 할당 상태를 1워드로 묶는다 */
#define PACK(size, alloc) ((unsigned)((size) | (alloc)))

// read and write a word at address p
/* 주소 p에 있는 값을 읽고 쓴다 */

#define GET(p) 				(*(unsigned*)(p))
#define PUT(p,val) 			(*(unsigned*)(p) = (unsigned)(val))
#define GET8(p) 			(*(unsigned long*)(p))
#define PUT8(p,val) 		(*(unsigned long*)(p) = (unsigned long)(val))

// read the size and allocated fields from address p
/* 주소 p에서 블록의 크기와 할당상태를 읽어온다 */
#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

// Given block ptr bp, compute address of its header and footer
/* 블록 포인터 bp를 받으면, 그 블록의 헤더와 풋터 주소를 반환한다 */
#define HDRP(bp)    ((char *)(bp) - WSIZE)
/* 포인터가 배열[0]을 가리키고 있을 것이기 때문에 1워드만큼 앞으로 가면 헤더가 나온다 */
#define FTRP(bp)    ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// Given block ptr bp, compute address of next and previous blocks
/* Compute address of next & prev free block entries */
#define NEXT_FREEP(bp) ((char *)(bp))
#define PREV_FREEP(bp) ((char *)(bp) + DSIZE)

// Given free block pointer bp,compute address of next and previous free blocks
#define NEXT_FREE_BLKP(bp) ((char *)GET8((char*)(bp)))
#define PREV_FREE_BLKP(bp) ((char *)GET8((char*)(bp) + DSIZE))

/* 포인터를 헤더로 옮겨서 사이즈를 가져온 다음, 포인터부터 시작해서 사이즈만큼 뒤로 간다, 
    그리고 헤더와 풋터를 제외한만큼(2워드)앞으로 간다 */
/* 블록 포인터 bp를 받으면, 그 이전 블록과 이후의 블록의 주소를 반환한다 */
// Given free block pointer bp, compute address of next pointer and prev pointer
/* 현재 블록의 헤더로 가서 사이즈 값을 받아 그 만큼 뒤로 간다 -> 다음 블록 시작으로 간다 */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char*)(bp) - WSIZE))
/* 이전 블록의 풋터로 간 다음 사이즈 값을 받아 이전 블록의 시작으로 간다 */
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char*)(bp) - DSIZE))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)

static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void *coalesce(void * bp);
static void place(void *bp, size_t asize);
static void removes(void *bp);
void *malloc (size_t size);
void free (void *bp);
void *realloc(void *oldptr, size_t size);
void *calloc (size_t nmemb, size_t size);
// static int in_heap(const void *p);
// static int aligned(const void *p);
void mm_checkheap(int verbose);

static char* h_ptr = 0; //free list
static char* heap_start = 0;
/*
 * Initialize: return -1 on error, 0 on success.
 */
int mm_init(void) {
    // Request memory for the initial empty heap
    // 6개 padding, prol_header, prol_footer, PRE, SUCC, epli_header
    if((heap_start = mem_sbrk(2 * 4 * HDRSIZE)) == NULL)
        return -1;

    PUT(heap_start, NULL); // 더블 워드 경계로 정렬된 미사용 패딩
    PUT(heap_start + DSIZE, NULL);
    PUT(heap_start + DSIZE * 2, 0);
    PUT(heap_start + DSIZE * 2 + HDRSIZE, PACK(OVERHEAD, 1));
    PUT(heap_start + DSIZE * 2 + HDRSIZE + FTRSIZE, PACK(OVERHEAD, 1));
    PUT(heap_start + DSIZE * 2 + HDRSIZE * 2, PACK(0, 1));

    // Move heap pointer over to footer
	h_ptr = heap_start; //가용 블록 리스트 포인터에 맨 앞의 NEXT 주소를 저장함, 연결 리스트의 시작
	heap_start += DSIZE*2;
    // Leave room for the previous and next pointers, place epilogue 3 words down
    // epilogue = heap_start + HDRSIZE;

    // extend the empty heap with a free block of CHUNKSIZE bytes
    // 그 후 CHUNKSIZE 만큼 힙을 확장해 초기 가용 블록을 생성 실패 시 -1, 성공 시 0 return
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/*
 * extend_heap
 */
static void *extend_heap(size_t words){
	char *bp;
	unsigned size;
	//홀 수 사이즈 조정
	size = (words % 2) ? (words+1)*WSIZE : words*WSIZE;
	//bp에 size에 해당하는 공간 할당
	if((long)(bp = mem_sbrk(size)) < 0)
		return NULL;

	//초기셋팅
	PUT(HDRP(bp), PACK(size,0));
	PUT(FTRP(bp), PACK(size,0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));
	//병합 & free block list에 추가될 것임 
	return coalesce(bp);
}

static void *find_fit(size_t asize){
	void *bp;
	//first fit
	for(bp = h_ptr; bp != NULL; bp = NEXT_FREE_BLKP(bp))
		if(asize <= (size_t)GET_SIZE(HDRP(bp)))
			return bp;

	return NULL;
}

static void place(void *bp, size_t asize){
	size_t csize = GET_SIZE(HDRP(bp));
	//블록 분할이 가능한 경우
	if((csize-asize) >= (3*DSIZE)){
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		removes(bp);
		bp = NEXT_BLKP(bp);
		PUT(HDRP(bp), PACK(csize-asize, 0));
		PUT(FTRP(bp), PACK(csize-asize, 0));
		coalesce(bp);
	}
	//한 번에 배치
	else{
		PUT(HDRP(bp), PACK(csize, 1));
		PUT(FTRP(bp), PACK(csize, 1));
		removes(bp);
	}
	
}
/*
 * malloc
 */

void *malloc (size_t size) {
    char *bp;                   // block pointer, points to first byte of payload
    unsigned asize;             // block size adjusted for alignment and overhead
    unsigned extendsize;        // amount to extend heap if no fit
	//exception
    if (size == 0)      // size가 올바르지 않을 때 예외 처리
		return NULL;
	//allocated size
	asize = MAX(ALIGN(size) + DSIZE, 2*DSIZE+2*WSIZE);
	//find -> alloc
	if((bp = find_fit(asize))){    // block의 크기 결정
		place(bp, asize);
		return bp;
	}
	//alloc 실패, extend 
	extendsize = MAX(asize, CHUNKSIZE);
	if((bp = extend_heap(extendsize/WSIZE)) == NULL)
		return NULL;
	//다시 alloc
	place(bp,asize);

	return bp;
}
/*
 * free
 */
void free (void *ptr) {
	//exception
	if(!ptr)
		return;
	//alloc bit 초기화
	size_t size = GET_SIZE(HDRP(ptr));
	PUT(HDRP(ptr), PACK(size,0));
	PUT(FTRP(ptr), PACK(size, 0));
	//free block이 생성됨에 따라 coalesce 호출
	coalesce(ptr);
}

static void *coalesce(void *bp){
	//     //직전 블록의 footer, 직후 블록의 header를 보고 가용 여부를 확인.
	size_t next_alloc = GET_ALLOC((void*)(FTRP(bp))+WSIZE); // 직전 블록 가용 여부 확인
	size_t prev_alloc = GET_ALLOC((void*)(bp) - 2*WSIZE); // 직전 블록 가용 여부 확인
	size_t size = GET_SIZE(HDRP(bp));
	// case 1: 직전, 직후 블록이 모두 할당 -> 해당 블록만 free list에 넣어준다.
    // case 2: 직전 블록 할당, 직후 블록 가용
    // case 3: 직전 블록 가용, 직후 블록 할당
    // case 4: 직전, 직후 블록 모두 가용

	//병합 불가
	if(prev_alloc & next_alloc){}
	//next 병합 가능
	else if(prev_alloc && !next_alloc){
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		removes(NEXT_BLKP(bp));
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}
	//prev 병합 가능
	else if(!prev_alloc && next_alloc){
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
		bp = PREV_BLKP(bp);
		removes(bp);
	}
	// next & prev 병합 가능
	else{
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
		removes(NEXT_BLKP(bp));
		removes(PREV_BLKP(bp));
		bp = PREV_BLKP(bp);
	}
	// 연결된 새 가용 블록을 free list에 추가
	//free block list 맨 앞에 추가
	if(h_ptr){
		PUT8(NEXT_FREEP(bp),h_ptr);
		PUT8(PREV_FREEP(bp), NULL);
		PUT8(PREV_FREEP(h_ptr),bp);
		h_ptr = bp;
	}
	//free block list가 비어있음.
	else{
		h_ptr = bp;
		PUT8(NEXT_FREEP(h_ptr), NULL);
		PUT8(PREV_FREEP(h_ptr), NULL);
	}

	return bp;
}

/*
 * realloc - you may want to look at mm-naive.c
 */
void *realloc(void *oldptr, size_t size) {
   
	size_t oldsize;
	void *newptr;

	if(size == 0){
		free(oldptr);
		return 0;
	}

	if(oldptr == NULL)
		return malloc(size);

	newptr = malloc(size);

	if(!newptr)
		return 0;

	oldsize = GET_SIZE(HDRP(oldptr));
	if(size < oldsize)
		oldsize = size;
	memcpy(newptr, oldptr, oldsize);

	free(oldptr);

	return newptr;
}

/*
 * calloc - you may want to look at mm-naive.c
 * This function is not tested by mdriver, but it is
 * needed to run the traces.
 */

void *calloc (size_t nmemb, size_t size) {
  size_t bytes = nmemb * size;
  void *newptr;

  newptr = malloc(bytes);
  memset(newptr, 0, bytes);

  return newptr;
}
/*
 * removes
 */
static void removes(void *bp){
	//앞 뒤 모두 존재
	if(PREV_FREE_BLKP(bp) && NEXT_FREE_BLKP(bp)){
		PUT8(NEXT_FREEP(PREV_FREE_BLKP(bp)), NEXT_FREE_BLKP(bp));
		PUT8(PREV_FREEP(NEXT_FREE_BLKP(bp)), PREV_FREE_BLKP(bp));
	}
	//뒤만 존재
	else if(!PREV_FREE_BLKP(bp) && NEXT_FREE_BLKP(bp)){
		h_ptr = NEXT_FREE_BLKP(bp);
		PUT8(PREV_FREEP(NEXT_FREE_BLKP(bp)),NULL);
	}
	//앞만 존재
	else if(PREV_FREE_BLKP(bp) && !NEXT_FREE_BLKP(bp)){
		PUT8(NEXT_FREEP(PREV_FREE_BLKP(bp)),NULL);
	}
	//앞, 뒤 모두 존재하지 않음
	else if(!PREV_FREE_BLKP(bp) && !NEXT_FREE_BLKP(bp)) {
		h_ptr = NULL;
	}
}

// /*
//  * Return whether the pointer is in the heap.
//  * May be useful for debugging.
//  */
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
