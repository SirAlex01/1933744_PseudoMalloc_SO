#pragma once
#include "bit_map.h"

typedef struct  {
  BitMap bit_map; // to store the tree structure
  unsigned char * memory; // the memory area to be managed
  int num_levels; // the number of levels 
  int max_bucket_size; // the maximum number of bytes that can be returned
  int min_bucket_size; // the minimum number of bytes that can be returned
} BuddyAllocator;


int BuddyAllocator_setSuccessorBits(BitMap* bitmap,int bit_value, int start, int index, int end);

// initializes the buddy allocator, and checks that the buffer is large enough
void BuddyAllocator_init(BuddyAllocator* alloc,
                         unsigned char* memory,
                         int num_levels, 
                         int max_bucket_size,
                         int min_bucket_size);

// returns (allocates) a buddy at a given level.
// side effect on the internal structures
// 0 id no memory available
int BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level);

// releases an allocated buddy, performing the necessary joins
// side effect on the internal structures
void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, int block_idx);

//allocates memory
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size);

int BuddyAllocator_isMyBlock(BuddyAllocator* alloc, void* mem);
//releases allocated memory
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem);
