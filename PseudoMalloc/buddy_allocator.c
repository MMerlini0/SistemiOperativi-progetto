#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "buddy_allocator.h"



// Raccolta di funzioni ausiliarie per la bitmap, che sfruttano le propriet' dell'albero binario


// Riporta il livello a cui mi trovo dell'indice (posizione)
int levelIdx(size_t idx){
  return (int)floor(log2(idx+1));
};


// Funzione che, dato un indice, ne riporta l'indice del fratello
int buddyIdx(int idx) { 
  if (idx == 0) { return 0;}
  if (idx&0x1) {
    return idx+1;
  }
  return idx-1;
}

// Funzione che, dato un indice, ne riporta l'indice del padre
int parentIdx(int idx){
  if (idx == 0) return -1;
  return (idx - 1) / 2;
}

// Funzione che, dato un livello, ne riporta l'indice del primo figlio
int firstIdx(int level) { 
  return (1 << level)-1;
}

// Funzione che, dato un indice, riporta la differenza in posizioni fra l'indice passato e il primo
// indice del livello in cui si trova (l'indice passato)
int startIdx(int idx){
  return (idx-(firstIdx(levelIdx(idx))));
}

// Funzione che, dato un indice e uno stato status (0-1), setta quel bit a status e ripete per tutti i genitori
void BitMap_setParentsBit(BitMap *bit_map, int bit_num, int status){
  while (bit_num >= 0) {
    BitMap_setBit(bit_map, bit_num, status);
    bit_num = parentIdx(bit_num);
  }
}

// Funzione che, dato un indice e uno status, setta l'indice e tutti i suoi figli a status
void BitMap_setChildrensBit(BitMap *bit_map, int bit_num, int status) {
  if (bit_num >= bit_map->num_bits) return;
  BitMap_setBit(bit_map, bit_num, status);
  int figlioSx = 2 * bit_num + 1;
  BitMap_setChildrensBit(bit_map, figlioSx, status);
  int figlioDx = 2 * bit_num + 2;
  BitMap_setChildrensBit(bit_map, figlioDx, status);

}
// Fine delle funzioni ausiliarie per l'implementazione a bitmap




// Funzione che, dato il numero di livelli del buddy, calcola la dimensione del buffer
int BufferSizeCalculator(int num_levels) {
    int num_bits = (1 << (num_levels + 1)) - 1 ;
    int num_bytes = BitMap_getBytes(num_bits);
    return num_bytes;
}


// Funzione di inizializzazione del buddyAllocator
int BuddyAllocator_init(BuddyAllocator* alloc,
                        int num_levels,
                        char* bufferBitmap,
                        int bufferBitmap_size,
                        char* memory,
                        int memory_size,
                        int min_bucket_size) {

  if (min_bucket_size < 1) { 
      printf("Minimum bucket troppo piccolo\n");
      return 0;
  }
  int num_bits = (1 << (num_levels + 1)) - 1 ;
  int num_bytes = BitMap_getBytes(num_bits);
  if(bufferBitmap_size < num_bytes){
    printf("MEMORIA INSUFFICIENGTE PER LA BITMAP");
    return 0;
  }

  alloc->num_levels=num_levels;
  alloc->memory=memory;
  alloc->memory_size = memory_size;
  alloc->min_bucket_size=min_bucket_size;

  printf("BUDDY INITIALIZING\n");
  printf("levels: %d \n", num_levels);
  printf("Memoria per la BitMap: %d\n", num_bytes);
  printf("Bytes forniti (buffer): %d \n", bufferBitmap_size);
  printf("min_bucket size:%d\n", min_bucket_size);
  printf("Memoria gestita: %d bytes\n", (1<<num_levels)*min_bucket_size);


  BitMap_init(&alloc->bitmap, num_bits, bufferBitmap);
  Bitmap_print(&alloc->bitmap);
  return 1;

};







