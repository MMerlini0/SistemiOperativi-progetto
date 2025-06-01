# SistemiOperativi-progetto

Il corrente git riporta l'implementazione della seguente consegna di progetto:

## L1. Pseudo Malloc

> This is a malloc replacement.  
> The system relies on `mmap` for the physical allocation of memory, but handles the requests in two ways:

- **For small requests** (< 1/4 of the page size):  
  It uses a **buddy allocator**.  
  Clearly, such a buddy allocator can manage at most page-size bytes.  
  For simplicity, a single buddy allocator is used, implemented with a **bitmap** that manages **1 MB** of memory for these small allocations.

- **For large requests** (≥ 1/4 of the page size):  
  It uses a direct call to **`mmap()`**.

---

All'interno del git vi si possono trovare **due cartelle**, che riportano lo stesso codice.

- La cartella **`CodiceCommentato/`** contiene:
  - Una gran parte di **commenti** che spiegano quasi ogni singola operazione e funzione.
  - Presenta inoltre anche le parti di codice della vecchia implementazione del `buddyAllocator` tramite liste, e, in alcune parti, commenti che ne specificano  le differenze.

- La cartella **`PseudoMalloc/`** contiene:
  - **Solo il codice** per una visualizzazione più snella.

---

## Programma `main`

Il programma `main` presenta una funzione che permette all'utilizzatore di testare dinamicamente funzioni di malloc e free per differenti puntatori, permettendone quindi l'arbitrario testaggio senza dover scrivere codice.

> Il `numero di puntatori massimi` quali è possibile gestire è riportato dalla variabile **NUMEROPUNTATORI** definita *all'inizio del codice* presente nel file main **disastrOS_malloc.c**, `il valore di default è 10`.

---

### Nota sulla gestione dei puntatori

Questa funzione di gestione puntatori è stata volutamente complicata, al posto di avere dei banali controlli sui valori dell'array che contiene i puntatori, del tipo:
```c
if (arrayPuntatori[i] == NULL) { ... }

ho voluto usare la stessa metodologia di controllo per spazi liberi e occupati applicata nel buddyAllocator, ossia ho utilizzato un altra bitmap che ne tiene conto.


