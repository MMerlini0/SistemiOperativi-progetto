#pragma once
#include "bit_map.h"







// Definizione della struct BuddyAllocator, dove rimuovo puntatore alla lista dei blocchi liberi
// e il poolallocator
typedef struct  {
  //ListHead free[MAX_LEVELS];
  BitMap bitmap;
  int num_levels;
  //PoolAllocator list_allocator;
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



/*
// Nella versione con gestione degli spazi liberi con liste avevo la struct BuddyListItem che si occupava di 
// gestire le liste associate ad un livello specifico del buddy
typedef struct BuddyListItem {
  ListItem list;
  int idx;   // tree index
  int level; // level for the buddy
  char* start; // start of memory
  int size;
  struct BuddyListItem* buddy_ptr;
  struct BuddyListItem* parent_ptr;
} BuddyListItem;


// Funzioni ausiliari per il buddyListItem:

// computes the size in bytes for the buffer of the allocator
int BuddyAllocator_calcSize(int num_levels);

// returns (allocates) a buddy at a given level.
// side effect on the internal structures
// 0 id no memory available
BuddyListItem* BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level);

// releases an allocated buddy, performing the necessary joins
// side effect on the internal structures
void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, BuddyListItem* item);

//allocates memory
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size);

//releases allocated memory
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem);
*/