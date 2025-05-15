#include "buddy_allocator.h"
#include <stdio.h>

#define BUFFER_SIZE 102400
#define BUDDY_LEVELS 9
#define MEMORY_SIZE (1024*1024)
#define MIN_BUCKET_SIZE (MEMORY_SIZE>>(BUDDY_LEVELS))

char buffer[BUFFER_SIZE]; // 100 Kb buffer to handle memory should be enough
char memory[MEMORY_SIZE];

BuddyAllocator alloc;


int main(int argc, char** argv) {
  

  /*
  //1 we see if we have enough memory for the buffers
  int req_size=BuddyAllocator_calcSize(BUDDY_LEVELS);
  printf("size requested for initialization: %d/BUFFER_SIZE\n", req_size);
  */

  //2 we initialize the allocator
  printf("init... ");
  int costruttore = BuddyAllocator_init(&alloc,
                      BUDDY_LEVELS,
                      buffer,
                      BUFFER_SIZE,
                      memory,
                      MEMORY_SIZE,
                      MIN_BUCKET_SIZE);

  if (costruttore==0){
    printf("Qualcosa è andato storto nell'inizializzazione del Buddy Allocator");
    return 0;
  }
  printf("DONE\n");

  void* p1=BuddyAllocator_malloc(&alloc, 100);
  BuddyAllocator_free(&alloc, p1);

  void* p2=BuddyAllocator_malloc(&alloc, 14);
  BuddyAllocator_free(&alloc, p2);


  void* p3=BuddyAllocator_malloc(&alloc, 100000);
  BuddyAllocator_free(&alloc, p3);
  
}
