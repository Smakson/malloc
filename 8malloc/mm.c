/*
 * mm.c - explicit doubly linked list of free blocks
 *
 * strategy:
 * - for malloc, we find the first big enough free block
 *   and split it if there is enough space left
 * - for realloc, we try to have the expanding block at the very edge
 *   of the heap to allow cheap growth
 * - when expanding the heap for a small we allocate a bigger one,
 *   to avoid having small blocks between bigger ones,
 *   esp if they cannot be allocated afterwards
 *   (could be improved)
 *
 * details:
 * we have a pointer to the first block of the least
 * at the beginning of the heap.
 * each block contains a header 8 bytes and a footer 8 bytes
 * header contains next block pointer and size + (free ? 0 : 1)
 * footer contains previous block pointer and size + (free ? 0 : 1)
 * the invariant is that two free blocks are never adjacent
 * when freeing we make sure to merge with neighbors if they are free
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
    size_t SB = 64; // smallboiz
    char *p = mem_sbrk(HS + SB); // header
    char *h = p+8; // first
    P(p) = h;
    C(h) = SB;
    P(h) = NULL;
    char *f = h + SB - FS;
    P(f) = p;
    C(f) = SB;
    return 0;
}

/* mm_malloc
 * iterate over free blocks, if one free is big enough
 * split it if there is enough space left
 * or extend mem
 */
void *mm_malloc(size_t size) {
    size_t s = A(size) + HS + FS;
    char *ph = mem_heap_lo();
    char *ch = P(ph);
    char *bp = NULL;
    size_t best = -1;
    while ( ch ) {
        size_t bs = C(ch);
        if (bs>=s && bs<best) {
            bp = ch;
            best = bs;
        }
        ch = P(ch);
    }
    if ( bp ) {
        ch = bp;
        size_t bs = C(ch); // block size
        ph = ch + bs - FS;
        ph = P(ph);
        assert(ph);
        assert(P(ph)==ch);
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
        printf("lel\n");
    }
    // nothing founded, go get more mem
    if (size <= 64) {
        size_t t = s * 3;
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
        if (size < 512) {
            size = 8 * size / 7;
            s = A(size) + HS + FS;
        }
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
 * rewiring and merging
 */
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
 * dont shrink
 * if at edge, extend,
 * otherwise move to edge
 */
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

int mm_check(void) {
    char *lo = mem_heap_lo();
    char *ph = lo;
    char *hi = (char *) mem_heap_hi() + 1;
    char *ch = P(ph);
    while ( ch ) {
        // check free
        int s1 = C(ch);
        if (s1&1) return 1;
        // check same
        char *cf = ch + s1 - FS;
        int s2 = C(cf);
        if ( s2 != s1 ) return 2;
        // check back pointer
        char *p = P(cf);
        if ( p != ph ) return 3;
        // check free neighbors
        if ( ch != lo + HS ) {
            p = ch - HS;
            if (!(C(p)&1)) return 4;
        }
        if ( cf + FS != hi ) {
            p = cf + FS;
            if (!(C(p)&1)) return 5;
        }
        ph = ch;
        ch = P(ch);
    }
    return 0;
}


