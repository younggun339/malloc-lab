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

static char *heap_listp = 0;
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void *free_insert(void *bp);
static void *free_delete(void *bp);


int mm_init(void)
{
    //printf("시이작!\n");
    	// 만약 현재 4배의 더블 워드를 더한다면 죽는다고 선언할 경우 실패.
	if((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
    	return -1;
    
    //printf("%p 을 시작점으로 간다!\n", heap_listp);
    // 루트 노드.
    PUT(heap_listp, PACK(2*DSIZE, 0));
    PUT(heap_listp + (1*WSIZE), NULL); // prev
    PUT(heap_listp + (2*WSIZE), NULL); //next
    // 에필로그 헤더.
    PUT(heap_listp + (2*DSIZE), PACK(0,1));
    heap_listp += (2*WSIZE);
    //printf("%p 로 이동 했다!\n", heap_listp);
    
    // 다 배정했으니 heap의 크기를 늘려놓겠다.
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
    	return -1;

    //printf("init 종료!\n");
    //printf("----------\n");
    return 0;

}


void *mm_malloc(size_t size)
{
    //printf("%d 만큼 크기 할당할게!\n", size);

	size_t asize; // 적정 블록 사이즈.
    size_t extendsize; // 만약 초과한다면 참고할 사이즈. 
    char *bp;
    
    // 만약 용량을 요구하지 않을 경우 무시.
    if (size == 0)
    	return NULL;
    
    // overhead와 정렬 reqs를 포함한 사이즈.
    if (size <= DSIZE)
    	asize = 2*DSIZE;
    else
    	asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

    //printf("%d 로 가공해드렸습니다.\n", asize);

    // free list에서 딱 맞는 거 찾기.
    if ((bp = find_fit(asize)) != NULL){
        //printf("%p 에 배치되시면 될것 같군요.\n", bp);
    	place(bp, asize);
        free_delete(bp);
        //printf("malloc 종료!\n");
        //printf("----------\n");
        return bp;}
    else{
        coalesce();
    }

    if((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        free_delete(bp);
        return bp;
    }
    else{
            // 딱 맞는거 못 찾았다면 메모리 더 갖고 오기.    
        extendsize = MAX(asize, CHUNKSIZE);
        //printf("%d 만큼 더 늘려야할 것 같은데?\n", extendsize);
        if ((bp = extend_heap(extendsize/WSIZE)) == NULL){
            //printf("malloc 종료!\n");
            //printf("----------\n");
            return NULL;
        }
        place(bp, asize);
        // tmp_ptr = bp;
        //printf("malloc 종료!\n");
        //printf("----------\n");
        return bp;

    }
        

    
    }


void mm_free(void *ptr)
{
    //printf("%p에 있는걸 해제시키신다고요?\n", ptr);
    // 헤더 위치에 있는 사이즈를 가져온다.
	size_t size = GET_SIZE(HDRP(ptr));
    //printf("%d 만큼의 크기의 블록이군요.\n", size);
    
    // 가용 가능인 걸로 헤더와 풋터를 바꿔버림.
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    
    PUT(PREV_FREE(ptr), NULL);
    PUT(NEXT_FREE(ptr), NULL);
    free_insert(ptr);
    // 기준으로 연결 가능한 지에 따라 연결.
    //printf("가용처리 완료! %p가 연결되는지 확인하게 시키고.\n", ptr);
    //printf("free 종료!\n");
    //printf("----------\n");
    }


// first fit
static void *find_fit(size_t asize){
    //printf("%d 만한 적당한 크기가 있는지 확인해보겠습니다.\n", asize);

    void *bp;

    for (bp = DSIZE; GET(bp) != DSIZE; bp = NEXT_BLKP(bp))
    {
        //printf("현재 %p가 위치인데...\n", bp);
        //printf("%d 크기만큼의 블록이긴한데...\n",GET_SIZE(HDRP(bp)));
        if((asize <= GET_SIZE(HDRP(bp)))){
            //printf("할당 받을수도 있을거같고, 내가 필요한 크기보다 큰데?\n");
            //printf("%p를 반환해야징.\n",bp);
            return bp;
        }
        //printf("음, 다음 블록을 봐야겠군.\n");
    }

    //printf("포기! NULL입니다!\n");
    return NULL;
}

static void place(void *bp, size_t asize){
    //printf("%p에 %d만한 걸 넣어볼까.\n",bp, asize);
    size_t csize = GET_SIZE(HDRP(bp));
    //printf("지금 넣을 블록은 %d 크기니까...\n", csize);

    if ((csize - asize)>= (2*DSIZE)){
        //printf("휴, 공간이 넉넉하니 잘라서 쓰자.\n");
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        free_delete(bp);
        bp = NEXT_BLKP(bp);
        //printf("%p 위치는 비워놔야겠당. %d는 넉넉하겠군!\n", bp, csize-asize);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
        free_insert(bp);
    }
    else{
        //printf("공간이 넉넉하지 않으니 그대로 쓸까.\n");
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        free_delete(bp);
    }
    //printf("place 종료!\n");
    //printf("----------\n");
}


static void *coalesce()
{

    void *bp = NEXT_BLKP(WSIZE);
    
    //printf("%p 에 있는 거 연결을 확인할 건데...\n", bp);
    
    while(bp != WSIZE){
        // 앞, 뒤 블록의 가용 여부 가져오기.
        size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
        size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
        // 현재 블록 가용 여부 가져오기.
        size_t temp_alloc = GET_ALLOC(HDRP(bp));
        // 현재 블록 크기 가져오기.
        size_t size = GET_SIZE(HDRP(bp));
        //printf("내가 받은 블록 포인터 크기가 %d 라고?\n", size);
        
        if(!temp_alloc){
            // case 1 : 둘 다 할당.
            if(prev_alloc && next_alloc) {
                bp = NEXT_BLKP(bp);
                    //printf("앞뒤 찼다!\n");
                    //printf("다음!\n");
                }
            // case 2 : 뒤가 가용.
            else if (prev_alloc && !next_alloc){
                //printf("뒤가 비었다!\n");
                size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
                free_delete(bp);
                PUT(HDRP(bp), PACK(size, 0));
                PUT(FTRP(bp), PACK(size, 0));
                free_insert(bp);
                //printf("bp는 그대로 두고 뒷놈만 풀어줬어요!\n");
                bp = NEXT_BLKP(bp);
                }

            // case 3 : 앞이 가용.
            else if (!prev_alloc && next_alloc){
                //printf("앞이 비었다!\n");
                size += GET_SIZE(HDRP(PREV_BLKP(bp)));
                free_delete(bp);
                PUT(FTRP(bp), PACK(size, 0));
                PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
                bp = PREV_BLKP(bp);
                free_insert(bp);
                //printf("bp는 %p로 이동하고, 원래 풋터랑 이전 헤더 풀어줬어요!\n",(void*)bp);
                bp = NEXT_BLKP(bp);
                }
            // case 4 : 앞 뒤 가용.
            else{
                //printf("앞뒤가 비었다!\n");
                size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
                free_delete(bp);
                PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
                PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
                bp= PREV_BLKP(bp);
                free_insert(bp);
                //printf("bp는 %p로 이동하고, 뒤엣놈 풋터랑 이전 헤더 풀어줬어요!\n",(void*)bp);
                bp = NEXT_BLKP(bp);
                }
            }
        }

    //printf("coalesce 종료!\n");
    //printf("----------\n");
    
    }
     

static void *extend_heap(size_t words)
{
	char *bp;
    size_t size;

    //printf("%d 만큼 힙을 늘려줄까해요.\n", words);
   
   // 짝수배의 워드만큼 할당.
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    //printf("%d 로 정렬해서 늘려줘야지.\n", size);
    if ((long)(bp = mem_sbrk(size)) == -1)
    	return NULL;

    //printf("%p 라는 새로운 포인터 도착!\n", bp);    
    //free block 헤더, 풋터, 에필로그 헤더 초기화.
    PUT(HDRP(bp), PACK(size,0)); // free 헤더
    PUT(bp, NULL); // free prev
    PUT(bp+WSIZE, NULL); //free next
    PUT(FTRP(bp), PACK(size,0)); // free 풋터
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1)); // 새로운 에필로그 헤더
    
    free_insert(bp);
    //printf("extend_heap 종료!\n");
    //printf("----------\n");
    return ;
    
    }


void free_insert(void *ptr){

    void *bp = DSIZE;

    size_t next_block = GET(bp); //다음 bp의 포인터 값.

    size_t next_next_block = GET(NEXT_BLKP(next_block)); // 다다음 bp의 포인터 값.

    PUT(bp, ptr);
    PUT()







}

void free_delete(void *ptr){

}

void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL){
      //printf("realloc 종료!\n");
      //printf("----------\n");
      return NULL;}
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    //printf("realloc 종료!\n");
    //printf("----------\n");
    return newptr;
    
}

