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
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    /* Implicit Free List - First fit */
    // 8의 배수로 재설정한다.
    int newsize = ALIGN(size + SIZE_T_SIZE); // size(payload) + SIZE_T_SIZE(header)
    
    // find available freed block
    void *p = mem_heap_lo();
    // 끝에 도달할 때 까지, freed block이면서 해당 블록이 할당될 사이즈보다 크거나 같을때까지 반복
    while(p <= mem_heap_hi() && ((*(size_t *)p & 1) || (*(size_t *)p & ~0x7) < (size_t)newsize)) {
        p = (void *)((char *)p + (*(size_t *)p & ~0x7)); // block jump
    }
    
    if(p > mem_heap_hi()) {
        // 가용 가능한 블럭을 못 찾으면
        p = mem_sbrk(newsize); // mem_brk을 newsize만큼 increase시킨다. 시작 지점 반환
        if (p == (void *)-1)
            return NULL;
        else
        {
            *(size_t *)p = newsize | 1; // size를 넣는다. | allocated flag를 넣는다. (size는 8의 배수라 flag와 겹치지 않음을 이용)
            return (void *)((char *)p + SIZE_T_SIZE); // header를 점프한(8byte jump) 지점을 반환
        }
    } else {
        size_t prev_size = *(size_t *)p & ~0x7; // freed 블록 사이즈 계산
        *(size_t *)p = newsize | 1; // size를 넣는다. | allocated flag를 넣는다. (size는 8의 배수라 flag와 겹치지 않음을 이용)
        if(prev_size > newsize) {
            void* next_p = (char *)p + newsize;
            *(size_t *)next_p = (prev_size - newsize);
        }
        return (void *)((char *)p + SIZE_T_SIZE); // header를 점프한(8byte jump) 지점을 반환
    }  
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    if(ptr == NULL) return;
    ptr = ((char*)ptr - SIZE_T_SIZE); // 포인터를 뒤로 가서 header를 가리키기

    size_t size = *(size_t *)ptr = *(size_t *)ptr & ~1; // flag 해제

    void* nextPtr = (char *)ptr + size; // 다음 블록 시작 지점

    if(nextPtr > mem_heap_hi()) return;

    if((*(size_t *)nextPtr & 1) == 0) {

        size_t nextSize = *(size_t *)nextPtr;
        *(size_t *)ptr = (size + nextSize);
    }
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;


    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE) & ~0x7;
    if (size < copySize)
        copySize = size;
    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}