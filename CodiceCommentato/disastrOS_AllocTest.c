
#define _GNU_SOURCE // Per la | MAP_ANONYMOUS

#include "buddy_allocator.h"
#include "bit_map.h"
#include "mallocFreeInterface.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>




// Il main presenta un array void** di puntatori void*, questo verra' utilizzato per immagazzinare, al fine di poter gestire
// e sperimentare, quanti puntatori si voglia (modificare il valore di NUMEROPUTATORI per poter arbitrariamente scegliere quanti puntatori gestire simultaneamente).
// Al posto di aggiungere dei semplici controlli sul valore della posizione di tale array (if array[i] == NULL)
// per non sovrascrivere o cancellare tali puntatori, ho voluto implementare la stessa meccanica usata nel buddy, basata su bitmap,
// per tenere traccia delle posizioni occupate dai miei puntatori.


// Memoria che do al buddyAllocator da gestire
#define BUDDYALLOCATOR_TOTAL_MEMORY_SIZE (1024*1024) // LA MEMORY SIZE INIZIALE E' DI 1MB, COME ASSEGNATO NELLE SPECIFICHE DI PROGETTO
// Quanti puntatori voglio 'gestire'
#define NUMEROPUNTATORI 10



// Definisco il buddyAllocator
BuddyAllocator alloc;
// Definisco la bitmap usata per tenere conto dello stato dei puntatori
BitMap bitmapInterna;


// Array di char che simula la memoria totale che il buddyAllocator gestira'
char memoria_totale[BUDDYALLOCATOR_TOTAL_MEMORY_SIZE]; // LISTA DI 1MB 




