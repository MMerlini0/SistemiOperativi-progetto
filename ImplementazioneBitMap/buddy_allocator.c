#include <stdio.h>
#include <assert.h>
#include <math.h> // for floor and log2
#include "buddy_allocator.h"

// these are trivial helpers to support you in case you want
// to do a bitmap implementation
// LE FUNZIONI DATE DAL PROF CONSIDERAVANO UN ALBERO CON INDICE DI PARTENZA 1
int levelIdx(size_t idx){ // RIPORTA IL LIVELLO DELL'INDICE
  return (int)floor(log2(idx+1));
};


// LA FUNZIONE RIPORTA L'INDICE DEL BLOCCO FRATELLO
int buddyIdx(int idx) { 
  // CASO DELLA RADICE
  if (idx == 0) { return 0;}
  // CONTROLLO SE L'INDICE DATO E' PARI
  // CIOE' SE E' UN FIGLIO SINISTRO O DESTRO
  if (idx&0x1){ // RIPORTA 1 SE idx E' DISPARI -> idx E' UN FIGLIO DI SX
    return idx+1; // NEL CASO DI idx SINISTRO, COMPAGNO = DX
  }
  return idx-1; // SE E'DX RIPORTO IL SX
}

int parentIdx(int idx){ // RIPORTA L'INDICE DEL PADRE
  if (idx == 0) return -1; // CASO DELLA RADICE
  return (idx - 1) / 2;
}


  // Il primo indice del livello l è sempre 2^lvl - 1.
  // return (idx-(1<<levelIdx(idx)));
int firstIdx(int level) { // PRIMO INDICE DEL LIVELLO
  return (1 << level)-1; //2^level -1
}

// RESITTUISCE L'OFFSET DELL'INDICE PASSATO RISPETTO AL PRIMO INDICE DI QUEL LIVELLO
int startIdx(int idx){
  return (idx-(firstIdx(levelIdx(idx))));
}



// SETTA I BIT DEI PARENTI DELL'INDICE PASSATO
void BitMap_setParentsBit(BitMap *bit_map, int bit_num, int status){
  while (bit_num >= 0) {
    BitMap_setBit(bit_map, bit_num, status);
    bit_num = parentIdx(bit_num);
  }
}
// SETTA I BIT DEI FIGLI DELL'INDICE PASSATO
void BitMap_setChildrensBit(BitMap *bit_map, int bit_num, int status) {
    int stack[bit_map->num_bits];  // Stack esplicito per simulare la ricorsione
    int top = 0;

    stack[top++] = bit_num;        // Inizia dal nodo di partenza

    while (top > 0) {
        int current = stack[--top];  // Estrai un nodo dallo stack

        if (current >= bit_map->num_bits)
            continue;                // Ignora se è fuori dai limiti della bitmap

        BitMap_setBit(bit_map, current, status); // Imposta il bit a 0 o 1

        // Calcola gli indici dei figli
        int left = 2 * current + 1;
        int right = 2 * current + 2;

        // Push dei figli nella lista simula-stack (prima destro per visitare sinistro prima)
        if (right < bit_map->num_bits)
            stack[top++] = right;
        if (left < bit_map->num_bits)
            stack[top++] = left;
    }
}










/*
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
}; */



/*
// detaches and destroys an item in the free lists 
void BuddyAllocator_destroyListItem(BuddyAllocator* alloc, BuddyListItem* item){
  int level=item->level;
  List_detach(&alloc->free[level], (ListItem*)item);
  printf("Destroying Item. level:%d, idx:%d, start:%p, size:%d\n",
        item->level, item->idx, item->start, item->size);
  PoolAllocatorResult release_result=PoolAllocator_releaseBlock(&alloc->list_allocator, item);
  assert(release_result==Success);

}; */




// FUNZIONE DI CALCOLO DIMENSIONE BUFFER 
int BufferSizeCalculator(int num_levels) {
    // CALCOLO BITS PER LA BITMAP
    int num_bits = (1 << (num_levels + 1)) - 1 ;  //n.bit = (2^(num_levels+1))-1
    int num_bytes = BitMap_getBytes(num_bits);
    // CONTROLLI SU NUMERO LIVELLI E CAPACITA MEMORIA

    return num_bytes;
}


