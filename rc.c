#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <limits.h>
#include "distribution.h"


// STAILQ_HEAD(stailhead, entry) head =
//    STAILQ_HEAD_INITIALIZER(head);
// struct stailhead *headp;                /* Singly-linked tail queue head. */
// struct entry {
//        ...
//        STAILQ_ENTRY(entry) entries;    /* Tail queue. */
//        ...
// } *n1, *n2, *n3, *np;

// STAILQ_INIT(&head);                     /* Initialize the queue. */

// n1 = malloc(sizeof(struct entry));      /* Insert at the head. */
// STAILQ_INSERT_HEAD(&head, n1, entries);

// n1 = malloc(sizeof(struct entry));      /* Insert at the tail. */
// STAILQ_INSERT_TAIL(&head, n1, entries);

// n2 = malloc(sizeof(struct entry));      /* Insert after. */
// STAILQ_INSERT_AFTER(&head, n1, n2, entries);
//                                        /* Deletion. */
// STAILQ_REMOVE(&head, n2, entry, entries);
// free(n2);
//                                        /* Deletion from the head. */
// n3 = STAILQ_FIRST(&head);
// STAILQ_REMOVE_HEAD(&head, entries);
// free(n3);
//                                        /* Forward traversal. */
// STAILQ_FOREACH(np, &head, entries)
//        np-> ...
//                                        /* Safe forward traversal. */
// STAILQ_FOREACH_SAFE(np, &head, entries, np_temp) {
//        np->do_stuff();
//        ...
//        STAILQ_REMOVE(&head, np, entry, entries);
//        free(np);
// }
//                                        /* TailQ Deletion. */
// while (!STAILQ_EMPTY(&head)) {
//        n1 = STAILQ_FIRST(&head);
//        STAILQ_REMOVE_HEAD(&head, entries);
//        free(n1);
// }
//                                        /* Faster TailQ Deletion. */
// n1 = STAILQ_FIRST(&head);
// while (n1 != NULL) {
//        n2 = STAILQ_NEXT(n1, entries);
//        free(n1);
//        n1 = n2;
// }
// STAILQ_INIT(&head);

STAILQ_HEAD(_stailhead, entry);
typedef struct _stailhead *stailhead;

// Un nodo della lista
typedef struct entry {
  unsigned int enc; //pacchetto codificato
  int * indices; //gli indici generatori del pacchetto
  int indices_count;
  STAILQ_ENTRY(entry) entries;
} *enc_packet;

// Crea un pacchetto codificato vuoto
enc_packet create_enc_packet(int d)
{
  enc_packet p = malloc(sizeof(struct entry));
  p->indices = malloc(sizeof(int) * d);
  p->indices_count = d;
  return p;
}

// Distrugge 
void destroy_enc_packet(enc_packet p)
{
  free(p->indices);
  free(p);
}

// Funzione di codifica.
stailhead encode(unsigned int *x, int k, stailhead encoded_packets)
{
  // Inizializza le variabili utili per la codifica
  initialize_rsd(0.05, 0.05, k); // Inizializza il generatore random rsd
  int n,i,j,d;
  n = blocksNeeded();
  enc_packet p;

  // Ciclo principale, lo esegue n volte
  for (i = 0; i < n; ++i)
  { 
    d = rsd();  // il grado
    p = create_enc_packet(d); // il pacchetto codificato (vuoto)
    int taken_count = 0;  // il numero degli indici già pescati
    unsigned int enc = 0;

    //Ciclo interno. Sorteggia a caso per d volte (senza ripetizione) uno dei k pacchetti.
    for (j = 0; j < d; ++j)
    {
      int index;
      index = next_int(0, k-1);
      int start_index = index;

      // se l'indice è stato già sorteggiato passa al successivo
      while(index_taken(index, p->indices, taken_count))
      {
        index = (index + 1) % k;
        
        // Controllo per evitare loop infiniti
        if(start_index == index)
        {
          printf("Warning! infinite loop detected!\n");
          break;
        }
      }

      // imposta l'indice estratto per evitare un'ulteriore estrazione
      p->indices[taken_count++] = index;

      // effettua la codifica man mano che va avanti
      enc = enc ^ x[index];
    }

    // Completa il pacchetto e lo inserisce in fondo alla lista
    p->enc = enc;
    STAILQ_INSERT_TAIL(encoded_packets, p, entries);
  }
}

int index_taken(int index, int * taken_indices, int count)
{
  int i;
  for (i = 0; i < count; ++i)
    if(taken_indices[i] == index)
      return 1;
  return 0;
}

int main(int argc, char const *argv[])
{
  srand(clock(NULL));

  // Crea la lista che conterrà i pacchetti codificati
  struct _stailhead _encoded_packets = STAILQ_HEAD_INITIALIZER(_encoded_packets);
  stailhead encoded_packets;
  encoded_packets = &_encoded_packets;
  STAILQ_INIT(encoded_packets);

  // Inizializza la sorgente
  unsigned int source[700];
  int i;
  for (i = 0; i < 700; ++i)
  {
    //Crea la sorgente random
    source[i] = (unsigned)next_int(0, RAND_MAX);
  }

  // Codifica
  encode(source, 700, encoded_packets);

  // Itera per stampare ogni packet
  enc_packet iterator;
  i=0;
  STAILQ_FOREACH(iterator, encoded_packets, entries)
  {
    printf("packet %d - enc: %d\n", i, iterator->enc);
    i++;
  }
  return 0;
}