#include "buddy_allocator.h"
#include <stdio.h>
#include "math.h"

#define MEMORY_SIZE (1024*1024)
#define MIN_BUCKET_SIZE (8)
#define PAGE_SIZE 4096

unsigned char memory[MEMORY_SIZE];

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

  
  void* p1=BuddyAllocator_malloc(&alloc, 1);
  void* p2=BuddyAllocator_malloc(&alloc, 100);
  void* p3=BuddyAllocator_malloc(&alloc, 1000);
  BuddyAllocator_free(&alloc, p1);
  BuddyAllocator_free(&alloc, p2);
  BuddyAllocator_free(&alloc, p3);

  p1=BuddyAllocator_malloc(&alloc, 200);
  p2=BuddyAllocator_malloc(&alloc, 300);
  p3=BuddyAllocator_malloc(&alloc, 400);
  void* p4=BuddyAllocator_malloc(&alloc, 1000);
  void* p5=BuddyAllocator_malloc(&alloc, 1000);

  BuddyAllocator_free(&alloc, p5);

  void* p6=BuddyAllocator_malloc(&alloc, 400);
  void* p7=BuddyAllocator_malloc(&alloc, 450);

  BuddyAllocator_free(&alloc, p1);
  BuddyAllocator_free(&alloc, p2);
  BuddyAllocator_free(&alloc, p4);
  BuddyAllocator_free(&alloc, p6);
  BuddyAllocator_free(&alloc, p3);
  BuddyAllocator_free(&alloc, p7);
  

  p6=BuddyAllocator_malloc(&alloc, 1020);
  BuddyAllocator_free(&alloc, p6);

  BuddyAllocator_free(&alloc,NULL);
  //double free
  //BuddyAllocator_free(&alloc, p6);
  /*
  //prova allocazione di tutta la memoria disponibile (+1)
  for (int i=0;i<=4*MEMORY_SIZE/PAGE_SIZE;i++) {
    void* p=BuddyAllocator_malloc(&alloc,509);
    printf("%p,%p\n",p,p+1024);
    //BuddyAllocator_free(&alloc,p);
  }
  */
}
