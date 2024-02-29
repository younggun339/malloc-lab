#include <stdio.h>

extern int mm_init (void);
extern void *mm_malloc (size_t size);
extern void mm_free (void *ptr);
extern void *mm_realloc(void *ptr, size_t size);


/* 
 * Students work in teams of one or two.  Teams enter their team name, 
 * personal names and login IDs in a struct of this
 * type in their bits.c file.
 */
typedef struct {
    char *teamname; /* ID1+ID2 or ID1 */
    char *name1;    /* full name of first member */
    char *id1;      /* login ID of first member */
    char *name2;    /* full name of second member (if any) */
    char *id2;      /* login ID of second member */
} team_t;

extern team_t team;

#define WSIZE	4	// word와 헤더, 풋터 사이즈.
#define DSIZE	8	 // 더블 워드 사이즈.
#define CHUNKSIZE (1<<12)	// 초기 가용 블록과 힙 확장을 위한 기본 크기.

#define MAX(x, y) ((x) >(y)? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc)) 
// 크기와 할당 비트를 통합. 헤더와 풋터에 저장할 수 있는 값을 리턴함.

#define GET(p)	(*(unsigned int *)(p)) // 인자 p가 참조하는 워드를 읽어서 리턴. 
// p가 가리키는 메모리 위치에서 4바이트 워드를 읽어서 리턴.
#define PUT(p, val) (*(unsigned int *)(p) = (val)) 
// 인자 p가 가리키는 워드에 val을 저장.

#define GET_SIZE(p)	(GET(p) & ~0x7) // 주소 p에 있는 헤더 또는 풋터의 size를 리턴.
#define GET_ALLOC(p) (GET(p) & 0x1) // 주소 p에 있는 헤더 또는 풋터의 할당 비트를 리턴.

// bp는 블록 포인터.
#define HDRP(bp)	((char *)(bp) - WSIZE) // 헤더를 가리키는 포인터를 리턴.
#define FTRP(bp) 	((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) 
// 풋터를 가리키는 포인터를 리턴.

#define NEXT_BLKP(bp)	((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) 
// 다음 블록 포인터를 리턴.
#define PREV_BLKP(bp) 	((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) 
// 이전 블록 포인터를 리턴.


// #define PREV_FREE(bp)    GET(((char *)(bp) + WSIZE))    //이어진 이전 free 리스트의 주소를 가져오기
#define NEXT_FREE(bp)    GET(((char *)(bp) + WSIZE)) //이어진 이후 free 리스트 주소를 가져오기.

#define PUT_ADDR(p, val) (*(unsigned int *)(p) = (unsigned int)(val))
#define GET_ADDR(p) (*(unsigned int *)((void *)(p)))