// USO UN UNICO BUFFER, CHE DIVIDERO' IN DUE PARTI, L'INIZIO PER LA BITMAP E IL RESTO PER L'ALLOCATOR
int BuddyAllocator_init(BuddyAllocator* alloc,
                        int num_levels,
                        char* bufferBitmap,
                        int bufferBitmap_size,
                        char* memory, // BUFFER E' LA MEMORIA, E' QUANTO STO ALLOCANDO MANNAGGIA
                        int memory_size,
                        int min_bucket_size) {

  
  if (min_bucket_size < 8) { // PIU PICCOLO DI UN BYTE
      printf("Minimum bucket troppo piccolo\n");
      return 0;
  }

  
  // CALCOLO BITS PER LA BITMAP
  int num_bits = (1 << (num_levels + 1)) - 1 ;  //n.bit = (2^(num_levels+1))-1
  int num_bytes = BitMap_getBytes(num_bits);
  // CONTROLLI SU NUMERO LIVELLI E CAPACITA MEMORIA

  if(num_levels>MAX_LEVELS){
    printf("NUMERO DI LIVELLI MAGGIORE DI 16, INIZIALIZZAZIONE BUDDY ALLOCATOR TERMINATA");
    return 0;
  }
  if(bufferBitmap_size < num_bytes){
    printf("MEMORIA INSUFFICIENGTE PER LA BITMAP");
    return 0;
  }

  // we need room also for level 0
  alloc->num_levels=num_levels;
  alloc->memory=memory;
  alloc->memory_size = memory_size;
  alloc->min_bucket_size=min_bucket_size;


  /*
  int list_items=1<<(num_levels+1); // maximum number of allocations, used to size the list
  int list_alloc_size=(sizeof(BuddyListItem)+sizeof(int))*list_items;
  */

  printf("BUDDY INITIALIZING\n");
  printf("levels: %d \n", num_levels);
  printf("Memoria per la BitMap: %d\n", num_bytes);
  printf("Bytes forniti (buffer): %d \n", bufferBitmap_size);
  printf("min_bucket size:%d\n", min_bucket_size);
  printf("Memoria gestita: %d bytes\n", (1<<num_levels)*min_bucket_size);



  // INIZIALIZZO LA BITMAP
  BitMap_init(&alloc->bitmap, num_bits, bufferBitmap);

  Bitmap_print(&alloc->bitmap);
  
  return 1;


  /*
  // the buffer for the list starts where the bitmap ends
  char *list_start=buffer;
  PoolAllocatorResult init_result=PoolAllocator_init(&alloc->list_allocator,
              sizeof(BuddyListItem),
              list_items,
              list_start,
              list_alloc_size);
  printf("%s\n",PoolAllocator_strerror(init_result));

  // we initialize all lists
  for (int i=0; i<MAX_LEVELS; ++i) {
    List_init(alloc->free+i);
  }

  // we allocate a list_item to mark that there is one "materialized" list
  // in the first block
  BuddyAllocator_createListItem(alloc, 1, 0);
  */
};

/*
BuddyListItem* BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level){
  if (level<0)
    return 0;
  assert(level <= alloc->num_levels);

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
} */























