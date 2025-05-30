
#define _GNU_SOURCE // Per la | MAP_ANONYMOUS

#include "buddy_allocator.h"
#include "bit_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>


// Memoria che do al buddyAllocator da gestire
#define BUDDYALLOCATOR_TOTAL_MEMORY_SIZE (1024*1024) // LA MEMORY SIZE INIZIALE E' SEMPRE DI 1MB COME DETTO NELLE SPECIFICHE DI PROGETTO
// Con quanti puntatori voglio 'giocare'
#define NUMEROPUNTATORI 10



// Definisco il buddyAllocator
BuddyAllocator alloc;
// CREO BITMAP INTERNA PERCHE' RIUSO MAGHEGGIO PER TENERE STATO MEMORIA ALLO STESSO MODO HAHAH
BitMap bitmapInterna;

// Creo array di interi che salva la memoria richiesta da ogni puntatore con il quale sto giocando
int array_memoriaRichiesta[NUMEROPUNTATORI];

// Array di char che simula la memoria totale che il buddyAllocator gestira'
char memoria_totale[BUDDYALLOCATOR_TOTAL_MEMORY_SIZE]; 
// LISTA DI 1MB 



int main(int argc, char *argv[]) {




    printf("\n\n\n\n\n\n\n\n\n\n\n\n\nInizio programma...\n");
    sleep(1);

    printf("Mi calcolo quanti byte utilizzo per la bitmap sui puntatori che usero'\n");
    sleep(1);
    int BYTEOFNUMEROPUNTATORI = BitMap_getBytes(NUMEROPUNTATORI); // Mi prendo quandi byte occupano i puntatori (per la bitmap)
    printf("Byte per bitmap interna: %d\n", BYTEOFNUMEROPUNTATORI);
    sleep(1);
    printf("Creo buffer interno per la bitmap dei puntatori\n");
    char bufferInterno[BYTEOFNUMEROPUNTATORI]; // Creo un array di byte come buffer per la bitmap dei puntatori
    memset(bufferInterno, 0, BYTEOFNUMEROPUNTATORI); // azzero i valori
    sleep(1);
    printf("Creazione di un array di NUMEROPUNTATORI puntatori void*, dove salverò gli indirizzi di memoria che richiedero'\n");
    void** puntatori = (void**)malloc(NUMEROPUNTATORI * sizeof(void*));  // CREO ARRAY DI NUMEROPUNTATORI PUNTATORI A PUNTATORI VOID* E LI SETTO A VOID
    sleep(1);
    printf("Inizializzazione di ogni valore di questo array a NULL\n");
    for (int i = 0; i < 10; ++i) {
        puntatori[i] = NULL;
    }
    sleep(1);
    printf("Inizializzazione bitMap interna\n");
    BitMap_init(&bitmapInterna, NUMEROPUNTATORI, bufferInterno);
    sleep(1);


    long PAGESIZE = getpagesize();
    printf("Acquisita dimensione di pagina del sistema, dimensione: %ld\n", PAGESIZE);
    sleep(2);



    printf("\n\n------------ INIZIALIZZAZIONE BUDDY ALLOCATOR ------------\n\n");
    sleep(1);
    // RICHIESTA PER UTENTE NUMERO LIVELLI NEL BUDDY
    printf("Scegliere numero livelli massimi per il buddyAllocator: \n");
    int BUDDY_LEVELS;
    scanf("%d", &BUDDY_LEVELS);
    // CALCOLO DIMENSIONE DEL BUFFER
    int BUFFER_SIZE = BufferSizeCalculator(BUDDY_LEVELS);
    // SUDDIVISIONE DELLA MEMORIA TOTALE IN ZONA BUFFER (per bitmap) E ZONA DI MEMORIA
    int MEMORY_SIZE = BUDDYALLOCATOR_TOTAL_MEMORY_SIZE - BUFFER_SIZE;
    sleep(1);
    printf("\n\nMEMORY SIZE: %d\nBUFFER SIZE: %d\n", MEMORY_SIZE, BUFFER_SIZE);
    char* buffer = memoria_totale;                    // inizia da inizio array
    char* memory = memoria_totale + BUFFER_SIZE;      // parte dopo la bitmap
    // CALCOLO DIMENSIONE MINIMA BUCKET
    int MIN_BUCKET_SIZE =  (MEMORY_SIZE>>(BUDDY_LEVELS));
    printf("\nDimensione minima del bucket: %d\n", MIN_BUCKET_SIZE);
    // ORA CHE HO I PARAMETRI INIZIALIZZO
    sleep(1);
    printf("Inizializzazione... \n");
    sleep(2);
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
    sleep(1);




    // TEST ALLOCAZIONE MEMORIA
    printf("\n\n---------- TEST ALLOCAZIONE ----------\n\n");


    int scelta = 0;
    while (1) {


        printf("(1) Per allocare  memoria, (2) Per liberare memoria, (3) Per terminare il programma\n");
        scanf("%d", &scelta);

        if (scelta == 1) {
            printf("Allocazione memoria\n");


            // Vedo se ho memoria libera
            int posizione = -1;
            for (int i = 0; i < NUMEROPUNTATORI; i++) {
                if (!BitMap_bit(&bitmapInterna, i)) {
                    posizione=i;
                    break;
                }
            }

            // Se non ho memoria libera
            if (posizione == -1) {
                printf("Hai giocato con tutti i puntatori, liberane un po\n");
                break;
            }
            else { // Se ho memoria libera
                printf("\nQuanta memoria vuoi allocare? (in byte)\n");
                int memoriaRichiesta = 0;
                scanf("%d", &memoriaRichiesta);

                // Caso buddyAllocator
                if (memoriaRichiesta < (PAGESIZE/4)) {
                    printf("Allocazione tramite BuddyAllocator\n");
                    puntatori[posizione] = BuddyAllocator_malloc(&alloc, memoriaRichiesta);

                    if(puntatori[posizione] == NULL) { // caso in cui fallisce il buddyAllocator_malloc e riporta NULL
                        printf("!!! Errore nell'allocazione tramite BuddyAllocator !!!\n");
                        break;
                    }

                    printf("\nRicevuto blocco di memoria: %p\n", puntatori[posizione]);
                    array_memoriaRichiesta[posizione] = memoriaRichiesta;
                }
                else { // Caso mmap()
                    printf("Allocazione tramite mmap\n");
                    printf("Arrotondo la dimensione richiesta al multiplo maggiore della dim. pagina\n");
                    int memoriaRichiesta_arrotondata = ((memoriaRichiesta + PAGESIZE - 1) / PAGESIZE) * PAGESIZE;
                    puntatori[posizione] = mmap(NULL, memoriaRichiesta_arrotondata, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                    printf("\nRicevuto blocco di memoria: %p\n", puntatori[posizione]);
                    array_memoriaRichiesta[posizione] = memoriaRichiesta_arrotondata;
                }

                BitMap_setBit(&bitmapInterna, posizione, 1);
                printf("Stato dei puntatori con cui gioco:\n");
                BitmapMain_print(&bitmapInterna);
            }
        }

        if(scelta == 2) {
            printf("Liberazione memoria\n");
            printf("Stato bitmap: ");
            BitmapMain_print(&bitmapInterna);
            printf("Scegliere numero blocco memoria da liberare: ");
            int bloccoDaLiberare = 0;
            scanf("%d", &bloccoDaLiberare);
            if (bloccoDaLiberare > NUMEROPUNTATORI) {
                printf("Hai pigiato un numero sbagliato\n");
            } else {
                if (BitMap_bit(&bitmapInterna, bloccoDaLiberare) == 0) {
                    printf("Stai provando a liberare un blocco gia libero\n");
                } else {
                    if (array_memoriaRichiesta[bloccoDaLiberare] < (PAGESIZE/4)) { //libero da buddyAllocator
                        printf("Sto liberando dal buddyAllocator\n");
                        BuddyAllocator_free(&alloc, puntatori[bloccoDaLiberare]);
                    }
                    else { // libero da munmap
                        printf("Sto liberando da mmap()\n");
                        munmap(puntatori[bloccoDaLiberare], array_memoriaRichiesta[bloccoDaLiberare]);
                    }
                }
            }
            // libero le bitmap, arraymemoria richiesta e puntatori
            BitMap_setBit(&bitmapInterna, bloccoDaLiberare, 0);
            array_memoriaRichiesta[bloccoDaLiberare] = 0;
            puntatori[bloccoDaLiberare] = NULL;
        }

        if(scelta == 3) {
            // FINCHE HO BLOCCHI DA LIBERARE LIBERO
            // CHECK SU BITMAP
            // CHIAMO FUNZIONI
            printf("Deallocazione di ogni memoria\n");
            for (int i = 0; i < NUMEROPUNTATORI; i++) { // Guardo ogni posizione della bitmap
                if (BitMap_bit(&bitmapInterna, i)) { // se è occupato
                    if (array_memoriaRichiesta[i] < (PAGESIZE/4)) { //libero da buddyAllocator
                        printf("Sto liberando dal buddyAllocator\n");
                        BuddyAllocator_free(&alloc, puntatori[i]);
                    }
                    else { // libero da munmap
                        printf("Sto liberando da mmap()\n");
                        munmap(puntatori[i], array_memoriaRichiesta[i]);
                    }
                }
            }
            break;
        }
    }
    printf("----Terminazione programma----\n");
    printf("Deallocato ogni spazio di memoria\n");
    printf("Deallocazione void** puntatori\n");
    free(puntatori);
    return 1;





    return 1;
}