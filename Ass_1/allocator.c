//
//  COMP1927 Assignment 1 - Vlad: the memory allocator
//  allocator.c ... implementation
//
//  Created by Liam O'Connor on 18/07/12.
//  Modified by John Shepherd in August 2014, August 2015
//  Copyright (c) 2012-2015 UNSW. All rights reserved.
//

#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define FREE_HEADER_SIZE  sizeof(struct free_list_header)  
#define ALLOC_HEADER_SIZE sizeof(struct alloc_block_header)  
#define MAGIC_FREE     0xDEADBEEF   // Freed memory
#define MAGIC_ALLOC    0xBEEFDEAD   // Allocated memory

#define BEST_FIT       1
#define WORST_FIT      2
#define RANDOM_FIT     3

// Extra Macros
#define MIN_MALLOC     1024   // Minimum malloc size
#define MIN_ALLOCATE   8      // Minimum allocation size
#define TRUE           0
#define FALSE          1
#define POWER          2      // Malloc size must be power of 2
#define MULTIPLE       4      // Alloc size must be multiple of 4
#define THRESHOLD      (ALLOC_HEADER_SIZE + n + 2*FREE_HEADER_SIZE)

// Reference typedefs
typedef unsigned char byte;   // memory addresses in HEX
typedef u_int32_t vsize_t;    // size of allocated / free blocks
typedef u_int32_t vlink_t;    // reference to FREE REGION HEADERS
typedef u_int32_t vaddr_t;    // reference to MEMORY ADDRESS INDICES

// Free block struct
typedef struct free_list_header {
   u_int32_t magic;  // ought to contain MAGIC_FREE
   vsize_t size;     // # bytes in this block (including header)
   vlink_t next;     // memory[] index of next free block
   vlink_t prev;     // memory[] index of previous free block
} free_header_t;

// Allocated block struct
typedef struct alloc_block_header {
   u_int32_t magic;  // ought to contain MAGIC_ALLOC
   vsize_t size;     // # bytes in this block (including header)
} alloc_header_t;

// ################
// Global Variables
// ################

static byte *memory = NULL;   // pointer to start of allocator memory
static vaddr_t free_list_ptr; // index in memory[] of first block in free list
static vsize_t memory_size;   // number of bytes malloc'd in memory[]
static u_int32_t strategy;    // allocation strategy (by default BEST_FIT)

// #################
// Private Functions
// #################

// Converet size for vlad initialisation
static u_int32_t init_memory_size(u_int32_t size) {
   assert(size >= MIN_MALLOC);
   printf("Before conversion, size is: %d bytes\n", size);
   
   int convertedSize = MIN_MALLOC;
   while (size > convertedSize) {
      convertedSize *= 2;
   }
   printf("After conversion, size is: %d bytes\n", convertedSize);
   return convertedSize;
}

// Convert n bytes into appropriate size for allocation
static u_int32_t conv_n_bytes(u_int32_t n) {
   if (n < MIN_ALLOCATE) {
      n = MIN_ALLOCATE;
   }
   while (n%MULTIPLE != 0) {
      n++;
   }
   printf("...n bytes is now size: %d\n", n);
   assert(n%MULTIPLE == 0);
   return n;
}

// Convert index to ptr
static void *conv_to_ptr(vlink_t index) {
   void *ptr;
   ptr = memory + index;   // memory address in hex + free_region_header address = ptr to first byte after header (for alloc block)
   return ptr;
}

// Convert ptr to index
static u_int32_t conv_to_ind(void *ptr) {
   vlink_t index = 0;
   index = (byte *) ptr - memory;   // ptr address in hex - memory address in hex
   return index;
}

static void vlad_merge();

// ###################
// Interface Functions
// ###################

// Input: size - number of bytes to make available to the allocator
// Output: none              
// Precondition: Size >= 1024
// Postcondition: `size` bytes are now available to the allocator
// 
// (If the allocator is already initialised, this function does nothing,
//  even if it was initialised with different size)

void vlad_init(u_int32_t size)
{
   // Convert size and initialise memory
   if (memory == NULL) {             
      if (size < MIN_MALLOC) {             // convert size to MIN
         size = MIN_MALLOC;
         assert(size == MIN_MALLOC);
         memory = malloc(size);
      } else {                             // convert size to next power of 2
         size = init_memory_size(size);    
         memory = malloc(size);
      }
      printf("...Memory initialised\n");  
      strategy = BEST_FIT;
      memory_size = size;
      free_list_ptr = (vaddr_t) 0;
      free_header_t *init_header = (free_header_t *) memory;
      init_header->magic = MAGIC_FREE;
      init_header->size = size;
      init_header->next = free_list_ptr;
      init_header->prev = free_list_ptr;
      printf("...All other variables assigned\n");  
   }
   if (memory == NULL){
      fprintf(stderr, "vlad_init: insufficient memory\n");
      exit(EXIT_FAILURE);
   }
   printf("...Initialisation completed\n");
}

// Input: n - number of bytes requested
// Output: p - a pointer, or NULL
// Precondition: n < size of largest available free block
// Postcondition: If a region of size n or greater cannot be found, p = NULL 
//                Else, p points to a location immediately after a header block
//                      for a newly-allocated region of some size >= 
//                      n + header size.