//allocates memory
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size) {
  

  // VOGLIO ALLOCARE size BYTE + sizeof(int) BYTE PER L'INDICE DELLA BITMAP
  size += sizeof(int);

  
  int max_size = alloc->memory_size;
  int min_size = alloc->min_bucket_size;
  int total_levels = alloc->num_levels;

  //CONTROLLO DIMENSIONE RICHIESTA ALLOCAZIONE  
  if (max_size < size) {
      printf("\n max_size: %d, size: %d\n", max_size, size);
      printf("\nEXEDED MAX DIMENSION, MEMORY FAULT.\n");
      return NULL;
  }


  // DETERMINO IL LIVELLO DEL BLOCCO

  // SFRUTTO PROPRIETA' ALBERI
  // CERCO IL LIVELLO l TALE CHE block_size = min_size * 2^(num_levels - l) >= size
  int target_level = total_levels - (int)ceil(log2((double)size / min_size)); // lo arrotondo per eccesso

  // MI DISTINGUO NEI CASI DI LIVELLO MINIMO E MASSIMO
  if (target_level < 0) target_level = 0;
  if (target_level > total_levels) target_level = total_levels;

  printf("\n Livello da assegnare: %d", target_level);

  // CERCO UN BLOCCO LIBERO NEL LIVELLO TROCATO
  int freeidx=-1;  //free rimarrà -1 se non troviamo nessun blocco libero, altrimenti sarà l'indice del nostro blocco libero


  // CASO PRIMO LIVELLO
  if ( target_level == 0) {
    // CONTROLLO SE LIBERO
    if (!BitMap_bit(&alloc->bitmap, firstIdx(target_level))) {
      freeidx = 0;
    }
    // CASO IN CUI NON E' LIBERO
    else {
      printf("\nNon ci sono blocchi liberi. MEMORY FAULT. \n");
      return NULL;
    }
  }
  // SE NON E' IL PRIMO LIVELLO
  else {
    // RICORSIONE SU TUTTI I LIVELLI DI QUESTO BLOCCO FINO A QUELLI DEL SUCCESSIVO
    for (int j = firstIdx(target_level); j < firstIdx(target_level+1); j++) {
      // CONTROLLO SE LIBERO
      if (!BitMap_bit(&alloc->bitmap, j)) {
        printf("\nTrovato blocco libero nel livello %d\n", j);
        freeidx=j;
        break;
      }
    }
  }

  // CONTROLLO SE NON HA TROVATO BLOCCHI LIBERI
  if(freeidx == -1){  
    printf("\nNon ci sono blocchi liberi nel livello %d\n", target_level);
    // CONTROLLA TUTTI! I BIT DEL LIEVLLO, QUINDI ANCHE QUELLI NON SPLITTATI, RICERCA FRA TUTTI
    // I POSSIBILI SPAZI DI QUESTO LIEVLLO
    return NULL;
  }


  // SE HO TROVATO UN INDICE LIBERO, DOVRO' SEGNARE TUTTI I SUOI POSSIBILI FIGLI AD OCCUPATI
  // PERCHE' BLOCCO SUPERIORE USATO
  // INLTRE DEVO SETTARE ANCHE TUTTI I BLOCCHI PADRE AD OCCUPATI
  // OVVIAMENTE ANCHE L' INDICE TROVATO VERRA SETTATO AD OCCUPATO

  // TODO: non ho settato questo bit della bitmap ad uno, non so se lo fa uno delle funzioni chiamate sotto, stai attento
  // SETTA BIT GENITORI E DEI FIGLI
  BitMap_setParentsBit(&alloc->bitmap, freeidx, 1);
  BitMap_setChildrensBit(&alloc->bitmap, freeidx, 1);

  // MI PRENDO IL VALORE DELLA DIMENSIONE DI MEMORIA AL LIVELLO TROVATO
  int memory_size_at_level = alloc->min_bucket_size << (alloc->num_levels - target_level);

  // RIPORTO L'INDIRIZZO ALLA MEMORIA ALLOCATA, ASSEGNANDO E TENENDO CONTO IL byte PER IL BOOKKEPING
  // RICORDO CHE GIE NELLA size RICHIESTA ALL'INIZIO VI AVEVO CONSIDERATO IL + sizeof(int)
  char *indirizzo = alloc->memory + startIdx(freeidx) * memory_size_at_level;
  // CI METTO L'INDICE DEL BLOCCO
  ((int *)indirizzo)[0]=freeidx;
  printf("\nAllocato un nuovo blocco di dimensione %d al livello %d utilizzando un blocco di dimensioni %d.\nIndice %d,puntatore \033[34m%p\033[0m\n", size, target_level, memory_size_at_level, freeidx, indirizzo+sizeof(int));
  printf("Albero BITMAP dopo l'allocazione:\n");
  
  
  Bitmap_print(&alloc->bitmap);
  // RIPORTO IL BLOCCO, MA SALTANDO IL PRIMO BYTE CHE E' PER IL BOOKKEPING
  return (void *)(indirizzo + sizeof(int));
};













  //FREE
  void BuddyAllocator_free(BuddyAllocator *alloc, void *mem){
  
  if (mem==NULL){
    printf("MEMORY FAULT, BLOCCO SU CUI RICHIESTA FREE = NULL");
    return;
  }
  
  printf("\nLibero il blocco puntato da \033[34m%p\033[0m . . .\n", mem);

  // PRENDO L'INDICE DEL BUDDY PRESO IN CONSIDERAZIONE
  // IL PRIMO ELEMENTO DEL BUCKET E' L'INDICE
  int *primoElem = (int *)mem;
  int indice = primoElem[-1];
  printf("Indice da liberare: %d\n", indice);

  //dopo aver liberato un blocco dobbiamo settare a 0 tutti i suoi discendendi
  BitMap_setChildrensBit(&alloc->bitmap, indice, 0);
  // RICHIAMO LA FUNZIONE DI MERGE IN CASO AVESSE FRATELLO NON USATO
  merge(&alloc->bitmap, indice);

  //printf("Bitmap dopo la free:");
  Bitmap_print(&alloc->bitmap);
}


















