#include "buddy_allocator.h"
#include <stdio.h>
#include "math.h"

#define MEMORY_SIZE (1024*1024)
#define MIN_BUCKET_SIZE (8)
#define PAGE_SIZE 4096

char memory[MEMORY_SIZE];

BuddyAllocator alloc;
int main(int argc, char** argv) {

  //1 we see if we have enough memory for the buffers
  //int req_size=BuddyAllocator_calcSize(BUDDY_LEVELS);
  //printf("size requested for initialization: %d/BUFFER_SIZE\n", req_size);

  //2 we initialize the allocator
  printf("init... ");
    int num_levels=log(MEMORY_SIZE/MIN_BUCKET_SIZE)/log(2);

    BuddyAllocator_init(&alloc, 
                      memory,
                      num_levels,
                      PAGE_SIZE/4,
                      MIN_BUCKET_SIZE);
  printf("DONE\n");
  
  // we request two buddies of the smallest size
  printf("getting buddy of depth 7\n");
  int item7_1=BuddyAllocator_getBuddy(&alloc, 7);
  printf("%d\n",item7_1);
  printf("getting another buddy of depth 7\n");
  int item7_2=BuddyAllocator_getBuddy(&alloc, 7);
  printf("%d\n",item7_2);


  printf("getting a buddy of depth 6\n");
  int item6_1=BuddyAllocator_getBuddy(&alloc, 6); 
  printf("%d\n",item6_1);
  printf("getting another buddy of depth 6\n");
  int item6_2=BuddyAllocator_getBuddy(&alloc, 6);
  printf("%d\n",item6_2);
 
  printf("releasing a buddy of depth 7\n");
  BuddyAllocator_releaseBuddy(&alloc, item7_1);
  printf("releasing another buddy of depth 7\n");
  BuddyAllocator_releaseBuddy(&alloc, item7_2);
  printf("getting a buddy of depth 6\n");
  int item6_3=BuddyAllocator_getBuddy(&alloc, 6); 
  printf("%d\n",item6_3);

  printf("releasing a buddy of depth 6\n");
  BuddyAllocator_releaseBuddy(&alloc, item6_1);
  printf("releasing another buddy of depth 6\n");
  BuddyAllocator_releaseBuddy(&alloc, item6_2);
  printf("releasing another buddy of depth 6\n");
  BuddyAllocator_releaseBuddy(&alloc, item6_3);

  printf("getting buddy of depth 7\n");
  item7_1=BuddyAllocator_getBuddy(&alloc, 7);
  printf("%d\n",item7_1);
  printf("releasing a buddy of depth 7\n");
  BuddyAllocator_releaseBuddy(&alloc, item7_1);
  

  
  
}
