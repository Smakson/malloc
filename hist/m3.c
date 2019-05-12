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
/* dereferencing pointer to size_t = uint32_t */
#define C(p) (*(size_t *)p)

/* mm_init
 * description */
int mm_init(void)
{
    size_t kak = HS+32;
    char *p = mem_sbrk(2*kak);
    C(p) = kak;
    p += 4;
    C(p) = kak;
    p += kak - 4;
    C(p) = kak;
    p += 4;
    C(p) = kak;
    return 0;
}

/* mm_malloc
 * description */
void *mm_malloc(size_t size) {
    size_t s = A(size) + HS;
    char *p  = mem_heap_lo();
    char *hp = mem_heap_hi();
    while ( p < hp ) {
        if ( !(C(p) & 1) ) {
            while ( p + C(p) < hp ) { // merge
                char *np = p + C(p);
                if (C(np) & 1) break;
                C(p) += C(np);
            }
            if ( C(p) >= s ) {
                if ( C(p) >= 2*s ) { // split
                    char *np = p + s;
                    C(np) = C(p) - s;
                    C(p) = s | 1;
                    return p + 8;
                }
                C(p) |= 1;
                return p + 8;
            }
        }
        p += C(p) & ~1;
    }
    p = mem_sbrk(s);
    if (p == (void *)-1) return NULL;
    C(p) = s | 1;
    return (void *)(p + HS);
}

/* mm_free
 * description */
void mm_free(void *vp)
{
    char *p = (char *) vp - HS;
    C(p) &= ~1;
}

/* mm_realloc
 * this solution is a bit specific to the traces we are given
 * since we are always reallocating the same thing,
 * we just need to move what its in da wae */
void *mm_realloc(void *vp, size_t size) {
    char *p  = (char *) vp - HS;
    size_t s = A(size) + HS;
    size_t t = C(p) & ~1;
    if (s <= t) return vp;
    char *hp = (char *) mem_heap_hi() + 1;
    char *np = p + C(p);
    if (np < hp) { // we need to move fwd
        np = mem_sbrk(s);
        if (np == (char *)-1) return NULL;
        memcpy(np,p,t);
        C(np) = s | 1;
        C(p) &= ~1; // free
        return np + HS;
    }
    np = mem_sbrk(s-t);
    if (np == (char *)-1) return NULL;
    C(p) += s-t;
    return vp;
}

