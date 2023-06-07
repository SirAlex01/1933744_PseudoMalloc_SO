#include "pseudo_malloc.h"
#include <math.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>

BuddyAllocator alloc;
int buddy_initialized=0;
unsigned char memory[MEMORY_SIZE];

void* pseudo_malloc(uint64_t size){
    if (size+4<PAGE_SIZE/4) {
        if (!buddy_initialized) {
            int num_levels=log2(MEMORY_SIZE/MIN_BUCKET_SIZE);
            BuddyAllocator_init(&alloc, 
                                memory,
                                num_levels,
                                PAGE_SIZE/4,
                                MIN_BUCKET_SIZE);
            buddy_initialized=1;
        }
        void* res=BuddyAllocator_malloc(&alloc,size);
        if (res)
          return res;
    }
    char* req = (char*) mmap(0,
				            size+16,
				            PROT_READ|PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS,
				            -1,
		                    0);
    
    assert(req != MAP_FAILED);
    char** pointer=(char**)req;
    *pointer=req;
    //printf("%p,%p\n",*pointer,req);
    uint64_t* size_backup=(uint64_t*)(req+8);
    *size_backup=size+16;
    printf("%ld\n",*((uint64_t*)(req+8)));

    return req+16;

}

void pseudo_free(void* pointer) {
    if (!pointer)
      return;
    if (buddy_initialized && BuddyAllocator_isMyBlock(&alloc,pointer))
        BuddyAllocator_free(&alloc,pointer);
    else {
        char* p=(char*)pointer-16;
        char** same_pointer=(char**)p;
        //printf("%p,%p\n",p,*same_pointer);
        assert(*same_pointer==p && "puntatore non valido!");
        uint64_t size=*((uint64_t*)(p+8));
        //printf("%ld\n",size);
        int res=munmap(p,size);
        assert(!res && "munmap failed!!");
    }
}