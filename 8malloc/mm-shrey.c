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


The solution makes use of segregated free lists.
The size of classes in the list is in powers of 2.
The implementation is outlined in the format given in the book.
There are relevant macros and static functions that allow us to implement
malloc, realloc and free.
For freeing we made use of the LIFO policy instead of the addressed based policy.
For malloc we used the find_fit function which was described in the book.
We also tried to check if it possible to coalese with the right block to find a
sufficiently large block that would comply with the realloc request.
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
     "P.E.S.P. (Pandas Eat Sweet Potato)",
     /* First member's full name */
     "Kayo Yin",
     /* First member's email address */
     "kayo.yin@polytechnique.edu",
     /* Second member's full name (leave blank if none) */
     "Shrey Aryan",
     /* Second member's email address (leave blank if none) */
     "shrey.aryan@polytechnique.edu"
 };
/* alignment  must be equal to integer size */
#define ALIGNMENT 16

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0xf)

// Basic constants and macros used in the free list implementation
#define WSIZE       4           /* word size (bytes) */
#define DSIZE       8            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)       /* initial heap size (bytes) */
#define OVERHEAD    DSIZE        /* overhead of header and footer (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

// Pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))

// Read and write a word at address p
#define GET(p)            (*(unsigned int *)(p))
#define PUT(p, val)       (*(unsigned int *)(p) = (val))

// Read the size and allocated fields from address p
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

// Given block bp bp, compute address of its header and footer
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

//  Given block bp bp, compute address of next and previous blocks
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

// read address of next and previous blocks
#define FWD(bp) GET(bp)
#define BACK(bp) GET((char *)(bp)+WSIZE)

// write address at next and previous block
#define FWD_LINK(bp, val) PUT(bp, val)
#define BACK_LINK(bp, val) PUT((char *)(bp)+WSIZE, val)

// length of segregated list
#define LEN 20
#define GET_LIST(i)   (*(void **)(free_lists + (i*DSIZE)))
#define SET_LIST(i, ptr) ((GET_LIST(i)) = ptr)

// End of macros


/* Global variables */
void *heap_listp = NULL;  /* Pointer to first block */
void* free_lists;         /* Pointer to first block of the segregated list*/


// Helper Functions
static void *extend_heap(size_t size);
static void place(void *bp, size_t asize, int flag);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void add_to_free_lists(void *bp);
static void remove_from_list(void *bp);
static void free_with_lifo_policy(void *bp);
static int mm_check(void);
/*
 * mm_init :  initialize free_lists ,i.e. segregated list pointers
              initialize the heap
 */
int mm_init(void)
{
  free_lists = NULL;
  heap_listp = NULL;
  // We put our free_lists on the heap
  if ((free_lists = mem_sbrk(LEN*DSIZE)) == (void *)-1)
      return -1;

  /* Create the initial empty heap */
  if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
    return -1;

    // initialize free_lists
    int i;
    for(i = 0; i  < LEN; i++)
        SET_LIST(i, NULL);

    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += DSIZE;

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
   return -1;
return 0;
}

/* $end mminit */

 /*
  * mm_malloc - Allocate a block with size which is a multiple of 8
  *             by incrementing the brk pointer
  */
 /* $begin mmmalloc */
 void *mm_malloc(size_t size)
 {

     size_t asize;      /* Adjusted block size */
     size_t extendsize; /* Amount to extend heap if no fit */
     void * bp = NULL;

     if (heap_listp == 0)
       mm_init();

     /* Ignore spurious requests */
     if (size == 0)
       return NULL;

     /* Adjust block size to include overhead and alignment reqs. */
     if( size <= DSIZE)
         asize = 2*DSIZE;
     else
        asize =ALIGN(size + DSIZE);

     if ((bp = find_fit(asize)) != NULL)
       {
           place(bp, asize, 0);
           return bp;
       }

     /* No fit found. Get more memory and place the block */
     extendsize = MAX(asize,CHUNKSIZE);
     if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
         return NULL;
     place(bp,asize, 1);

     // Test heap consistency after malloc
     /*
          if (mm_check() == 0)
           {
             printf("Failure!\n");
             exit(0);
           }
     */

     return bp;
   }


