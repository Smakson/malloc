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
    char *p  = mem_heap_lo();
    char *hp = mem_heap_hi();
    while ( p < hp ) {
        if ( !(C(p) & 1) && (C(p) >= s) ) { // if free and big enough
            size_t fbs = C(p); // free block size
            size_t diff = fbs - s;
            if (diff >= 64) { // split
                C(p) = s | 1;
                char *nph = p + s;
                char *pf  = np - FS;
                C(pf) = s | 1;
                C(np) = diff;
                char *npft = p + fbs - FS;
                C(npft) = diff;
            } else { // dont split
                char *pft = p + s - FS;
                C(pft) |= 1;
                C(p) |= 1;
            }
            return p + HS;
        }
        p += (C(p) & ~1);
    }
    p = mem_sbrk(s);
    if (p == (char *)-1) return NULL;
    C(p) = s | 1;
    char *pft = p + s - FS;
    C(pft) = s | 1;
    return p + HS;
}
/* mm_free
 * description */
void mm_free(void *vp)
{   
    char *beg  = (char *) mem_heap_lo();
    char *end = (char *) mem_heap_hi();
    
    char *ch =  (char *) vp - HS;
    C(ch) &= ~1;
    char *cf = ch + C(ch) - HS;
    char *pf = ch - FS;
    char *nh = cf + FS;
    char *nf = nh + C(nh) - HS;

    if ((nf < end) && (!(C(nh) & 1))) {
        C(ch) += C(nh);
        C(nf) += C(cf);
    }

    if ( (ch != beg) && (!(C(pf) & 1))) {
        char *ph =  ch - C(pf);
        C(ph) += C(ch);
        C(cf) += C(pf);
    }

}

/* mm_realloc
 * description */
void *mm_realloc(void *vp, size_t size)
{
    char *p = (char *) vp - HS;
    size_t s = A(size) + HS + FS;
    size_t t = C(p) & ~1;
    if (t >= s) return vp;
    void *np = mm_malloc(size);
    if (np == NULL) return NULL;
    memcpy(np, vp, t-HS-FS);
    mm_free(vp);
    return np;
}

