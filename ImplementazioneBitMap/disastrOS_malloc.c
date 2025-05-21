
#include <stdio.h>

#include "buddy_allocator_test.h"

#define TOTAL_MEMORY_SIZE (1024*1024) // LA MEMORY SIZE INIZIALE E' SEMPRE DI 1MB COME DETTO NELLE SPECIFICHE DI PROGETTO

char memoria_totale[TOTAL_MEMORY_SIZE]; // LISTA CHE SIMULA LA MIA MEMORIA TOTALE PER IL BUDDY ALLOCATOR
                                        // LISTA DI 1MB 



int main(int argc, char *argv[]) {


    // QUANDO FACCIO MALLOC CHISSENE RICEVO PUNTATORE
    // QUANDO DEVO FARE FREE DEVO CAPIRE SE SONO IN mmap O IN BuddyAllocator



    // MI METTO SOLO IN CASO BUDDY ALLOCATOR

    return BuddyAllocator_alloc(memoria_totale);
}