// UNISCE I BUDDY FINO AL LIEVLLO PIU ALTO POSSIBILE
void merge(BitMap *bitmap, int idx) {
    assert("Non puoi fare il merge su un bit libero" && !BitMap_bit(bitmap, idx));

    while (idx != 0) { // finché non siamo alla radice

        int indice_fratello = buddyIdx(idx); // PRENDO INDICE DEL FRATELLO

        // SE IL BIT E' LIBERO
        if (!BitMap_bit(bitmap, indice_fratello)) {
            printf("Il buddy di %d, ovvero %d, e' libero: MERGE\n", idx, indice_fratello);
            printf("Eseguo il merge dei buddy %d e %d al livello %d . . .\n", idx, indice_fratello, levelIdx(idx));

            // SETTO IL GENITORE A LIBERO
            int indice_genitore = parentIdx(idx);
            BitMap_setBit(bitmap, indice_genitore, 0);
            // RISALGO AL GENITORE
            idx = indice_genitore;
        } else {
            printf("Il buddy di %d, ovvero %d, NON è libero: NO MERGE\n", idx, indice_fratello);
            break;
        }
    }
}


  /*
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
*/
  




// FUNZIONE DI PRINT CON RICORSIONE, DI DEBUGGING
void Bitmap_print(BitMap *bit_map){
    int remain_to_print = 0;
    int lvl = -1; 
    int tot = levelIdx(bit_map->num_bits) - 1;  //numero di livelli totale
    for (int i = 0; i < bit_map->num_bits; i++){  
        if (remain_to_print == 0){ //se non rimangono bit da stampare al livello lvl
            if(lvl==tot){ //se siamo arrivati all'ultimo livello stop
              break;
            } 
            printf("\n\033[93mLivello %d: \t\033[0m", ++lvl);     //indice del primo elemento del livello: i
            for (int j = 0; j < (1 << tot) - (1 << lvl); j++){   //stampa degli spazi dopo aver scritto "Livello x:"
              printf(" "); //stampa spazi
            } 
            remain_to_print = 1 << lvl; //al prossimo livello dovremo stampare 2^lvl bit
        }
        if (BitMap_bit(bit_map, i)==0){ //se il blocco è 0 lo stampiamo verde
          printf("\033[32m%d\033[0m ", BitMap_bit(bit_map, i));
        }
        else{   //altrimenti lo stampiamo rosso
          printf("\033[31m%d\033[0m ", BitMap_bit(bit_map, i));
        }
        remain_to_print--;  //1 bit in meno da stampare
    }
    printf("\n");
};
