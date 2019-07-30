/* Donald Elmore
 * Purpose: This file investigates strategies for managing a memory heap via
 *  Mem_alloc and Mem_free, without calling malloc or free.
 * Bugs: When user requests exactly one page, behavior is not consistent.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include "mem.h"

// Global variables required in mem.c only
static chunk_t Dummy = {&Dummy, 0};
static chunk_t * Rover = &Dummy;
static int NumPages = 0;
static int NumSbrkCalls = 0;

// private function prototypes
void mem_validate(void);

/* morecore
 * function to request 1 or more pageCount from the operating system.
 *
 * new_bytes must be the number of bytes that are being requested from
 *           the OS with the sbrk command.  It must be an integer 
 *           multiple of the PAGESIZE
 *
 * returns a pointer to the new memory location.  If the request for
 * new memory fails this function simply returns NULL, and assumes some
 * calling function will handle the error condition.  Since the error
 * condition is catastrophic, nothing can be done but to terminate 
 * the program.
 */
chunk_t *morecore(int new_bytes) 
{
    char *cp;
    chunk_t *new_test1;

    assert(new_bytes % PAGESIZE == 0 && new_bytes > 0);
    assert(PAGESIZE % sizeof(chunk_t) == 0);
    cp = sbrk(new_bytes);
    if (cp == (char *) -1)
        return NULL;
    new_test1 = (chunk_t *) cp;
    NumSbrkCalls++; 
    NumPages += (new_bytes / PAGESIZE);
    
    return new_test1;
}

/* Mem_free
 * deallocates the space pointed to by return_ptr; it does nothing if
 * return_ptr is NULL.  
 *
 * This function assumes that the Rover pointer has already been 
 * initialized and points to some memory block in the free list.
 */
void Mem_free(void *return_ptr)
{
    assert(Rover != NULL && Rover->next != NULL);
  
    if (return_ptr != NULL) {
        chunk_t *marker = NULL, *prevNode = NULL;
        chunk_t *dumChunk = return_ptr-sizeof(chunk_t);
        prevNode = Rover;
        Rover = Rover->next;
        marker = Rover;
        
        do { 
            if ((prevNode < dumChunk && Rover > dumChunk) || 
            (prevNode < dumChunk && Rover == &Dummy)) {
                prevNode->next = dumChunk;
                dumChunk->next = Rover;
                break;
            }
            prevNode = Rover;
            Rover = Rover->next;
        } while(Rover != marker);
        
        if (Coalescing == TRUE) {
            if (dumChunk + dumChunk->size == Rover) {
                dumChunk->next = Rover->next;
                Rover->next = NULL;
                dumChunk->size = dumChunk->size + Rover->size;
                Rover = dumChunk->next;
            }
            if (prevNode+prevNode->size == dumChunk) {
                prevNode->next = Rover;
                dumChunk->next = NULL;
                prevNode->size = prevNode->size + dumChunk->size;
            }
        }
    }
} 

/* Mem_alloc
 * returns a pointer to space for an object of size nbytes, or NULL if the
 * request cannot be satisfied.  The memory is uninitialized.
 *
 * This function assumes that there is a Rover pointer that points to
 * some item in the free list.  By default, Rover is initialized to the
 * address of the dummy block whose size is one, but set the size field
 * to zero so this block can never be removed from the list.  Rover can
 * never be null.
 */
