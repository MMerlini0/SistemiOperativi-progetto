#include "buddy_allocator.h"

void* disastrOS_malloc(BuddyAllocator* alloc, int size);
void disastrOS_free(BuddyAllocator* alloc, void* puntatore);