#include <stdio.h>
#include <assert.h>
#include <math.h> // for floor and log2
#include "buddy_allocator.h"

// these are trivial helpers to support you in case you want
// to do a bitmap implementation
int levelIdx(size_t idx){
  return (int)floor(log2(idx));
};

int buddyIdx(int idx){
  if (idx&0x1){
    return idx-1;
  }
  return idx+1;
}

int parentIdx(int idx){
  return idx/2;
}

int startIdx(int idx){
  return (idx-(1<<levelIdx(idx)));
}









// computes the size in bytes for the allocator
int BuddyAllocator_calcSize(int num_levels) {
  int list_items=1<<(num_levels+1); // maximum number of allocations, used to determine the max list items
  int list_alloc_size=(sizeof(BuddyListItem)+sizeof(int))*list_items;
  return list_alloc_size;
}









// creates an item from the index
// and puts it in the corresponding list
BuddyListItem* BuddyAllocator_createListItem(BuddyAllocator* alloc,
                                            int idx,
                                            BuddyListItem* parent_ptr){
  BuddyListItem* item=(BuddyListItem*)PoolAllocator_getBlock(&alloc->list_allocator);
  item->idx=idx;
  item->level=levelIdx(idx);
  item->start= alloc->memory + ((idx-(1<<levelIdx(idx))) << (alloc->num_levels-item->level) )*
    alloc->min_bucket_size;
  item->size=(1<<(alloc->num_levels-item->level))*alloc->min_bucket_size;
  item->parent_ptr=parent_ptr;
  item->buddy_ptr=0;
  List_pushBack(&alloc->free[item->level],(ListItem*)item);
  printf("Creating Item. idx:%d, level:%d, start:%p, size:%d\n", 
        item->idx, item->level, item->start, item->size);
  return item;
};







// detaches and destroys an item in the free lists 
void BuddyAllocator_destroyListItem(BuddyAllocator* alloc, BuddyListItem* item){
  int level=item->level;
  List_detach(&alloc->free[level], (ListItem*)item);
  printf("Destroying Item. level:%d, idx:%d, start:%p, size:%d\n",
        item->level, item->idx, item->start, item->size);
  PoolAllocatorResult release_result=PoolAllocator_releaseBlock(&alloc->list_allocator, item);
  assert(release_result==Success);

};











void BuddyAllocator_init(BuddyAllocator* alloc,
                        int num_levels,
                        char* buffer,
                        int buffer_size,
                        char* memory,
                        int min_bucket_size){

  // we need room also for level 0
  alloc->num_levels=num_levels;
  alloc->memory=memory;
  alloc->min_bucket_size=min_bucket_size;
  assert (num_levels<MAX_LEVELS);
  // we need enough memory to handle internal structures
  assert (buffer_size>=BuddyAllocator_calcSize(num_levels));
  


  
  // MA ORA IL BUFFER CONTERRA' ANCHE LE BITMAP
  int list_alloc_size = BuddyAllocator_calcSize(num_levels);
  // MI PRENDO QUANTI ELEMENTI AVRO' IN TOTALE NELL'ALBERO
  int elementi_totali=1<<(num_levels+1); 
  int bitmap_bytes = BitMap_getBytes(elementi_totali);      
  int sizeBufferConOgniBitmap = list_alloc_size + bitmap_bytes;
  // VERIFICO CHE CI SIA ABBASTANZA SPAZIO ANCHE CONSIDERANDO I BYTE PRESI PER LE BITMAP
  assert (buffer_size>=sizeBufferConOgniBitmap);





/*
  int list_items=1<<(num_levels+1); // maximum number of allocations, used to size the list
  int list_alloc_size=(sizeof(BuddyListItem)+sizeof(int))*list_items;
*/



  printf("BUDDY INITIALIZING\n");
  printf("\tlevels: %d", num_levels);
  printf("\tmax list entries %d bytes\n", list_alloc_size);
  printf("\tbucket size:%d\n", min_bucket_size);
  printf("\tmanaged memory %d bytes\n", (1<<num_levels)*min_bucket_size);
  


  // ALL'INIZIO DEL BUFFER METTERO' LA BITMAP
  BitMap_init(&alloc->bitmap, elementi_totali, (uint8_t*)buffer);  
  for (int i = 0; i < elementi_totali; ++i) {
    // METTO TUTTI I BIT DELLA BITMAP A 0
    BitMap_setBit(&alloc->bitmap, i, 0);                                                      
  }
  // TRANNE CHE PER LA BITMAP DEL PRIMO LIVELLO CHE VERRA' SETTATA A 1
  BitMap_setBit(&alloc->bitmap, 1, 1);



  // the buffer for the list starts where the bitmap ends
  // QUINDI SPOSTO L'INIZIO DELLA LISTA DI BITMAP BYTES
  char *list_start=buffer + bitmap_bytes;
  PoolAllocatorResult init_result=PoolAllocator_init(&alloc->list_allocator,
                sizeof(BuddyListItem),
                elementi_totali,
                list_start,
                list_alloc_size);
  printf("%s\n",PoolAllocator_strerror(init_result));


  /* // we initialize all lists
  for (int i=0; i<MAX_LEVELS; ++i) {
    List_init(alloc->free+i);
  }
  */

  // we allocate a list_item to mark that there is one "materialized" list
  // in the first block
  // BuddyAllocator_createListItem(alloc, 1, 0);
};







