#pragma once
#include "bit_map.h"






typedef struct  {
  BitMap bitmap;
  int num_levels;
  char* memory;
  int memory_size;
  int min_bucket_size;
} BuddyAllocator;




// Inizializzazione del buddyAllocator
int BuddyAllocator_init(BuddyAllocator* alloc,
                        int num_levels,
                        char* memory,
                        int memory_size,
                        char* bufferBitmap,
                        int bufferBitmap_size,
                        int min_bucket_size);



// Le funzioni sono definite nel file corrispettivo (buddy_allocator.c)
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size);
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem);
void BitMap_setParentsBit(BitMap *bit_map, int bit_num, int status);
void BitMap_setChildrensBit(BitMap *bit_map, int bit_num, int status);
void merge(BitMap *bitmap, int idx);
void Bitmap_print(BitMap *bit_map);
int BufferSizeCalculator(int num_levels);