/*
 * mm_free - Free a block
 */
 /* $begin mmfree */
void mm_free(void *bp)
{
    if(bp == 0)
      return;

    size_t size= GET_SIZE(HDRP(bp));

    if(heap_listp == 0)
      {
        mm_init();
      }

    // get rid of allocated tags from the header and footer
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));

    free_with_lifo_policy(bp);


    /*
         if (mm_check() == 0)
          {
            printf("Failure!\n");
            exit(0);
          }
    */
}

/* $end mmfree */

/*
 * mm_realloc - Almost naive implementation of realloc in terms of mm_free and
                mm_malloc.
 */
  /* $begin mm_realloc */
  void *mm_realloc(void *ptr, size_t size)
  {
      size_t oldsize;
      void *newptr;

      /* If size == 0 then this is just free, and we return NULL. */
      if (size == 0){
          mm_free(ptr);
          return NULL;
      }

      /* If oldptr is NULL, then this is just malloc. */
      if(ptr == NULL) 
	{
      return mm_malloc(size);
      }


      // Use the same block for realloc if it has the appropriate size
      if (size < GET_SIZE(HDRP(ptr))-2*WSIZE)
      {
          return ptr;
      }

      void *next = NEXT_BLKP(ptr);
      int next_alloc = GET_ALLOC(HDRP(next));
      size_t next_size = GET_SIZE(HDRP(next));

      // see if the next block can be coalesced with the current block
      size_t  adjust_size = (GET_SIZE(HDRP(next)) + GET_SIZE(HDRP(ptr)));
      if (!next_alloc && size <= adjust_size-2*WSIZE)
      {
          remove_from_list(next);
          PUT(HDRP(ptr), PACK(adjust_size, 1));
          PUT(FTRP(ptr), PACK(adjust_size, 1));
          return ptr;
      }


      // we could also try extending the heap
      if (!next_alloc && !next_size)
        {
          size_t remainder = GET_SIZE(HDRP(ptr)) + next_size - size;
          size_t extendsize;
          if (remainder < 0)
            {
              extendsize = MAX(-remainder, CHUNKSIZE);
              if (extend_heap(extendsize) == NULL)
                  return NULL;
              remainder += extendsize;
            }

          remove_from_list(next);
          PUT(HDRP(ptr), PACK(GET_SIZE(HDRP(ptr)) + remainder, 1));
          PUT(FTRP(ptr), PACK(GET_SIZE(HDRP(ptr)) + remainder, 1));
          return ptr;
        }
      
      newptr = mm_malloc(size);

      /* If realloc() fails the original block is left untouched  */
      if(!newptr)
      {
  	     return 0;
      }

      /* Copy the old data. */
      oldsize = GET_SIZE(HDRP(ptr));
      if (size < oldsize)
          oldsize = size;
      memcpy(newptr, ptr, oldsize);
      mm_free(ptr);


      /*
           if (mm_check() == 0)
            {
              printf("Failure!\n");
              exit(0);
            }
      */
      return newptr;
  }
 /* $end mm_realloc */

/*
 * The remaining routines are internal helper routines
 */

/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin mmextendheap */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

  return bp;
}
/* $end mmextendheap */

int get_index(size_t size)
  {
    int i = 0;
    while((i < LEN - 1) && (size > 1))
    {
      size = size >> 1;
      i ++;
    }
    return i;
    }

/*
 * add_to_free_lists: we add the free block to a list in the free_lists such that
                      it remains in ascending order of block size.
 */
