#include "buddy_allocator.h"
#include <stdio.h>

// #define BUFFER_SIZE 102400
#define TOTAL_MEMORY_SIZE (1024*1024) // LA MEMORY SIZE INIZIALE E' SEMPRE DI 1MB COME DETTO NELLE SPECIFICHE DI PROGETTO

BuddyAllocator alloc;


int BuddyAllocator_alloc(char* memoria_totale) {
  

  // RICHIESTA PER UTENTE NUMERO LIVELLI NEL BUDDY
  printf("Scegliere numero livelli massimi per il buddyAllocator: \n");
  int BUDDY_LEVELS;
  scanf("%d", &BUDDY_LEVELS);



  // CALCOLO DIMENSIONE DEL BUFFER
  int BUFFER_SIZE = BufferSizeCalculator(BUDDY_LEVELS);

  
  // SUDDIVISIONE DELLA MEMORIA TOTALE IN ZONA BUFFER (per bitmap) E ZONA DI MEMORIA
  int MEMORY_SIZE = TOTAL_MEMORY_SIZE - BUFFER_SIZE;
  printf("MEMORY SIZE: %d\n BUFFER SIZE: %d\n", MEMORY_SIZE, BUFFER_SIZE);
  char* buffer = memoria_totale;                    // inizia da inizio array
  char* memory = memoria_totale + BUFFER_SIZE;      // parte dopo la bitmap



  // CALCOLO DIMENSIONE MINIMA BUCKET
  int MIN_BUCKET_SIZE =  (MEMORY_SIZE>>(BUDDY_LEVELS));

  /*
  //1 we see if we have enough memory for the buffers
  int req_size=BuddyAllocator_calcSize(BUDDY_LEVELS);
  printf("size requested for initialization: %d/BUFFER_SIZE\n", req_size);
  */

  //2 we initialize the allocator
  printf("init... \n");
  int costruttore = BuddyAllocator_init(&alloc,
                      BUDDY_LEVELS,
                      buffer,
                      BUFFER_SIZE,
                      memory,
                      MEMORY_SIZE,
                      MIN_BUCKET_SIZE);
    
  if (costruttore==0){
    printf("Qualcosa è andato storto nell'inizializzazione del Buddy Allocator\n");
    return 0;
  }
  printf("DONE\n");

  void* p1=BuddyAllocator_malloc(&alloc, 100);
  BuddyAllocator_free(&alloc, p1);

  void* p2=BuddyAllocator_malloc(&alloc, 14);
  BuddyAllocator_free(&alloc, p2);


  void* p3=BuddyAllocator_malloc(&alloc, 100000);
  BuddyAllocator_free(&alloc, p3);


  return 1;
  
}
