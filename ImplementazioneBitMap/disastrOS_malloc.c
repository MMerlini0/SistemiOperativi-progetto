
#include "buddy_allocator.h"
#include "bit_map.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

#define TOTAL_MEMORY_SIZE (1024*1024) // LA MEMORY SIZE INIZIALE E' SEMPRE DI 1MB COME DETTO NELLE SPECIFICHE DI PROGETTO
#define SIZEOFPUNTATORI 10

BuddyAllocator alloc;
// CREO BITMAP INTERNA PERCHE' RIUSO MAGHEGGIO PER TENERE STATO MEMORIA ALLO STESSO MODO HAHAH
BitMap bitmapInterna;
int bufferInterno[SIZEOFPUNTATORI];
int array_memoriaRichiesta[SIZEOFPUNTATORI]; // salvo quanta memoria ha richiesto quel blocco


char memoria_totale[TOTAL_MEMORY_SIZE]; // LISTA CHE SIMULA LA MIA MEMORIA TOTALE PER IL BUDDY ALLOCATOR
                                        // LISTA DI 1MB 



int main(int argc, char *argv[]) {

    printf("Inizio programma...\n");
    sleep(1);
    long PAGESIZE = getpagesize();
    printf("Acquisita dimensione di pagina del sistema, dimensione: %d\n", PAGESIZE);
    sleep(1);



    printf("\n\n------------ INIZIALIZZAZIONE BUDDY ALLOCATOR ------------\n\n");
    // RICHIESTA PER UTENTE NUMERO LIVELLI NEL BUDDY
    printf("Scegliere numero livelli massimi per il buddyAllocator: \n");
    int BUDDY_LEVELS;
    scanf("%d", &BUDDY_LEVELS);
    // CALCOLO DIMENSIONE DEL BUFFER
    int BUFFER_SIZE = BufferSizeCalculator(BUDDY_LEVELS);
    // SUDDIVISIONE DELLA MEMORIA TOTALE IN ZONA BUFFER (per bitmap) E ZONA DI MEMORIA
    int MEMORY_SIZE = TOTAL_MEMORY_SIZE - BUFFER_SIZE;
    printf("\n\nMEMORY SIZE: %d\n BUFFER SIZE: %d\n", MEMORY_SIZE, BUFFER_SIZE);
    char* buffer = memoria_totale;                    // inizia da inizio array
    char* memory = memoria_totale + BUFFER_SIZE;      // parte dopo la bitmap
    // CALCOLO DIMENSIONE MINIMA BUCKET
    int MIN_BUCKET_SIZE =  (MEMORY_SIZE>>(BUDDY_LEVELS));
    printf("\n Dimensione minima del bucket: %d", MIN_BUCKET_SIZE);
    // ORA CHE HO I PARAMETRI INIZIALIZZO
    printf("Inizializzazione... \n");
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
    // CREO ARRAY DI SIZEOFPUNTATORI PUNTATORI A PUNTATORI VOID* E LI SETTO A VOID
    void** puntatori = (void**)malloc(SIZEOFPUNTATORI * sizeof(void*));
    int bitmap[SIZEOFPUNTATORI];
    for (int i = 0; i < 10; ++i) {
        puntatori[i] = NULL;
    }

    while (1) {

        int num_bytes = BitMap_getBytes(SIZEOFPUNTATORI);
        BitMap_init(&bitmapInterna, num_bytes, bufferInterno);


        printf("(1) Per allocare chiedere altra memoria, (2) Per liberare memoria, (3) Per terminare il programma\n");
        scanf("%d", &scelta);

        if (scelta == 1) {
            printf("Allocazione memoria\n");


            // Vedo se ho memoria libera
            int posizione = -1;
            for (int i; i < SIZEOFPUNTATORI; i++) {
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
                printf("\nQuanta memoria vuoi allocare?\n");
                int memoriaRichiesta = 0;
                scanf("%d", &memoriaRichiesta);

                // Caso buddyAllocator
                if (memoria_totale < (PAGESIZE/4)) {
                    printf("Allocazione tramite BuddyAllocator\n");
                    puntatori[posizione] = BuddyAllocator_malloc(&alloc, memoriaRichiesta);
                    printf("\nRicevuto blocco di memoria: %p\n", puntatori[posizione]);
                    array_memoriaRichiesta[posizione] = memoriaRichiesta;
                }
                else { // Caso mmap()
                    printf("Allocazione tramite bitMap\n");
                    printf("Arrotondo la dimensione richiesta al multiplo maggiore di dim. pagina\n");
                    int memoriaRichiesta_arrotondata = ((memoriaRichiesta + PAGESIZE - 1) / PAGESIZE) * PAGESIZE;
                    puntatori[posizione] = mmap(NULL, memoriaRichiesta_arrotondata, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                    printf("\nRicevuto blocco di memoria: %p\n", puntatori[posizione]);
                    array_memoriaRichiesta[posizione] = memoriaRichiesta_arrotondata;
                }

                BitMap_setBit(&bitmapInterna, posizione, 1);
                printf("Stato dei puntatori con cui gioco:\n");
                Bitmap_print(&bitmapInterna);
            }
        }

        if(scelta == 2) {
            printf("Liberazione memoria\n");
            printf("Stato bitmap: ");
            Bitmap_print(&bitmapInterna);
            printf("Scegliere numero blocco memoria da liberare: ");
            int bloccoDaLiberare = 0;
            scanf("%d", &bloccoDaLiberare);
            if (bloccoDaLiberare > SIZEOFPUNTATORI) {
                printf("Hai pigiato un numero sbagliato\n");
                break;
            }
            if (array_memoriaRichiesta[bloccoDaLiberare] < PAGESIZE) { //libero da buddyAllocator
                printf("Sto liberando dal buddyAllocator\n");
                BuddyAllocator_free(&alloc, bufferInterno[bloccoDaLiberare]);
            }
            else { // libero da munmap
                printf("Sto liberando da mmap()\n");
                munmap(bufferInterno[bloccoDaLiberare], array_memoriaRichiesta[bloccoDaLiberare]);
            }
        }

        if(scelta == 3) {
            // FINCHE HO BLOCCHI DA LIBERARE LIBERO
            // CHECK SU BITMAP
            // CHIAMO FUNZIONI
            printf("Deallocazione di ogni memoria\n");
            for (int i; i < SIZEOFPUNTATORI; i++) { // Guardo ogni posizione della bitmap
                if (BitMap_bit(&bitmapInterna, i)) { // se è occupato
                    if (array_memoriaRichiesta[i] < PAGESIZE) { //libero da buddyAllocator
                        printf("Sto liberando dal buddyAllocator\n");
                        BuddyAllocator_free(&alloc, bufferInterno[i]);
                    }
                    else { // libero da munmap
                        printf("Sto liberando da mmap()\n");
                        munmap(bufferInterno[i], array_memoriaRichiesta[i]);
                    }
                }
            }
            printf("Deallocato ogni spazio di memoria\n");
            printf("Deallocazione void** puntatori\n");
            free(puntatori);
        }
    }


    


    return 1;
}