/* $begin add_to_free_lists */
void add_to_free_lists(void *bp)
{
  // We need to add a free block to our segregated list
  // so we first determine the index (i) of the list inside which
  // we will insert the free block. This index will be determined
  // using the size.
  int size = GET_SIZE(HDRP(bp));
  int i = get_index(size);
  // pointer to our explicit list
  void *current = GET_LIST(i);
  // if the list is empty
  if (current == NULL)
    {
      // add bp to the list by setting its next block
      // and previous block as NULL
      FWD_LINK(bp, NULL);
      BACK_LINK(bp, NULL);
    }
  // else the list is not empty
  else
    {
      // add bp to the list by setting its next block
      // as the current block and its previous block as
      // NULL
      FWD_LINK(bp, current);
      BACK_LINK(bp, NULL);
      // finally set the previous block of the current block as
      // bp since bp is added at the bottom of the queue
      BACK_LINK(current, bp);
    }
  // update the list pointer, which points to the recently added
  // free block, that is, bp
  SET_LIST(i, bp);
  return;
}

/* $end add_to_free_lists */

/*
 * remove_from_list:  remove the free block from the appropriate
                      list inside free_lists.
 */
/* $begin remove_from_list */
static void remove_from_list(void * bp)
{
  // We need to add a free block to our segregated list
  // so we first determine the index (i) of the list inside which
  // we will insert the free block. This index will be determined
  // using the size.
  int size = GET_SIZE(HDRP(bp));
  int i = get_index(size);

  void *previous = BACK(bp);
  void *next =  FWD(bp);

  // We have a situation like this:
  //  next  <---  bp  <--- previous

  // Case 1:  the pointer bp is somewhere in the middle of the current_list
  //          next  <---  bp  <--- previous
  if (BACK(bp) != NULL && FWD(bp) != NULL)
    {
      FWD_LINK(BACK(bp), FWD(bp));
      BACK_LINK(FWD(bp), BACK(bp));
    }

    // Case 2:  the pointer bp is at the beginning of the current_list
    //          bp  <--- previous
    else if (BACK(bp) != NULL && FWD(bp) == NULL)
      {
        FWD_LINK(BACK(bp), NULL);
      }

    // Case 3:  the pointer bp is at the end of the current_list
    //          next  <---  bp
    else if (BACK(bp) == NULL && FWD(bp) != NULL)
      {
          SET_LIST(i, FWD(bp));
          BACK_LINK(FWD(bp), NULL);

      }

    // Case 4:  removing the pointer bp makes the current_list empty
    //          and so set the current_list pointer to NULL.
    //          NULL  <---  bp  <--- NULL

    else
      {
          SET_LIST(i, NULL);

      }

}

/* $end remove_from_list */


/*
 * coalesce - Boundary tag coalescing. Return bp to coalesced block.
 */
/* $begin coalesce */
void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    void* result = NULL;

    if (prev_alloc && next_alloc) {       /* Case 1 */
        result =  bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        result = bp;
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        result = PREV_BLKP(bp);
    }

    else {            /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
            GET_SIZE(FTRP(NEXT_BLKP(bp)))  ;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
        result = PREV_BLKP(bp);
    }

    return result;
}
/* $end coalesce */

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
/* $begin mmplace */
static void place(void *bp, size_t asize, int flag)
{
    size_t size = GET_SIZE(HDRP(bp));
    size_t remain = size - asize;
    void* dummy = NULL;

    // if the block size is more than required size then
    // we split, however we just need to consider whether
    // we extended the heap or not.
      if (remain >= 2*DSIZE)
        {
          // if the heap was not extended then we find_fit
          // worked and so bp is no more a free block
          if (flag == 0)
            {
              remove_from_list(bp);
            }

              PUT(HDRP(bp), PACK(asize, 1));
              PUT(FTRP(bp), PACK(asize, 1));
              dummy = NEXT_BLKP(bp);
              PUT(HDRP(dummy), PACK(remain, 0));
              PUT(FTRP(dummy), PACK(remain, 0));
              // dummy is the new free block
              add_to_free_lists(dummy);

        }
      else
        {
          PUT(HDRP(bp), PACK(size, 1));
          PUT(FTRP(bp), PACK(size, 1));
          // if the heap was not extended then bp was allocated
          // and so it is not a free block anymore
          if (flag == 0)
            {
              remove_from_list(bp);
            }

        }

}
/*
 * find_fit - Find a fit for a block with asize bytes
              Method used: FIRST FIT
 */
