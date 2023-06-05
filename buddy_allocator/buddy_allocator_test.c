#include "buddy_allocator.h"
#include <stdio.h>
#include "math.h"

#define MEMORY_SIZE (1024*1024)
#define MIN_BUCKET_SIZE (8)
#define PAGE_SIZE 4096

char memory[MEMORY_SIZE];

BuddyAllocator alloc;
int main(int argc, char** argv) {


  //2 we initialize the allocator
  printf("init... ");

  int num_levels=log(MEMORY_SIZE/MIN_BUCKET_SIZE)/log(2);
  
  BuddyAllocator_init(&alloc, 
                      memory,
                      num_levels,
                      PAGE_SIZE/4,
                      MIN_BUCKET_SIZE);
  printf("DONE\n");

  void* p1=BuddyAllocator_malloc(&alloc, 100);
  void* p2=BuddyAllocator_malloc(&alloc, 100);
  void* p3=BuddyAllocator_malloc(&alloc, 1000);
  BuddyAllocator_free(&alloc, p1);
  BuddyAllocator_free(&alloc, p2);
  BuddyAllocator_free(&alloc, p3);
  
}
