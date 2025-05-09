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
  //                                                                                                    NON MI SERVE PIU QUESTO CODICE PERCHE' ORA NON DEVO PIU'
  //                                                                                                    INSERIRE IL BLOCCO NELLA LISTA LIBERA, NON HO PIU ' UNA LISTA
  //                                                                                                    MA UNA BITMAP
  //List_pushBack(&alloc->free[item->level],(ListItem*)item);                                           // INSERISCE IL BLOCCO NELLA LISTA DEL LIVELLO CORRETTO
  BitMap_setBit(&alloc->bitmap, idx, 1);                                                                // SEGNA IL BIT CORRISPONDENTE NELLA BITMAP AD OCCUPATO
  printf("Creating Item. idx:%d, level:%d, start:%p, size:%d\n", 
    item->idx, item->level, item->start, item->size);
  return item;
};

// detaches and destroys an item in the free lists 
void BuddyAllocator_destroyListItem(BuddyAllocator* alloc, BuddyListItem* item){
  int level=item->level;
  //                                                                                                     NON HO PIU' BISOGNO DI STACCARE IL BLOCCO DALLA LISTA LIBERA
  //                                                                                                     PERCHE' ORA NON HO PIU' UNA LISTA MA UNA BITMAP
  //List_detach(&alloc->free[level], (ListItem*)item);                                                   // RIMUOVE IL BLOCCO NELLA LISTA LIBERA
  BitMap_setBit(&alloc->bitmap, item->idx, 0);                                                           // METTE A 0 IL BIT DELLA BITMAP SEGNANDO CHE IL BLOCCO E' LIBERO
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


      alloc->num_levels=num_levels;
  alloc->memory=memory;
  alloc->min_bucket_size=min_bucket_size;
  
  
  assert (num_levels<MAX_LEVELS); // Verifico che i livelli siano minori del massimo consentito
  int list_items=1<<(num_levels+1); // maximum number of allocations, used to size the list
  int list_alloc_size=(sizeof(BuddyListItem)+sizeof(int))*list_items;

  int bitmap_bytes = BitMap_getBytes(list_items);                                           // CALCOLO QUANTI BYTE OCCUPA LA BITMAP
  int required_size = bitmap_bytes + list_alloc_size;                                       // ORA LA DIMENSIONE DEVE CONSIDERARE ANCHE QUELLA DELLA BITMAP
  assert (buffer_size>=required_size);    // verifico che la memoria sia sufficiente per questo livello     CONSIDERA LA NUOVA DIMENSIONE

  printf("BUDDY INITIALIZING\n");
  printf("\tlevels: %d", num_levels);
  printf("\tmax list entries %d bytes\n", list_alloc_size);
  printf("\tbucket size:%d\n", min_bucket_size);
  printf("\tmanaged memory %d bytes\n", (1<<num_levels)*min_bucket_size);
  

  //                                                                                          // INIZIALIZZO LA BITMAP ALL'INIZIO DEL BUFFER
  BitMap_init(&alloc->bitmap, list_items, (uint8_t*)buffer);                                  // INIZIALIZZO LA BITMAP
  for (int i = 0; i < list_items; ++i) {
    BitMap_setBit(&alloc->bitmap, i, 0);                                                      // METTO TUTTI I BIT DELLA BITMAP A 0
  }



  // the buffer for the list starts where the bitmap ends
  char *list_start = buffer + bitmap_bytes;                                                   // AGGIUNGO I BYTES DELLA BITMAP
  PoolAllocatorResult init_result=PoolAllocator_init(&alloc->list_allocator,
    sizeof(BuddyListItem),
		list_items,
		list_start,
		list_alloc_size);
  printf("%s\n",PoolAllocator_strerror(init_result));


  /*
  // we initialize all lists                                                                  // QUA INIZIALIZZA LE FREE LIST PER OGNI LIVELLO
  for (int i=0; i<MAX_LEVELS; ++i) {
    List_init(alloc->free+i);
  }
  */

  // we allocate a list_item to mark that there is one "materialized" list
  // in the first block
  // BuddyAllocator_createListItem(alloc, 1, 0);                                               // QUA CREA PRIMO BLOCCO E LO INSERISCE NELLA FREE LIST
};


BuddyListItem* BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level){
  if (level<0)
    return 0;
  assert(level <= alloc->num_levels);


  /*
  if (! alloc->free[level].size ) { // no buddies on this level                               // VERIFICA SE LA FREE LIST A QUEL LIVELLO E' VUOTA
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
  */
  //                                                                                                  // CALCOLO GLI'INDICI DI QUESTO LIVELLO
  int level_start = (1 << level) - 1;
  int level_end = (1 << (level + 1)) - 2;

  for (int idx = level_start; idx <= level_end; ++idx) {
    if (!BitMap_bit(&alloc->bitmap, idx)) {                                                            // SE TROVO UN BUDDY LIBERO
      BuddyListItem* item = BuddyAllocator_createListItem(alloc, idx, 0);
      return item;
    }
  }

  //                                                                                                   // NO BLOCCHI LIBERI NEL LIVELLO, RISALGO
  BuddyListItem* parent_ptr = BuddyAllocator_getBuddy(alloc, level - 1);
  if (! parent_ptr)
    return 0;

  // BUDDY GENITORE VUOTO, LO SPLITTO IN DUE
  int left_idx = parent_ptr->idx * 2 + 1;
  int right_idx = buddyIdx(left_idx);

  printf("split l:%d, left_idx: %d, right_idx: %d\r", level, left_idx, right_idx);

  BuddyListItem* left_ptr = BuddyAllocator_createListItem(alloc, left_idx, parent_ptr);
  BuddyListItem* right_ptr = BuddyAllocator_createListItem(alloc, right_idx, parent_ptr);

  // imposta puntatori buddy
  left_ptr->buddy_ptr = right_ptr;
  right_ptr->buddy_ptr = left_ptr;

  // libera il parent e uno dei figli (right), teniamo solo left
  BuddyAllocator_destroyListItem(alloc, parent_ptr);
  BuddyAllocator_destroyListItem(alloc, right_ptr);

  return left_ptr;
}



void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, BuddyListItem* item){

  BuddyListItem* parent_ptr=item->parent_ptr;
  BuddyListItem *buddy_ptr=item->buddy_ptr;
  
  // buddy back in the free list of its level
  List_pushFront(&alloc->free[item->level],(ListItem*)item);                                    // REINSERISCE IL BLOCCO NELLA LISTA LIBERA

  // if on top of the chain, do nothing
  if (! parent_ptr)
    return;
  
  // if the buddy of this item is not free, we do nothing
  if (buddy_ptr->list.prev==0 && buddy_ptr->list.next==0)                                         // VERIFICA SE IL BUDDY E' NELLA LISTA LIBERA
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
