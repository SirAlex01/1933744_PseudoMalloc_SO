#define MEMORY_SIZE (1024*1024)
#define MIN_BUCKET_SIZE (8)
#define PAGE_SIZE 4096

#include "buddy_allocator.h"

void* pseudo_malloc(uint64_t size);
void pseudo_free(void* pointer);