/* $begin find_fit */
static void *find_fit(size_t asize)
{
  int i;
  // traverse the segregated list starting from the possible class size
  // until the end
  for (i = get_index(asize); i < LEN; i++)
    {
        // next iterate over the free list at index i
        // to find free block
        void *bp = GET_LIST(i);
        while (bp != NULL)
          {
              size_t curr_block_size = GET_SIZE(HDRP(bp));
              if (!GET_ALLOC(HDRP(bp)) && (asize <= curr_block_size))
                {
                  return bp;
                }
              bp  = FWD(bp);
          }
    }
  // Nothing found
  return NULL;
}
/* $end find_fit */

static void free_with_lifo_policy(void *bp)
{
  // We free with	a	LIFO Policy described in lecture
  // https://moodle.polytechnique.fr/pluginfile.php/130637/mod_resource/content/7/lec10-alloc.pdf

  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

  // Case 1:  If both the previous and the next block
  //          are allocated then cannot coalese.
  if (prev_alloc && next_alloc)
  {
      add_to_free_lists(bp);
      return;
  }

  // Case 2:  If the previous block is allocated and the
  //          the next block is not allocated then we can
  //          coalese the current block with the next block.
  else if (prev_alloc && !next_alloc)
  {
      remove_from_list(NEXT_BLKP(bp));

  }

  // Case 3:  If the previous block is not allocated and the
  //          the next block is allocated then we can
  //          coalese the current block with the previous block.

  else if (!prev_alloc && next_alloc)
  {
      remove_from_list(PREV_BLKP(bp));

  }

  // Case 4:  If the previous block is unallocated and the
  //          the next block is unallocated then we can
  //          coalese all 3 blocks.
  else
  {
      remove_from_list(PREV_BLKP(bp));
      remove_from_list(NEXT_BLKP(bp));
  }


  bp = coalesce(bp);
  add_to_free_lists(bp);
}




/*
 * mm_check - check of the heap for consistency

  Points to check:
      Is every block in the free list marked as free?
      Do the free blocks in the segregated list point to valid heap addresses?
      Do any allocated blocks overlap?
      Do the pointers in a heap block point to valid heap addresses?

mm_check returns 0 if the heap is inconsistent
else it returns 1

 */
static int mm_check(void)
{

  //  The following code snippet addresses the following questions.
  //      Is every free block actually in the free list?
  //      Do the free blocks in the segregated list point to valid heap addresses?
  int i;
  // iterate of over the segregated list
  for(i = 0; i < LEN; i++)
  {
      void *bp = GET_LIST(i);
      // iterate over the free list
      while (bp)
      {
          //  Is every block in the free list marked as free?
          if (GET_ALLOC(HDRP(bp)) == 1 || GET_ALLOC(FTRP(bp)) == 1)
          {
            printf ("Free blocks not marked as free\n");
            return 0;
          }
          //   Do the free blocks in the segregated list point to valid heap addresses?
          if ((heap_listp > bp) | (bp > mem_heap_hi()))
          {
                printf ("Free blocks pointing out of heap\n");
                return 0;
          }
            bp  = FWD(bp);
        }
    }

  // Do any allocated blocks overlap?
  void *ptr = heap_listp;
  // Iterate over the heap
  while(ptr != NULL && GET_SIZE(HDRP(ptr))!=0)
    {
        // If it is allocated
        if(GET_ALLOC(HDRP(ptr)))
        {
         void *next = NEXT_BLKP(ptr);
         // detect an overlap
         if (ptr + GET_SIZE(HDRP(ptr)) - WSIZE >= next)
            {
              printf("Allocated blocks overlap; %p\n",ptr);
              return 0;
            }
          }

     ptr = NEXT_BLKP(ptr);
 }

  // Do the pointers in a heap block point to valid heap addresses?
  void* start = mem_heap_lo();
  void* end = mem_heap_hi();
  ptr = start;
  while(GET_SIZE(HDRP(ptr))>0)
    {
        if(ptr > end || ptr < start)
        {
          printf("Pointers do not point to a valid heap address \n");
          return 0;
        }

        ptr = NEXT_BLKP(ptr);
    }

    return 1;
}