int main(int argc, char *argv[]) {
    printf("\n\n\n\n\n\n\nInizio programma...\n");
    printf("Calcolo byte richiesti per bitmap dei puntatori'\n");
    int BYTEOFNUMEROPUNTATORI = BitMap_getBytes(NUMEROPUNTATORI); // Mi prendo quandi byte occupano i puntatori (per la bitmap)
    printf("Byte per bitmap interna: %d\n", BYTEOFNUMEROPUNTATORI);
    printf("Creazione buffer interno per bitmap dei puntatori\n");
    char bufferInterno[BYTEOFNUMEROPUNTATORI]; // Creo un array di byte come buffer per la bitmap dei puntatori del main
    memset(bufferInterno, 0, BYTEOFNUMEROPUNTATORI); // azzero i valori
    printf("Creazione di un array di NUMEROPUNTATORI puntatori void* per indirizzi di memoria richiesta'\n");
    void** puntatori = (void**)malloc(NUMEROPUNTATORI * sizeof(void*));  // CREO ARRAY DI NUMEROPUNTATORI PUNTATORI A PUNTATORI VOID* E LI SETTO A VOID
    printf("Inizializzazione dell'array a NULL\n");
    for (int i = 0; i < NUMEROPUNTATORI; ++i) {
        puntatori[i] = NULL;
    }
    printf("Inizializzazione bitMap interna\n");
    BitMap_init(&bitmapInterna, NUMEROPUNTATORI, bufferInterno);
    long PAGESIZE = getpagesize();
    printf("Acquisita dimensione di pagina del sistema, dimensione: %ld\n", PAGESIZE);
    printf("\n\n------------ INIZIALIZZAZIONE BUDDY ALLOCATOR ------------\n\n");
    // RICHIESTA PER UTENTE NUMERO LIVELLI NEL BUDDY
    printf("Scegliere numero livelli massimi per il buddyAllocator: \n");
    int BUDDY_LEVELS;
    scanf("%d", &BUDDY_LEVELS);
    if (BUDDY_LEVELS>=100) {
        printf("Numero eccessivo, consiglio num liv < 30\n");
        return 0;
    }
    // CALCOLO DIMENSIONE DEL BUFFER
    int BUFFER_SIZE = BufferSizeCalculator(BUDDY_LEVELS);
    // SUDDIVISIONE DELLA MEMORIA TOTALE IN ZONA BUFFER (per bitmap) E ZONA DI MEMORIA
    int MEMORY_SIZE = BUDDYALLOCATOR_TOTAL_MEMORY_SIZE - BUFFER_SIZE;
    printf("\n\nMEMORY SIZE: %d\nBUFFER SIZE: %d\n", MEMORY_SIZE, BUFFER_SIZE);
    char* buffer = memoria_totale;                    // inizia da inizio array
    char* memory = memoria_totale + BUFFER_SIZE;      // parte dopo la bitmap
    // CALCOLO DIMENSIONE MINIMA BUCKET
    int MIN_BUCKET_SIZE =  (MEMORY_SIZE>>(BUDDY_LEVELS));
    printf("\nDimensione minima del bucket: %d\n", MIN_BUCKET_SIZE);
    // ORA CHE HO I PARAMETRI INIZIALIZZO
    printf("Inizializzazione BuddyAllocator... \n");
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
    printf("\n\n---------- DONE ----------\n\n");




    // TEST ALLOCAZIONE MEMORIA
    printf("\n\n---------- TEST ALLOCAZIONE ----------\n\n");
    int scelta = 0;
    while (1) {
        printf("(1) Per allocare  memoria, (2) Per liberare memoria, (3) Per terminare il programma\n");
        scanf("%d", &scelta);

        if (scelta == 1) {
            printf("Allocazione memoria\n\n\n\n");
            // Vedo se ho memoria libera
            int posizione = -1;
            for (int i = 0; i < NUMEROPUNTATORI; i++) { // Esce quando trova posizione
                if (!BitMap_bit(&bitmapInterna, i)) {
                    posizione=i;
                    break;
                }
            }
            // Se non ho memoria libera
            if (posizione == -1) {
                printf("Hai utilizzato tutti i puntatori, liberarne qualcuno\n");
            } else { // Se ho memoria libera
                printf("\nQuanta memoria vuoi allocare? (byte)\n");
                int memoriaRichiesta = 0;
                scanf("%d", &memoriaRichiesta);

                printf("Chiamata a funzione disastrOS_malloc\n");
                puntatori[posizione] = disastrOS_malloc(&alloc, memoriaRichiesta);

                // caso in cui fallisce il buddyAllocator_malloc e riporta NULL
                if(puntatori[posizione] == NULL) { 
                    printf("!!! Errore nell'allocazione tramite BuddyAllocator !!!\n");
                    continue;
                }

                printf("\nRicevuto blocco di memoria: %p\n", puntatori[posizione]);

                // Setto il bit della bitmap ad occupato
                BitMap_setBit(&bitmapInterna, posizione, 1);
                printf("Stato dei puntatori disponibili:\n");
                BitmapMain_print(&bitmapInterna);
            }
        }




        if(scelta == 2) { // free
            printf("Deallocazione memoria\n\n\n\n");
            printf("Stato bitmap: ");
            BitmapMain_print(&bitmapInterna);
            printf("Scegliere indice blocco memoria da liberare (prima posizione 0): ");
            int bloccoDaLiberare = 0;
            scanf("%d", &bloccoDaLiberare);
            if (bloccoDaLiberare > NUMEROPUNTATORI) {
                printf("Errore lettura numero\n");
            } else {
                if (BitMap_bit(&bitmapInterna, bloccoDaLiberare) == 0) {
                    printf("Stai provando a liberare un blocco gia libero\n");
                } else {
                    printf("Chiamo la funzione disastrOS_free\n");
                    disastrOS_free(&alloc, puntatori[bloccoDaLiberare]);
                }
            }
            // libero bit dalla bitmap, setto a NULL rispettivo indice da array puntatori
            BitMap_setBit(&bitmapInterna, bloccoDaLiberare, 0);
            puntatori[bloccoDaLiberare] = NULL;
        }



        if (scelta == 3) { // Terminazione programma        
            break;
        }
    }


    printf("----Terminazione programma----\n");
    printf("Deallocato ogni spazio di memoria\n");
    // FINCHE HO BLOCCHI DA LIBERARE LIBERO
    // CHECK SU BITMAP
    // CHIAMO FUNZIONI
    for (int i = 0; i < NUMEROPUNTATORI; i++) { // Guardo ogni posizione della bitmap
                if (BitMap_bit(&bitmapInterna, i)) { // se è occupato
                    printf("Chiamo la funzione disastrOS_free\n");
                    disastrOS_free(&alloc, puntatori[i]);
                }
            }
    printf("Deallocazione void** puntatori\n");
    free(puntatori);
    return 1;
}