void *vlad_malloc(u_int32_t n)
{
   // Convert n to suitable size
   n = conv_n_bytes(n);
   printf("...Conversion is successful!\n");
   // Initialise current ptr to first block
   free_header_t *curr = (free_header_t*) conv_to_ptr(free_list_ptr);
   if (curr->magic != MAGIC_FREE) {
      fprintf(stderr, "vlad_alloc: Memory corruption\n");
      exit(EXIT_FAILURE);
   }
   // Search for suitable region
   int found = FALSE;
   free_header_t *chosen = NULL;
   do {
      assert(curr->magic == MAGIC_FREE);
      if (curr->size >= (ALLOC_HEADER_SIZE + n)) {          // Suitable region found
         printf("selected region size = %u\n", curr->size);
         chosen = curr;
         found = TRUE;  
      } else {                                              // Continue free block traverse
         curr = (free_header_t*) conv_to_ptr(curr->next);
      }
   } while (curr != conv_to_ptr(free_list_ptr) && found == FALSE);
   printf("...Region is found!\n");
   printf("...Size of chosen is: %d\n", chosen->size);
   // Check if chosen is last free region available
   if (chosen->next == conv_to_ind(chosen) && chosen->size < THRESHOLD) {
      return NULL;
   }
   printf("...Passed last free region check\n"); 

   // Allocation PART 1: Declare pointers
   alloc_header_t *allocPart = NULL;
   free_header_t *freePart = NULL;
   free_header_t *temp1 = NULL;
   free_header_t *temp2 = NULL;
   // Allocation PART 2: Allocate region
   int originalSize = chosen->size;                // Save size of chosen
   allocPart = (alloc_header_t *) chosen;
   vsize_t allocSize = ALLOC_HEADER_SIZE + n;      // Save size of allocated region
   allocPart->magic = MAGIC_ALLOC;
   allocPart->size = allocSize;
   printf("...Size of allocPart is: %d\n", allocPart->size);
   
   // Allocation PART 3: Relinking free regions                                 
   if (originalSize >= THRESHOLD) {
      // Initialise remaining free block   
      byte *nextFree;
      nextFree = (byte *) chosen + allocSize;
      freePart = (free_header_t *) nextFree;
      freePart->magic = MAGIC_FREE;
      freePart->size = originalSize - allocSize;
      printf("...Size of freePart is: %d\n", freePart->size);
      // Connect remaining free block into free list
      temp1 = conv_to_ptr(chosen->prev);
      temp1->next = conv_to_ind(freePart);
      freePart->prev = chosen->prev;
      temp2 = conv_to_ptr(chosen->next);
      temp2->prev = conv_to_ind(freePart);
      freePart->next = chosen->next;
   } else {
      // Connect prev free region to next region
      temp1 = conv_to_ptr(chosen->prev);
      temp1->next = chosen->next;
      temp2 = conv_to_ptr(chosen->next);
      temp2->prev = chosen->prev;
   }
   // Post-allocation: Repoint free_list_ptr to start
   curr = (free_header_t *) conv_to_ptr(free_list_ptr);
   if (curr->magic == MAGIC_ALLOC) {
      free_list_ptr = curr->next;
   }
   // Return 1st byte immediately after allocated region header
   byte *chosen_ptr = (byte *) allocPart;
   return ((void*) chosen_ptr + ALLOC_HEADER_SIZE);
}

// Input: object, a pointer.
// Output: none
// Precondition: object points to a location immediately after a header block
//               within the allocator's memory.
// Postcondition: The region pointed to by object has been placed in the free
//                list, and merged with any adjacent free blocks; the memory
//                space can be re-allocated by vlad_malloc

void vlad_free(void *object)
{
   // Set ptr = object index
   byte *alloc_addr = (void *) object;
   alloc_header_t *alloc_block = (alloc_header_t *) alloc_addr;
   printf("...Initialisation successful\n");

   vaddr_t index = conv_to_ind(alloc_block);

   //byte *regionStart = (byte *) alloc_block - ALLOC_HEADER_SIZE;
   
   /*Check if ptr lies within allocated region
   if (regionStart >= NULL) {
      fprintf(stderr, "vlad_free: Attempt to free via invalid pointer\n");
      exit(EXIT_FAILURE);
   }*/

   // Check if region is allocated
   if (alloc_block->magic != MAGIC_ALLOC){
      fprintf(stderr, "vlad_free: Attempt to free via non-allocated memory\n");
      exit(EXIT_FAILURE);
   }
   printf("...Region is currently allocated\n");

   // Re-arrange free_list_ptr to correct region
   if (index < free_list_ptr){
      free_list_ptr = index;
   }
   vlad_merge(index);
}

// Input: current state of the memory[]
// Output: new state, where any adjacent blocks in the free list
//            have been combined into a single larger block; after this,
//            there should be no region in the free list whose next
//            reference is to a location just past the end of the region

static void vlad_merge(vaddr_t index)
{
   // Initialise traversal pointers
   free_header_t *curr = (free_header_t *) conv_to_ptr(index);
   free_header_t *temp = (free_header_t *) conv_to_ptr(index);
   // Check adjacency
   while (curr->next != free_list_ptr) {
      if (index + curr->size == curr->next){ 
         curr->next = temp->next;
         temp = conv_to_ptr(temp->next);
         temp->prev = index;
         vlad_merge(index);
      }
      temp = curr;
      curr = conv_to_ptr(curr->next);
   }
}

// Stop the allocator, so that it can be init'ed again:
// Precondition: allocator memory was once allocated by vlad_init()
// Postcondition: allocator is unusable until vlad_int() executed again

void vlad_end(void)
{
   memory = NULL;
   free(memory);
   printf("Vlad the impaler is dead!\n");
}


// Precondition: allocator has been vlad_init()'d
// Postcondition: allocator stats displayed on stdout

void vlad_stats(void)
{
   // TODO
   // put whatever code you think will help you
   // understand Vlad's current state in this function
   // REMOVE all of these statements when your vlad_malloc() is done
   printf("vlad_stats() won't work until vlad_malloc() works\n");
   return;
}

