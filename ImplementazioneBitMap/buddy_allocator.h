#pragma once
// #include "pool_allocator.h"
#include "bit_map.h"

#define MAX_LEVELS 16



// NON USO PIU LA LISTA
/*
// one entry of the buddy list
typedef struct BuddyListItem {
  ListItem list;
  int idx;   // tree index
  int level; // level for the buddy
  char* start; // start of memory
  int size;
  struct BuddyListItem* buddy_ptr;
  struct BuddyListItem* parent_ptr;
} BuddyListItem;
 */


typedef struct  {
  //ListHead free[MAX_LEVELS];
  BitMap bitmap;
  int num_levels;
  //PoolAllocator list_allocator;
  char* memory; // MEMORIA
  int memory_size;
  int min_bucket_size; // the minimum page of RAM that can be returned
} BuddyAllocator;


/*
// computes the size in bytes for the buffer of the allocator
int BuddyAllocator_calcSize(int num_levels);
*/


// initializes the buddy allocator, and checks that the buffer is large enough
int BuddyAllocator_init(BuddyAllocator* alloc,
                        int num_levels,
                        char* memory,
                        int memory_size,
                        char* bufferBitmap,
                        int bufferBitmap_size,
                        int min_bucket_size);






/*
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

//allocates memory
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size);

//releases allocated memory
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem);

//setta lo stato dei genitori a status
void BitMap_setParentsBit(BitMap *bit_map, int bit_num, int status);

//setta lo stato dei discendenti a status
void BitMap_setChildrensBit(BitMap *bit_map, int bit_num, int status);

//funzione per ricompattare i vari livelli
void merge(BitMap *bitmap, int idx);

void Bitmap_print(BitMap *bit_map);

int BufferSizeCalculator(int num_levels);