BuddyListItem* BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level){
  if (level<0)
    return 0;
  assert(level <= alloc->num_levels);

  // IL SE NON CI SONO BUDDIES IN QUESTO LIVELLO DIVENTA
  // SE, DALLA PARTENZA DEGLI INDICI NELLA BITMAP DI QUESTO LIVELLO, ALLA SUA FINE, NON CE NE SONO CON BIT ZERO
  // QUINDI PRIMA MI PRENDO GLI INDICI
  int level_start = (1 << level) - 1;
  int level_end = (1 << (level + 1)) - 2;


  //VERIFICO SE NON CI SONO BUDDIES IN QUESTO LIVELLO
  for (int idx = level_start; idx <= level_end; ++idx) {
    // QUINDI MI METTO PRIMA NEL CASO IN CUI VE NE SIA UNO LIBERO
    if (!BitMap_bit(&alloc->bitmap, idx)) {
      // MARCO ORA IL BIT DELLA BITMAP COME USATO
      BitMap_setBit(&alloc->bitmap, idx, 1);
      BuddyListItem* item = BuddyAllocator_createListItem(alloc, idx, 0);
      return item;
    }
  }


  if (! alloc->free[level].size ) { // no buddies on this level
    BuddyListItem* parent_ptr=BuddyAllocator_getBuddy(alloc, level-1);
    if (! parent_ptr)
      return 0;

    // parent already detached from free list
    int left_idx=parent_ptr->idx<<1;
    int right_idx=left_idx+1;
    
    printf("split l:%d, left_idx: %d, right_idx: %d\r", level, left_idx, right_idx);
    BuddyListItem* left_ptr=BuddyAllocator_createListItem(alloc,left_idx, parent_ptr);
    BuddyListItem* right_ptr=BuddyAllocator_createListItem(alloc,right_idx, parent_ptr);
    // we need to update the buddy ptrs
    left_ptr->buddy_ptr=right_ptr;
    right_ptr->buddy_ptr=left_ptr;
  }
  // we detach the first
  if(alloc->free[level].size) {
    BuddyListItem* item=(BuddyListItem*)List_popFront(alloc->free+level);
    return item;
  }
  assert(0);
  return 0;
}

void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, BuddyListItem* item){

  BuddyListItem* parent_ptr=item->parent_ptr;
  BuddyListItem *buddy_ptr=item->buddy_ptr;
  
  // buddy back in the free list of its level
  List_pushFront(&alloc->free[item->level],(ListItem*)item);

  // if on top of the chain, do nothing
  if (! parent_ptr)
    return;
  
  // if the buddy of this item is not free, we do nothing
  if (buddy_ptr->list.prev==0 && buddy_ptr->list.next==0) 
    return;
  
  //join
  //1. we destroy the two buddies in the free list;
  printf("merge %d\n", item->level);
  BuddyAllocator_destroyListItem(alloc, item);
  BuddyAllocator_destroyListItem(alloc, buddy_ptr);
  //2. we release the parent
  BuddyAllocator_releaseBuddy(alloc, parent_ptr);

}

//allocates memory
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size) {
  // we determine the level of the page
  int mem_size=(1<<alloc->num_levels)*alloc->min_bucket_size;
  int  level=floor(log2(mem_size/(size+8)));

  // if the level is too small, we pad it to max
  if (level>alloc->num_levels)
    level=alloc->num_levels;

  printf("requested: %d bytes, level %d \n",
        size, level);

  // we get a buddy of that size;
  BuddyListItem* buddy=BuddyAllocator_getBuddy(alloc, level);
  if (! buddy)
    return 0;

  // we write in the memory region managed the buddy address
  BuddyListItem** target= (BuddyListItem**)(buddy->start);
  *target=buddy;
  return buddy->start+8;
}
//releases allocated memory
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem) {
  printf("freeing %p", mem);
  // we retrieve the buddy from the system
  char* p=(char*) mem;
  p=p-8;
  BuddyListItem** buddy_ptr=(BuddyListItem**)p;
  BuddyListItem* buddy=*buddy_ptr;
  //printf("level %d", buddy->level);
  // sanity check;
  assert(buddy->start==p);
  BuddyAllocator_releaseBuddy(alloc, buddy);
  
}
