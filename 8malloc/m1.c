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
    "ep.pink",
    /* First member's full name */
    "Matthias Hasler",
    /* First member's email address */
    "matthias.hasler@polytechnique.edu",
    /* Second member's full name (leave blank if none) */
    "Miha Smaka",
    /* Second member's email address (leave blank if none) */
    "miha.smaka@polytechnique.edu"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define A(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
/* size of flag */
#define SoF (A(sizeof(size_t)))
/* cast to size_t */
#define C(p) (*(size_t *)p)

/* mm_init
 * description */
int mm_init(void)
{
    return 0;
}

/* mm_malloc
 * description */
void *mm_malloc(size_t size) {
    size_t s = A(size) + SoF;
    char *p  = (char *) mem_heap_lo();
    char *hp = (char *) mem_heap_hi();
    while ( p < hp ) { //inside the heap
        if ( !(C(p) & 1) ) { //checking if its free
            while ( C(p) < hp-p ) { //checking that we are not outside 
                char *np = p + C(p);
                if (C(np) & 1) break;
                C(p) += C(np);
            }
            
            if ( C(p) >= s ) {
                C(p) |= 1;
                return (void *)(p + SoF);
            }
        }
        p += C(p) & ~1;
    }
    p = mem_sbrk(s);
    if (p == (void *)-1) return NULL;
    C(p) = s | 1;
    return (void *)(p + SoF);
}

/* mm_free
 * description */
void mm_free(void *vp)
{
    char *p = (char *) vp - SoF;
    C(p) &= ~1;
}

/* mm_realloc
 * description */
void *mm_realloc(void *vp, size_t size)
{
    char *p = (char *) vp - SoF;
    size_t s = A(size) + SoF;
    size_t t = C(p) & ~1;
    if (t >= s) return vp;
    void *np = mm_malloc(size);
    if (np == NULL) return NULL;
    C(p) &= ~1;
    memcpy(np, vp, t-SoF);
    return np;
}

