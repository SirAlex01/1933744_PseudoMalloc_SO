#include "pseudo_malloc.h"
#include <math.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>

BuddyAllocator alloc;
int buddy_initialized=0;
unsigned char memory[MEMORY_SIZE];

void* pseudo_malloc(uint64_t size){
    //il blocco del buddy allocator ha 4 B di overhead
    if (size+4<PAGE_SIZE/4) {
        //se non è inizializzato
        if (!buddy_initialized) {
            int num_levels=log2(MEMORY_SIZE/MIN_BUCKET_SIZE);
            //inizializzo il buddy allocator
            BuddyAllocator_init(&alloc, 
                                memory,
                                num_levels,
                                PAGE_SIZE/4,
                                MIN_BUCKET_SIZE);
            buddy_initialized=1;
        }
        //alloco con il buddy allocator
        void* res=BuddyAllocator_malloc(&alloc,size);

        //se non già tutto allocato, restituirà un blocco di memoria
        // che sarà il risultato
        if (res)
          return res;
    }
    //mmap della memoria richiesta +16 B di overhead
    //8 per il backup del puntatore
    //8 per il backup della dimensione della memoria allocata 
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
    printf("requested %ld bytes\n",*((uint64_t*)(req+8))-16);

    return req+16;

}

void pseudo_free(void* pointer) {
    //comportamento della vera malloc: free(NULL) non restituisce né errori né avvisi di alcun tipo
    if (!pointer)
      return;
    //se il blocco appartiene al buddy, sarà lui a doverlo deallocare
    if (buddy_initialized && BuddyAllocator_isMyBlock(&alloc,pointer))
        BuddyAllocator_free(&alloc,pointer);
    else {
        //altrimenti recupero il puntatore originale al blocco di memoria
        printf("freeing %p\n",pointer);

        char* p=(char*)pointer-16;
        char** same_pointer=(char**)p;
        //verifico che il backup del puntatore contenga lo stesso puntatore
        assert(*same_pointer==p && "puntatore non valido!");
        //prendo la dimensione (per l'munmap)
        uint64_t size=*((uint64_t*)(p+8));

        int res=munmap(p,size);
        assert(!res && "munmap failed!!");
    }
}