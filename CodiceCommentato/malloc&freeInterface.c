#define _GNU_SOURCE
#include <string.h>
#include <unistd.h>
#include <sys/mman.h> 
#include "malloc&freeInterface.h"





void* disastrOS_malloc(BuddyAllocator *alloc, int size) {
    // Mi prendo la size di una pagina
    long PAGESIZE = getpagesize();

    // Aggiungo alla mia richiesta sizeof(int) per poterci inserire size richiesta
    int memoriaRichiesta = size + sizeof(int);

    // Inizializzo puntatore
    void * puntatoreMemoria;

    // Richiedo l'appropriata funzione (buddy o mmap) con questa nuova size
    if (memoriaRichiesta < (PAGESIZE/4)) { // Caso buddyAllocator
        printf("Allocazione tramite BuddyAllocator\n");
        puntatoreMemoria = BuddyAllocator_malloc(&alloc, memoriaRichiesta);
        // Controllo errore nell'allocazione malloc
        if (puntatoreMemoria == NULL) {
            return NULL;
        }
    }
    else { // Caso mmap
        printf("Allocazione tramite mmap\n");
        // Arrotondo la dimensione richiesta al multiplo maggiore della dim. pagina
        int memoriaRichiesta_arrotondata = ((memoriaRichiesta + PAGESIZE - 1) / PAGESIZE) * PAGESIZE;
        puntatoreMemoria = mmap(NULL, memoriaRichiesta_arrotondata, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }



    // Aggiungo memoriaRichiesta in prima posizione del blocco di memoria
    *((int*)puntatoreMemoria) = memoriaRichiesta;
    // Sposto il puntatore a dopo il valore inserito
    puntatoreMemoria = puntatoreMemoria + sizeof(int);

    // Calcolo di un carattere deterministico con il quale sporcare la memoria per controllo
    // Uso un carattere alfabetico da A a Z, prendendo il resto della divisione della memoriaRichiesta con 26
    char c = 'A' + (memoriaRichiesta % 26);
    // Riempio il resto della memoria con questo carattere
    memset(puntatoreMemoria, size, c);
}




void disastrOS_free(BuddyAllocator *alloc) {

}