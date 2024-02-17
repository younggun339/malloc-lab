/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
  * NOTE TO STUDENTS: Before you do anything else, please
  * provide your team information in the following struct.
  ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
/* Basic constants and macros */
#define WSIZE 4                 /* Word and header/footer size (bytes) */
#define DSIZE 8                 /* Double word size (bytes) */
#define CHUNKSIZE (1<<12)       /* Extend heap by this amount (bytes) */
#define MAX(x, y)           ((x) > (y) ? (x) : (y))
/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)   ((size) | (alloc))
/* Read and write a word at address p */
#define GET(p)              (*(unsigned int *)(p))
#define PUT(p, val)         (*(unsigned int *)(p) = (val))
/* Read the size and allocated fields from address p */
#define GET_SIZE(p)         (GET(p) & ~0x7)
#define GET_ALLOC(p)        (GET(p) & 0x1)
/* Given block ptr bp, compute address of its header and footer */
// bp가 항상 직접 읽어올 준비를 하기 위해 헤더 시작점이 아닌 메모리 시작점에 있구나!
#define HDRP(ptr)            ((char *)(ptr) - WSIZE)
#define FTRP(ptr)            ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)
/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(ptr)       ((char *)(ptr) + GET_SIZE(HDRP(ptr)))
#define PREV_BLKP(ptr)       ((char *)(ptr) - GET_SIZE(((char *)(ptr) - DSIZE)))
// For First-Fit
static char* FF_ptr;
// For Next-Fit
static char* NF_ptr;
static int circled = 0x0;

static void* coalesce(void* ptr);
static void* extend_heap(size_t words);
static void place(void* ptr, size_t newsize);
static void* find_fit(size_t newsize);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((NF_ptr = mem_sbrk(4 * WSIZE)) == (void*)-1)
        return -1;
    /* Alignment pading */
    PUT(NF_ptr, 0);
    /* Prologue header */
    PUT(NF_ptr + (1 * WSIZE), PACK(DSIZE, 1));
    /* Prologue footer */
    PUT(NF_ptr + (2 * WSIZE), PACK(DSIZE, 1));
    /* Epilogue header */
    PUT(NF_ptr + (3 * WSIZE), PACK(0, 1));
    FF_ptr = NF_ptr + 2;
    // printf("%d\n", mem_heapsize());
    /* Extend the empty heap with a free block of CHUNKSIZE(1024) bytes */
    //****** 어차피 extend_heap에서 에필로그 헤더를 정의할거면 왜 위에서 먼저 정의하고 호출하지? 번거롭게
    if ((NF_ptr = extend_heap(CHUNKSIZE / WSIZE)) == NULL)
        return -1;
    return 0;
}
/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void* mm_malloc(size_t size)
{
    if (size == 0)
        return NULL;
    size_t newsize = ALIGN(size) + DSIZE;
    char* ptr;
    if ((ptr = find_fit(newsize)) != NULL) {
        place(ptr, newsize);
        return ptr;
    }
    /* Np fit found. Get more memory and place the block */
    size_t extend_size = MAX(newsize, CHUNKSIZE);
    if ((ptr = extend_heap(extend_size / WSIZE)) == NULL)
        return NULL;
    place(ptr, newsize);
    return ptr;
}
/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void* ptr)
{
    // 메모리를 반납시킬 포인터(ptr)가 현재 위치하고 있는 메모리의 사이즈를 가져옴
    size_t size = GET_SIZE(HDRP(ptr));
    // ptr이 현재 위치하고 있는 메모리의 헤더를 (size, 1) -> (size, 0)으로 만듦, 즉 반납
    PUT(HDRP(ptr), PACK(size, 0));
    // ptr이 현재 위치하고 있는 메모리의 푸터를 (size, 1) -> (size, 0)으로 만듦, 즉 반납
    PUT(FTRP(ptr), PACK(size, 0));
    /* Coalesce if the previous block was free */
    coalesce(ptr);
}
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void* mm_realloc(void* ptr, size_t size)
{
    void* oldptr = ptr;
    void* newptr;
    size_t copySize;
    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = *(size_t*)((char*)oldptr - SIZE_T_SIZE);
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
static void* extend_heap(size_t words)
{
    char* ptr;
    size_t size;
    circled |= 0x1;
    /* Allocate an even number of words to maintain alingnment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    //******** 왜 long으로 캐스팅????
    if ((long)(ptr = mem_sbrk(size)) == -1)
        return NULL;
    /* Initialize free block header/footer and the epilogue header */
    /* Free block list header */
    PUT(HDRP(ptr), PACK(size, 0));
    /* Free block list footer */
    PUT(FTRP(ptr), PACK(size, 0));
    /* New epilogue header */
    PUT(HDRP(NEXT_BLKP(ptr)), PACK(0, 1));
    /* Coalesce if the previous block was free */
    return coalesce(ptr);
}
static void* coalesce(void* ptr)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t size = GET_SIZE(HDRP(ptr));
    if (prev_alloc == 1 && next_alloc == 1) {
        // do nothing
    }
    else if (prev_alloc == 1 && next_alloc == 0) {
        size += GET_SIZE(HDRP((NEXT_BLKP(ptr))));
        // NEXT 블록의 FOOTER 부터 갱신
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
        PUT(HDRP(ptr), PACK(size, 0));
    }
    else if (prev_alloc == 0 && next_alloc == 1) {
        size += GET_SIZE(HDRP((PREV_BLKP(ptr))));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        PUT(FTRP(ptr), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    }
    else {
        size += GET_SIZE(FTRP(PREV_BLKP(ptr)));
        size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    }
    return ptr;
}
static void* find_fit(size_t newsize)
{
    /* First-fit search */
    //void* ptr;
    //for (ptr = FF_ptr; GET_SIZE(HDRP(ptr)) > 0; ptr = NEXT_BLKP(ptr)) {
    //    printf("%zu\n", (size_t)((char* )ptr - (char*)mem_heap_lo()));      // 4 -> 16
    //    if (GET_ALLOC(HDRP(ptr)) == 0 && newsize <= GET_SIZE(HDRP(ptr))) {
    //        return ptr;
    //    }
    //}
    //return NULL;
    /* Next-fit search */
    for (;; NF_ptr = NEXT_BLKP(NF_ptr)) {
        if (GET_ALLOC(HDRP(NF_ptr)) == 0 && newsize <= GET_SIZE(HDRP(NF_ptr))) {
            return NF_ptr;
        }
        if (*NF_ptr == PACK(0, 1)) {
            if (circled == 1) {
                circled &= 0x0;
                NF_ptr = FF_ptr;
            }
            else
                break;
        }
    }
    return NULL;
}
static void place(void* ptr, size_t newsize) {
    size_t csize = GET_SIZE(HDRP(ptr));
    // 가용 블록이 남지 않고 딱 맞는다면
    if (csize == newsize) {
        PUT(HDRP(ptr), PACK(newsize, 1));
        PUT(FTRP(ptr), PACK(newsize, 1));
    }
    // 가용 블록이 남는다면
    else {
        PUT(HDRP(ptr), PACK(newsize, 1));
        PUT(FTRP(ptr), PACK(newsize, 1));
        PUT(HDRP(NEXT_BLKP(ptr)), PACK(csize - newsize, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(csize - newsize, 0));
    }
}