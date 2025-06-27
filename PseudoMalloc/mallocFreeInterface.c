#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h> 
#include "mallocFreeInterface.h"





void* disastrOS_malloc(BuddyAllocator* alloc, int size) {

    long PAGESIZE = getpagesize();

    int memoriaRichiesta = size + sizeof(int);

    void* puntatoreMemoria;

    if (memoriaRichiesta < (PAGESIZE/4)) { // caso buddyAllocator
        printf("Allocazione tramite BuddyAllocator\n");
        puntatoreMemoria = BuddyAllocator_malloc(alloc, memoriaRichiesta);
        
        if (puntatoreMemoria == NULL) {
            return NULL;
        }
    }
    else { // Caso mmap
        printf("Allocazione tramite mmap\n");
        memoriaRichiesta = ((memoriaRichiesta + PAGESIZE - 1) / PAGESIZE) * PAGESIZE;
        puntatoreMemoria = mmap(NULL, memoriaRichiesta, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }



    *((int*)puntatoreMemoria) = memoriaRichiesta;
    puntatoreMemoria = puntatoreMemoria + sizeof(int);

    char c = 'A' + (memoriaRichiesta % 26);

    printf("Inserimento del carattere di padding.\n");
    memset(puntatoreMemoria, c, (memoriaRichiesta-sizeof(int)));


    printf("Blocco di memoria: ");
    char *pPrint = (char*)puntatoreMemoria;
    for (int i = 0; i < (memoriaRichiesta-sizeof(int)); ++i) {
        printf("%c", pPrint[i]);
    }
    printf("\n");
    fflush(stdout);

    return puntatoreMemoria;
}




void disastrOS_free(BuddyAllocator* alloc, void* puntatore) {
    long PAGESIZE = getpagesize();

    void* puntatoreTemp = puntatore - sizeof(int);

    int memoriaRichiesta = *((int*)puntatoreTemp);

    printf("memoriaRichiesta letta: %d\n", memoriaRichiesta);

    char c = 'A' + (memoriaRichiesta % 26);

    const char* pControllo = puntatore;
    printf("Controllo valori del carattere di padding nel blocco di memoria.\n");
    printf("Blocco di memoria letto: ");
    for (int i = 0; i < (memoriaRichiesta - sizeof(int)); i++) {
        printf("%c", pControllo[i]);
        if (pControllo[i] != c) {
            printf("\nNON HO IL CARATTERE CHE DOVREI AVERE NELLA POSIZIONE i = %d\n", i);
            printf("Carattere: %c\n", pControllo[i]);
            return;
        }
    }
    printf("\n");
    fflush(stdout);

    printf("\nControllo superato con successo, dealloco\n");
    if (memoriaRichiesta < (PAGESIZE/4)) { // Caso buddyAllocator
        printf("Free tramite BuddyAllocator\n");
        BuddyAllocator_free(alloc, puntatoreTemp);
    }
    else { // Caso mmap
        printf("Free tramite munmap\n");
        munmap(puntatoreTemp, memoriaRichiesta);
    }
    return;
}