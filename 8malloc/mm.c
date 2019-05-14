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
/* lilboi or bigboi */

/* mm_init
 * description */
int mm_init(void) {
    // init first pointer
    char *p = mem_sbrk(HS);
    P(p) = NULL;
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
            if ( diff >= 16 + HS + FS ) { // split block
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
                // will be added to linked list
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
    // nothing founded, go get more mem
    if (s <= 64 + HS + FS) {
        size_t t = s * 4;
        ch = mem_sbrk(t);
        if (ch == (char *)-1) return NULL;

        char *cf = ch + s - FS;
        size_t diff = t - s;
        char *nh = ch + s;
        char *nf = ch + t - FS;
        C(ch) = s | 1;
        C(cf) = s | 1;
        C(nh) = diff | 1;
        C(nf) = diff | 1;
        mm_free(nh + HS);
    } else {
        ch = mem_sbrk(s);
        if (ch == (char *)-1) return NULL;
        // allocate
        char *cf = ch + s - FS;
        C(ch) = s | 1;
        C(cf) = s | 1;
    }
    return ch + HS;
}

/* mm_free
 * description */
void mm_free(void *vp) {
    char *ch = (char *) vp - HS;
    char *lo = mem_heap_lo();
    size_t s = C(ch) & ~1;

    // prev in mem
    char *pmf = ch - FS;
    if ( lo != pmf && !(C(pmf)&1)) {
        char *pmh = ch - C(pmf);

        char *nfh = P(pmh);
        char *pfh = P(pmf);

        P(pfh) = nfh;

        if (nfh) {
            char *nff = nfh + C(nfh) - FS;
            P(nff) = pfh;
        }        
        s += C(pmh);        
        ch = pmh;
    }

    // next in mem
    char *hi = mem_heap_hi();
    char *nmh = ch + s;
    if ( nmh < hi && !(C(nmh)&1)) {
        char *nmf = nmh + C(nmh) - FS;
        char *nfh = P(nmh);
        char *pfh = P(nmf);

        P(pfh) = nfh;

        if (nfh) {
            char *nff = nfh + C(nfh) - FS;
            P(nff) = pfh;
        }        
        s += C(nmh);        
    }

    char *cf = ch + s - FS;
    char *nh = P(lo);
    P(ch) = nh;
    if (nh) {
        char *nf = nh + C(nh) - FS;
        P(nf) = ch;
    }
    P(cf) = lo;
    P(lo) = ch;

    C(ch) = s;
    C(cf) = s;
}

/* mm_realloc
 * description */
void *mm_realloc(void *vp, size_t size) {
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