void *Mem_alloc(const int nbytes)
{
    assert(nbytes > 0);
    assert(Rover != NULL && Rover->next != NULL);

    int pageCount = 0, unitNum = 0, space = 0;

    chunk_t *marker, *next, *prev, *bestFit, *test1, *test2;

    //Determine unitNum and pageCount
    if (nbytes < PAGESIZE) {
        pageCount = 1;
    }
    else if (nbytes % PAGESIZE != 0) {      //nbytes > PAGESIZE
        pageCount = (nbytes / PAGESIZE) + 1;    //if nbytes is not a mult of PAGESIZE, extra page
    }
    else {
        pageCount = nbytes / PAGESIZE;
    }

    if (nbytes % sizeof(chunk_t) != 0) {       //if nbytes is not evenly div. by sizeof(chunk_t)
        unitNum = (nbytes/sizeof(chunk_t))+1;    //   provide number of chunk_t's + 1
    }
    else {
        unitNum = nbytes/sizeof(chunk_t);        //if nbytes is evenly divis. by sizeof(chunk_t)
    }                                          //   return quotient

    if (Dummy.next == &Dummy) {                //if list is empty
        Rover = morecore(pageCount * PAGESIZE);    //then move rover to new memory block
        
        if (Rover == NULL)
            return NULL;
            
        Dummy.next = Rover;
        Rover->size = unitNum + 1;
        
        //check if user wants exactly one page
        if (((pageCount * PAGESIZE) / sizeof(chunk_t)) - (unitNum + 1) == 0) {
            Rover->next = NULL;
            return (Rover + 1);
        }
        
        Rover += unitNum + 1;         //add to rover unitNum including header
        Rover->next = &Dummy;
        Rover->size = ((pageCount * PAGESIZE) / sizeof(chunk_t)) - (unitNum + 1);
        Dummy.next = Rover;
        (Rover - unitNum - 1)->next = NULL;
        test1 = Rover - unitNum - 1;
        test2 = Rover - unitNum;
        
        //assertion checks
        assert(test1 + 1 == test2);  
        assert((test1->size - 1)*sizeof(chunk_t) >= nbytes);
        assert((test1->size - 1)*sizeof(chunk_t) <= nbytes + sizeof(chunk_t));
        assert(test1->next == NULL);

        return Rover - unitNum;
    }
    else if (SearchPolicy == FIRST_FIT) {       //list is not empty & search policy is first fit
        marker = Rover;                           //  marker at current rover
        do {
            if (Rover->size == unitNum + 1) {     //if block has exact space
                marker = Rover;
                Rover = Rover->next;
                prev = &Dummy;

                while (prev->next != marker)    //find prevNode memory block
                    prev = prev->next;

                prev->next = Rover;
                marker->next = NULL;

                test1 = marker;
                test2 = marker + 1;

                //assertion checks
                assert(test1 + 1 == test2);
                assert((test1->size - 1)*sizeof(chunk_t) >= nbytes);
                assert((test1->size - 1)*sizeof(chunk_t) <= nbytes + sizeof(chunk_t));
                assert(test1->next == NULL);

                return (marker + 1);
            }
            else if (Rover->size > unitNum + 1) {     //if block has more than enough space
                marker = Rover;
                long remainder = Rover->size;
                next = Rover + unitNum + 1;
                Rover->size = unitNum + 1;
                prev = &Dummy;
                
                while (prev->next != marker)      //find prev block of mem
                    prev = prev->next;

                prev->next = next;    
                Rover += unitNum + 1;
                Rover->next = marker->next;
                Rover->size = remainder - (unitNum + 1);
                (Rover - unitNum - 1)->next = NULL;

                test1 = Rover - unitNum - 1;
                test2 = Rover - unitNum;

                //assertion checks
                assert(test1 + 1 == test2);  
                assert((test1->size - 1) * sizeof(chunk_t) >= nbytes);
                assert((test1->size - 1) * sizeof(chunk_t) <= nbytes + sizeof(chunk_t));
                assert(test1->next == NULL);

                return (Rover - unitNum);
            }

            Rover = Rover->next;

        } while (Rover != marker);
    }
    else if (SearchPolicy == BEST_FIT) {
        marker = Rover;
        do {
            if (Rover->size == unitNum + 1) { //if exact space needed
                marker = Rover;
                Rover = Rover->next;
                prev = &Dummy;

                while (prev->next != marker)   //find prev mem block
                    prev = prev->next;

                prev->next = Rover; //set prevNode next to rover
                marker->next = NULL;

                test1 = marker;
                test2 = marker + 1;
                
                //assertion checks
                assert(test1 + 1 == test2); 
                assert((test1->size - 1)*sizeof(chunk_t) >= nbytes);
                assert((test1->size - 1)*sizeof(chunk_t) <= nbytes + sizeof(chunk_t));
                assert(test1->next == NULL);

                return (marker + 1);
            }
            else if (Rover->size > unitNum + 1) { //if not exact space, find best fit
                marker = Rover;
                bestFit = Rover;
                space = bestFit->size - (unitNum + 1);
                prev = &Dummy;

                while (prev->next != marker) {
                    prev = prev->next;
                    if ((prev->size - (unitNum + 1)) < space) {
                        bestFit = prev;
                        space = bestFit->size - (unitNum + 1);
                    }
                }

                next = bestFit->next;

                while (prev->next != bestFit)      //find prev mem block
                    prev = prev->next;

                bestFit += unitNum + 1;
                prev->next = bestFit;
                bestFit->next = next;
                bestFit->size = space;
                (bestFit - unitNum - 1)->size = unitNum + 1;
                (bestFit - unitNum - 1)->next = NULL;
                                
                test1 = bestFit - unitNum - 1;
                test2 = bestFit - unitNum;

                //assertion checks
                assert(test1 + 1 == test2);   
                assert((test1->size - 1) * sizeof(chunk_t) >= nbytes);
                assert((test1->size - 1) * sizeof(chunk_t) <= nbytes + sizeof(chunk_t));
                assert(test1->next == NULL);

                Rover = bestFit->next;
                return bestFit - unitNum;
            }

            Rover = Rover->next;

        } while (Rover != marker);
    }

        next = Rover->next;
        prev = &Dummy;

        while (prev->next != Rover)   //find prev mem block
            prev = prev->next;

        Rover = morecore(pageCount * PAGESIZE);

        if (Rover == NULL)
            return NULL;

        Rover->size = unitNum + 1;
        
        if (((pageCount * PAGESIZE) / sizeof(chunk_t)) - (unitNum + 1) == 0) {
            chunk_t *test = Rover;
            Rover = Dummy.next;
            test->next = NULL;
            return (test + 1);
        }
        
        chunk_t *lastChunk = Rover;
        while (lastChunk->next != &Dummy)
            lastChunk = lastChunk->next;
        lastChunk->next = Rover;
            
        Rover += unitNum + 1;
        Rover->next = &Dummy;
        prev->next = Rover;
        (Rover - unitNum - 1)->next = NULL;

        test1 = Rover - unitNum - 1;
        test2 = Rover - unitNum;

        assert(test1 + 1 == test2);  
        assert((test1->size - 1) * sizeof(chunk_t) >= nbytes);
        assert((test1->size - 1) * sizeof(chunk_t) <= nbytes + sizeof(chunk_t));
        assert(test1->next == NULL);

        return (Rover - unitNum);

    return NULL;
}

