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
/* header size */
#define HS (A(sizeof(size_t)))
/* footer size */
#define FS (A(sizeof(size_t)))
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
    size_t s = A(size) + HS + FS;
    char *ch = mem_heap_lo();
    char *hi = mem_heap_hi();
    while ( ch < hi ) {
        if ( !(C(ch) & 1) && (C(ch) >= s) ) { // if free and big enough
            size_t fs = C(ch); // free block size
            size_t diff = fs - s;
            if (diff >= 64) { // split
                char *nh = ch + s;
                char *cf = ch + s - FS;
                char *nf = ch + fs - FS;
                C(ch) = s | 1;
                C(cf) = s | 1;
                C(nh) = diff;
                C(nf) = diff;
            } else { // dont split
                char *cf = ch + fs - FS;
                C(ch) |= 1;
                C(cf) |= 1;
            }
            return ch + HS;
        }
        ch += C(ch) & ~1;
    }
    ch = mem_sbrk(s);
    if (ch == (char *)-1) return NULL;
    char *cf = ch + s - FS;
    C(ch) = s | 1;
    C(cf) = s | 1;
    return ch + HS;
}
/* mm_free
 * description */
void mm_free(void *vp)
{
    char *lo = (char *) mem_heap_lo();
    char *hi = (char *) mem_heap_hi();
    char *ch = (char *) vp - HS;

    size_t s = C(ch) & ~1;
    char *cf = ch + s - FS;
    char *pf = ch - FS;
    char *nh = ch + s;

    C(ch) &= ~1;
    C(cf) &= ~1;

    if ((nh < hi) && (!(C(nh) & 1))) {
        char *nf = nh + C(nh) - HS;
        C(ch) += C(nh);
        C(nf) += C(cf);
    }

    if ((ch > lo) && (!(C(pf) & 1))) {
        char *ph =  ch - C(pf);
        C(ph) += C(ch);
        C(cf) += C(pf);
    }

}

/* mm_realloc
 * description */
void *mm_realloc(void *vp, size_t size)
{
    char *ch = (char *) vp - HS;
    size_t s = A(size) + HS + FS;
    size_t t = C(ch) & ~1;
    if (t >= s) return vp;

    char *nh = ch + t;
    char *hi = mem_heap_hi();

    printf("%d\n", hi-nh);
    if (nh > hi) {
        mm_malloc(s-t); // extend
        char *cf = ch + s - FS;
        C(cf) = s | 1;
        C(ch) = s | 1;
        return vp;
    }

    ch = mm_malloc(s);
    if (ch == NULL) return NULL;

    char *cf = ch + s - FS;
    C(ch) = s | 1;
    C(cf) = s | 1;

    memcpy(ch+HS, vp, t-HS-FS);
    mm_free(vp);

    return ch+HS;
}