// Funzione per allocazione della memoria
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size) {
  

  size += sizeof(int);

  
  int max_size = alloc->memory_size;
  int min_size = alloc->min_bucket_size;
  int total_levels = alloc->num_levels;
 
  if (max_size < size) {
      printf("\n max_size: %d, size: %d\n", max_size, size);
      printf("\nEXEDED MAX DIMENSION, MEMORY FAULT.\n");
      return NULL;
  }


  int target_level = total_levels - (int)ceil(log2((double)size / min_size));


  if (target_level < 0) target_level = 0;
  if (target_level > total_levels) target_level = total_levels;

  printf("\nLivello da assegnare: %d", target_level);


  int freeidx=-1;


  if ( target_level == 0) {
    if (!BitMap_bit(&alloc->bitmap, firstIdx(target_level))) {
      freeidx = 0;
    }
    else {
      printf("\nNon ci sono blocchi liberi. MEMORY FAULT. \n");
      return NULL;
    }
  }
  else {
    for (int j = firstIdx(target_level); j < firstIdx(target_level+1); j++) {
      if (!BitMap_bit(&alloc->bitmap, j)) {
        printf("\nTrovato blocco libero nel livello %d\n", j);
        freeidx=j;
        break;
      }
    }
  }

  if(freeidx == -1){  
    printf("\nNon ci sono blocchi liberi nel livello %d\n", target_level);
    return NULL;
  }


  BitMap_setParentsBit(&alloc->bitmap, freeidx, 1);
  BitMap_setChildrensBit(&alloc->bitmap, freeidx, 1);

  int memory_size_at_level = alloc->min_bucket_size << (alloc->num_levels - target_level);


  char *indirizzo = alloc->memory + startIdx(freeidx) * memory_size_at_level;
  ((int *)indirizzo)[0]=freeidx;
  printf("\nAllocao blocco di memoria");
  printf("\nDimensione richiesta: %d",size);
  printf("\nLivello blocco: %d", target_level);
  printf("\nDimensione blocco usato: %d", memory_size_at_level);
  printf("\nIndice del livello: %d", freeidx);
  printf("\nPuntatore ricevuto: %p",indirizzo+sizeof(int));
  printf("\nAlbero BITMAP dopo l'allocazione:\n");
  Bitmap_print(&alloc->bitmap);

  return (void *)(indirizzo + sizeof(int));
};


// Funzione di free
void BuddyAllocator_free(BuddyAllocator *alloc, void *mem){
  
  if (mem==NULL){
    printf("MEMORY FAULT, BLOCCO SU CUI RICHIESTA FREE = NULL");
    return;
  }
  
  printf("\nLibero il blocco puntato da %p\n", mem);

  int *primoElem = (int *)mem;
  int indice = primoElem[-1];
  printf("Indice da liberare: %d\n", indice);

  BitMap_setChildrensBit(&alloc->bitmap, indice, 0);

  merge(&alloc->bitmap, indice);

  Bitmap_print(&alloc->bitmap);
}


// Funzione che unisce (merge) i blocchi, se liberi, fino alla radice
void merge(BitMap *bitmap, int idx) {
    assert("Non puoi fare il merge su un bit libero" && !BitMap_bit(bitmap, idx));

    while (idx != 0) { 
        int indice_fratello = buddyIdx(idx);


        if (!BitMap_bit(bitmap, indice_fratello)) {
            printf("Il buddy di %d, ovvero %d, e' libero: MERGE\n", idx, indice_fratello);
            printf("Eseguo il merge dei buddy %d e %d al livello %d . . .\n", idx, indice_fratello, levelIdx(idx));

            int indice_genitore = parentIdx(idx);
            BitMap_setBit(bitmap, indice_genitore, 0);
            idx = indice_genitore;
        } else {
            printf("Il buddy di %d, ovvero %d, NON è libero: NO MERGE\n", idx, indice_fratello);
            break;
        }
    }
}




// FUNZIONE DI PRINT CON RICORSIONE, DI DEBUGGING
void Bitmap_print(BitMap *bit_map){
    int remain_to_print = 0;
    int lvl = -1; 
    int tot = levelIdx(bit_map->num_bits) - 1;
    for (int i = 0; i < bit_map->num_bits; i++){  
        if (remain_to_print == 0) {
            if(lvl==tot){ 
              break;
            } 
            printf("\n\033[93mLivello %d: \t\033[0m", ++lvl);    
            for (int j = 0; j < (1 << tot) - (1 << lvl); j++){   
              printf(" "); 
            } 
            remain_to_print = 1 << lvl; 
        }
        if (BitMap_bit(bit_map, i)==0){ 
          printf("\033[32m%d\033[0m ", BitMap_bit(bit_map, i));
        }
        else{   
          printf("\033[31m%d\033[0m ", BitMap_bit(bit_map, i));
        }
        remain_to_print--; 
    }
    printf("\n");
};