/* Mem_stats
 * prints stats about the current free list
 *
 * -- number of items in the linked list including dummy item
 * -- min, max, and average size of each item (in bytes)
 * -- total memory in list (in bytes)
 * -- number of calls to sbrk and number of pages requested
 *
 * A message is printed if all the memory is in the free list
 */
void Mem_stats(void)
{
    // One of the stats you must collect is the total number
    // of pages that have been requested using sbrk.
    // Say, you call this NumPages.  You also must count M,
    // the total number of bytes found in the free list 
    // (including all bytes used for headers).  If it is the case
    // that M == NumPages * PAGESiZE then print
    
    //get the total number of unitNum in the list
    
    printf("\nTotal Number of Pages = %d\n", NumPages);
    printf("SBRK was called a total of %d times.\n", NumSbrkCalls);
    
    chunk_t *dumChunk = &Dummy;
    dumChunk = dumChunk->next;
    long min = 999999999, max = 0;
    long M = 0, avg = 0;
    int numItems = 1;
    min = dumChunk->size;
    max = dumChunk->size;
    while(dumChunk != &Dummy) {
        if (dumChunk->size >= max) {        //max
            max = dumChunk->size;
        }
        if (dumChunk->size <= min && dumChunk->size != 0) {   //min
            min = dumChunk->size;
        }
        avg += dumChunk->size;                              //avg
        M += (dumChunk->size * sizeof(chunk_t));
        dumChunk = dumChunk->next;
        numItems++;                                         //increment count
    }
    
    printf("Total number of items in free list = %d\n", numItems);
    printf("Min size of chunk in free list = %ld\n", min);
    printf("Max size of chunk in free list = %ld\n", max);
    printf("Average size of chunk in free list = %ld\n", avg/((long)numItems));
    printf("The size of a chunk_t is %lu.\n", sizeof(chunk_t));
    printf("Total bytes in free list = %ld\n\n", M);
    if(M == (NumPages * PAGESIZE))   
        printf("all memory is in the heap -- no leaks are possible\n");
}

/* Mem_print
 * print table of memory in free list 
 *
 * The print should include the dummy item in the list 
 */
void Mem_print(void)
{
    assert(Rover != NULL && Rover->next != NULL);
    chunk_t *start = &Dummy;
    chunk_t *test1 = start;
    do {
        // example format.  Modify for your design
        printf("p=%p, size=%ld, end=%p, next=%p %s\n", 
                test1, test1->size, test1 + test1->size, test1->next, test1->size!=0?"":"<-- dummy");
        test1 = test1->next;
    } while (test1 != start);
    mem_validate();
}

/* This is an experimental function to attempt to validate the free
 * list when coalescing is used.  It is not clear that these tests
 * will be appropriate for all designs.  If your design utilizes a different
 * approach, that is fine.  You do not need to use this function and you
 * are not required to write your own validate function.
 */
void mem_validate(void)
{
    assert(Rover != NULL && Rover->next != NULL);
    assert(Rover->size >= 0);
    int wrapped = FALSE;
    int found_dummy = FALSE;
    int found_rover = FALSE;
    chunk_t *test1, *largest, *smallest;

    // for validate begin at Dummy
    test1 = &Dummy;
    do {
        if (test1->size == 0) {
            assert(found_dummy == FALSE);
            found_dummy = TRUE;
        } else {
            assert(test1->size > 0);
        }
        if (test1 == Rover) {
            assert(found_rover == FALSE);
            found_rover = TRUE;
        }
        test1 = test1->next;
    } while (test1 != &Dummy);
    assert(found_dummy == TRUE);
    assert(found_rover == TRUE);

    if (Coalescing) {
        do {
            if (test1 >= test1->next) {
                // this is not good unless at the one wrap
                if (wrapped == TRUE) {
                    printf("validate: List is out of order, already found wrap\n");
                    printf("first largest %p, smallest %p\n", largest, smallest);
                    printf("second largest %p, smallest %p\n", test1, test1->next);
                    assert(0);   // stop and use gdb
                } else {
                    wrapped = TRUE;
                    largest = test1;
                    smallest = test1->next;
                }
            } else {
                assert(test1 + test1->size < test1->next);
            }
            test1 = test1->next;
        } while (test1 != &Dummy);
        assert(wrapped == TRUE);
    }
}

/* vi:set ts=8 sts=4 sw=4 et: */

