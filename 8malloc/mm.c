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
#define A(size) (((size) + 7) & ~7)
/* header size */
#define HS 8
/* footer size */
#define FS 8
/* cast to size_t */
#define C(p) (*(size_t *)p)
/* cast to char* */
#define P(p) ( *(char **) ( (char *) p + 4 ) )

/* mm_init
 * description */
int mm_init(void) {
    // init first pointer
    char *p = mem_sbrk(HS);
    P(p) = NULL;
    return 0;
}

/* mm_malloc
 * description */
void *mm_malloc(size_t size) {
    size_t s = A(size) + HS + FS;
    char *ph = mem_heap_lo();
    char *ch = P(ph);
    while ( ch ) {
        size_t bs = C(ch); // block size
        assert((bs&1)==0);
        if ( bs >= s ) { // big enough
            size_t diff = bs - s;
            if ( diff >= 16 + HS + FS ) { // big enough to split block
                // allocate first half
                char *cf = ch + s - FS;
                C(ch) = s | 1;
                C(cf) = s | 1;
                // create block for second half
                char *nmh = ch + s;
                char *nmf = ch + bs - FS;
                C(nmh) = diff | 1;
                C(nmf) = diff | 1;
                // rewiring
                char *nh = P(ch);
                P(ph) = nh;
                if ( nh ) {
                    char *nf = nh + C(nh) - FS;
                    P(nf) = ph;
                }
                mm_free(nmh + HS);
            } else { // dont split
                // allocate
                char *cf = ch + bs - FS;
                C(ch) = bs | 1;
                C(cf) = bs | 1;
                // rewiring
                char *nh = P(ch);
                P(ph) = nh;
                if ( nh ) {
                    char *nf = nh + C(nh) - FS;
                    P(nf) = ph;
                }

            }
            return ch + HS;
        }
        ph = ch;
        ch = P(ch);
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
void mm_free(void *vp) {
    char *ch = (char *) vp - HS;
    char *lo = mem_heap_lo();

    size_t s = C(ch) & ~1;
    
    // prev in mem
    char *pf = ch - FS;
    if ( lo != pf && !(C(pf)&1) && 0 ) {
        char *ph = ch - C(pf);
        char *ph0 = P(pf);
        char *nh0 = P(ph);
        char *nf0 = nh0 + C(nh0) - FS;
        P(ph0) = nh0;
        if (nh0) P(nf0) = ph0;
        s += C(ph);
        ch = ph;
    }

    // next in mem
    char *hi = mem_heap_hi();
    char *nh = ch + s;
    if ( nh < hi && !(C(nh)&1) && 0 ) {
        char *nf = nh + C(nh) - FS;
        char *ph0 = P(nf);
        char *nh0 = P(nh);
        char *nf0 = nh0 + C(nh0) - FS;
        P(ph0) = nh0;
        if (nh0) P(nf0) = ph0;
        s += C(nh);
   }

    P(ch) = P(lo);
    P(lo) = ch;

    char *cf = ch + s - FS;
    C(ch) = s;
    C(cf) = s;
}

/* mm_realloc
 * description */
void *mm_realloc(void *vp, size_t size)
{
    return NULL;
    char *ch = (char *) vp - HS;
    size_t s = A(size) + HS + FS;
    size_t t = C(ch) & ~1;
    if (t >= s) return vp;

    char *nh = ch + t;
    char *hi = mem_heap_hi();

    if (nh > hi) { // forget about extending for the moment
        char *p = mem_sbrk(s-t);
        if (p == (char *)-1) return NULL;
        char *cf = ch + s - FS;
        C(ch) = s | 1;
        C(cf) = s | 1;
        return vp;
    }

    ch = mm_malloc(size);
    if (ch == NULL) return NULL;

    memcpy(ch, vp, t-HS-FS);
    mm_free(vp);

    return ch;
}

