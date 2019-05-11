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
    "",
    /* Second member's email address (leave blank if none) */
    ""
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
int mm_init(void) {
    size_t *p = mem_sbrk(8);
    *p = 0;
    return 0;
}

/* mm_malloc
 * description */
void *mm_malloc(size_t size) {
    size_t s = A(size) + 8;
    char  *p = mem_heap_lo();
    while ( C(p) != 0 ) {
        char *np = (char *) C(p);
        np = np + 4;
        if (C(np) >= s) {
            if (C(np) >= 2*s) {
                char *ep = np + s;
                C(ep) = C(np) - s;
                mm_free(ep + 4);
                C(np) = s;
            }
            np = np - 4;
            C(p) = C(np);
            return np + 8;
        }
        p = np - 4;
    }
    p = mem_sbrk(s);
    if (p == (void *)-1) return NULL;
    p = p + 4;
    C(p) = s;
    return p + 4;
}

/* mm_free
 * description */
void mm_free(void *vp) {
    char *p = vp;
    p = p - 8;
    char *b = mem_heap_lo();
    C(p) = C(b);
    C(b) = (size_t)p;
}

/* mm_realloc
 * description */
void *mm_realloc(void *vp, size_t size) {
    char *p = vp;
    size_t s = A(size) + 8;
    p = p - 4;
    size_t t = C(p);
    if (t >= s) return vp;
    char *np = mm_malloc(size);
    if (np == NULL) return NULL;
    memcpy(np, vp, t-8);
    np = np - 4;
    C(np) = s;
    mm_free(vp);
    return np + 4;
}

