#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h> 
#include "mallocFreeInterface.h"





void* disastrOS_malloc(BuddyAllocator* alloc, int size) {
    // Mi prendo la size di una pagina
    long PAGESIZE = getpagesize();

    // Aggiungo sizeof(int) alla mia richiesta per poterci inserire size richiesta
    int memoriaRichiesta = size + sizeof(int);

    // Inizializzo puntatore
    void* puntatoreMemoria;

    // Richiedo l'appropriata funzione (buddy o mmap) con questa nuova size
    if (memoriaRichiesta < (PAGESIZE/4)) { // Caso buddyAllocator
        printf("Allocazione tramite BuddyAllocator\n");
        puntatoreMemoria = BuddyAllocator_malloc(alloc, memoriaRichiesta);
        // Controllo errore nell'allocazione malloc
        if (puntatoreMemoria == NULL) {
            return NULL;
        }
    }
    else { // Caso mmap
        printf("Allocazione tramite mmap\n");
        // Arrotondo la dimensione richiesta al multiplo maggiore della dim. pagina
        puntatoreMemoria = mmap(NULL, memoriaRichiesta, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }



    // Aggiungo memoriaRichiesta in prima posizione del blocco di memoria
    *((int*)puntatoreMemoria) = memoriaRichiesta;
    printf("memoriaRichiesta scritta (interface): %d\n", memoriaRichiesta);
    // Sposto il puntatore a dopo il valore inserito
    puntatoreMemoria = puntatoreMemoria + sizeof(int);

    // Calcolo di un carattere deterministico con il quale sporcare la memoria per controllo
    // Uso un carattere alfabetico da A a Z, prendendo il resto della divisione della memoriaRichiesta con 26
    char c = 'A' + (memoriaRichiesta % 26);
    

    // Riempio il resto della memoria con questo carattere
    printf("Inserimento del carattere di padding.\n");
    memset(puntatoreMemoria, c, (memoriaRichiesta-sizeof(int)));



    // Printo la memoria
    printf("Blocco di memoria: ");
    char *pPrint = (char*)puntatoreMemoria;
    for (int i = 0; i < (memoriaRichiesta-sizeof(int)); ++i) {
        printf("%c", pPrint[i]);
    }
    printf("\n");
    fflush(stdout);

    // RItorno il puntatore
    return puntatoreMemoria;
}




void disastrOS_free(BuddyAllocator* alloc, void* puntatore) {
    // Mi prendo la size di una pagina
    long PAGESIZE = getpagesize();

    // riporto il puntatore alla posizione vera
    void* puntatoreTemp = puntatore - sizeof(int);

    // Mi prendo il valore della memoriaRichiesta
    int memoriaRichiesta = *((int*)puntatoreTemp);

    printf("memoriaRichiesta letta (interface): %d\n", memoriaRichiesta);


    // Calcolo il valore che dovrei trovarci
    char c = 'A' + (memoriaRichiesta % 26);

    // Controllo se è presente il valore corretto:
    // Mi prendo un puntatore copia
    const char* pControllo = puntatore;
    // Itero per ogni posizione della memoria
    printf("Controllo valori del carattere di padding nel blocco di memoria.\n");
    printf("Blocco di memoria letto: ");
    for (int i = 0; i < (memoriaRichiesta - sizeof(int)); i++) {
        printf("%c", pControllo[i]);
        // Controllo se non è presente quel carattere
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
        printf("Free tramite mmap\n");
        munmap(puntatoreTemp, memoriaRichiesta);
    }
    return;
}