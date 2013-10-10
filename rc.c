#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include "distribution.h"

const char *allowed_distributions_name[] = {"rsd", "uniform", "invexp"};
const char *allowed_distributions_desc[] = {"Robust Soliton Distribution"," Uniform Distribution", "Inverse Exponential Distribution"};
const int allowed_distributions_count = 3;
int verbose;

typedef enum _allowed_distribution
{
  RSD = 0,
  UNIFORM = 1,
  INVEXP = 2
} allowed_distribution;

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
int encode(unsigned int *x, int k, int n,  allowed_distribution distr, stailhead out_encoded_packets)
{
  // Inizializza le variabili utili per la codifica
  initialize_rsd(0.05, 0.05, k); // Inizializza il generatore random rsd
  int i,j,d;
  enc_packet p;

  // Ciclo principale, lo esegue n volte
  for (i = 0; i < n; ++i)
  {
    switch(distr)
    {
      case RSD:
        d = rsd();  // il grado
        break;
      case UNIFORM:
        d = next_int(1, k);  // il grado
        break;
      case INVEXP:
        d = expd(k);
        break;
      default:
        if(verbose)
          printf("Invalid distribution index set: %d\n", distr);
        return 0;
    }
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
          if(verbose)
            printf("Error! infinite loop detected!\n");
          return 0;
        }
      }

      // imposta l'indice estratto per evitare un'ulteriore estrazione
      p->indices[taken_count++] = index;

      // effettua la codifica man mano che va avanti
      enc = enc ^ x[index];
    }

    // Completa il pacchetto e lo inserisce in fondo alla lista
    p->enc = enc;
    STAILQ_INSERT_TAIL(out_encoded_packets, p, entries);
  }
  return 1;
}

// Controlla se l'indice è già stato selezionato
int index_taken(int index, int * taken_indices, int count)
{
  int i;
  for (i = 0; i < count; ++i)
    if(taken_indices[i] == index)
      return 1;
  return 0;
}

// Trova un pacchetto di grado 1
enc_packet find_degree1(stailhead packets)
{
  enc_packet p;
  enc_packet found = NULL;
  STAILQ_FOREACH(p, packets, entries)
  {
    if(p->indices_count == 1)
    {
      found = p;
      break;
    }
  }
  return found;
}

// Trova e rimuove un indice dall'array di indici di un dato pacchetto
int find_remove_index(enc_packet p, int index)
{
  // Cerca l'indice
  int i;
  for (i = 0; i < p->indices_count; ++i)
  {
    if(p->indices[i] == index)
      break;
  }

  // Se l'ha trovato
  if (i < p->indices_count)
  {
    // Trasla tutto l'array
    int j;
    for (j = i; j < p->indices_count; ++j)
    {
      if(j < p->indices_count - 1)
      {
        p->indices[j] = p->indices[j+1];
      }
    }

    // Riduce di 1 il numero di indici presenti nel pacchetto
    p->indices_count--;
    return 1;
  }
  else
    return 0;
}

// Funzione di decodifica
int decode(stailhead encoded_packets, unsigned int *out_x)
{
  enc_packet p;
  while(1)
  {
    // Trova un pacchetto di grado 1
    p = find_degree1(encoded_packets);

    // Se non esiste un pacchetto di grado 1 termina
    if (p == NULL)
    {
      if(verbose)
        printf("Decode failed: no more packets with degree 1 were found.\n");
      return 0;
    }
    
    // Decodifica il pacchetto e lo elimina dalla lista
    out_x[p->indices[0]] = p->enc;
    STAILQ_REMOVE(encoded_packets, p, entry, entries);

    // Per ogni pacchetto che contiene l'indice p->indices[0], esegue lo xor con p->enc e rimuove l'indice
    enc_packet iterator;
    STAILQ_FOREACH(iterator, encoded_packets, entries)
    {
      if(find_remove_index(iterator, p->indices[0]))
      {
        iterator->enc = iterator->enc ^ p->enc;
      }
    }

    // Libera la memoria occupata dal pacchetto
    destroy_enc_packet(p);

    // Elimina i pacchetti che sono rimasti con grado 0
    enc_packet p_temp;
    STAILQ_FOREACH_SAFE(iterator, encoded_packets, entries, p_temp)
    {
      if(iterator->indices_count < 1)
      {
        STAILQ_REMOVE(encoded_packets, iterator, entry, entries);
        destroy_enc_packet(iterator);
      }
    }

    // Termina se non ci sono più pacchetti da codificare
    if(STAILQ_EMPTY(encoded_packets))
      break;
  }
  return 1;
}

// Stampa il corretto utilizzo
void print_usage()
{
  printf("rc - Esegue la codifica di k pacchetti presi da una sorgente e la successiva decodifica e informa riguardo al successo o insuccesso dell'operazione.\n");
  printf("Usage: ./rc [options]\n");
  printf("Options:\n");
  printf("  %-20s%s\n", "-h", "Prints this message.");
  printf("\n");
  printf("  %-20s%s\n", "-k <value>", "Sets to <value> the number of packets emitted by the source. Default is 70");
  printf("  %-20s%s\n", "-n <value>", "Encodes the k packets of the source into <value> encoded packets. Default is k * 2");
  printf("  %-20s%s\n", "","min value is 2, maximum is 100000");
  printf("\n");
  printf("  %-20s%s\n", "-d <ditsr>", "Use the distribution <distr> for encoding.");
  printf("  %-20s%s\n", "", "Allowed <distr> are:");
  printf("  %-20s-  %-10s%s\n", "", "rsd", "Robust Soliton Distribution");
  printf("  %-20s-  %-10s%s\n", "", "uniform", "Uniform Distribution");
  printf("  %-20s-  %-10s%s\n", "", "invexp", "Inverse Exponential Distribution");
}

// Entry point
int main(int argc, char **argv)
{
  // TODO: da mettere come opzione argv
  int k = 70;
  allowed_distribution dist = RSD;
  int o;
  int i;
  int n = 0;
  verbose = 0;

  while ((o = getopt (argc, argv, "k:d:hn:v")) != -1)
  {
    int found = 0;
    switch (o)
    {
      case 'h':
        print_usage();
        return 0;
      case 'k':
        k = atoi(optarg);
        break;
      case 'n':
        n = atoi(optarg);
        break;
      case 'v':
        verbose = 1;
        break;
      case 'd':
        for (i = 0; i < allowed_distributions_count && !found; i++)
        {
          if (!strcmp(optarg,allowed_distributions_name[i]))
          {
            dist = i;
            found = 1;
          }
        }
        if(!found)
        {
          printf("%s is not a valid value for option -d.\n", optarg);
          print_usage();
          return 1;
        }
        break;
      case '?':
        if (optopt == 'k')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        print_usage();
        return 1;
      default:
        print_usage();
        return 1;
    }
  }
  if (!n)
  {
    n = k*2;
  }

  if(verbose)
  {
    printf("Rateless encoding and decoding of a random source.\n");
    printf("k:\t%d\n", k);
    printf("n:\t%d\n", n);
    printf("distr:\t%s\n", allowed_distributions_desc[dist]);
  }

  srand(clock(NULL));

  // Crea la lista che conterrà i pacchetti codificati
  struct _stailhead _encoded_packets = STAILQ_HEAD_INITIALIZER(_encoded_packets);
  stailhead encoded_packets;
  encoded_packets = &_encoded_packets;
  STAILQ_INIT(encoded_packets);

  // Inizializza la sorgente
  unsigned int source[k];

  // Crea la sorgente random
  for (i = 0; i < k; ++i)
    source[i] = (unsigned)next_int(0, RAND_MAX);

  if(verbose)
    printf("\nEncoding...\n");
  // Codifica
  int encoded = encode(source, k, n, dist, encoded_packets);
  if(!encoded)
    return 1;
  else
    if (verbose)
      printf("Success!\n");

  if (verbose)
    printf("\nDecoding...\n");
  // Decodifica
  unsigned int dest[k];
  int decoded = decode(encoded_packets, dest);

  // Se la decodifica è andata a buon fine, controlla la validità.
  if(decoded)
  {
    if (verbose)
    {
      printf("Success!\n");
      printf("\nComparison between source and destination...\n");
    }
    int failed = 0;
    for (i = 0; i < k && !failed; ++i)
    {
      if(source[i] != dest[i])
        failed = 1;
    }
    if(failed)
    {
      if (verbose)
        printf("Error! Source packets and decoded packets are different.\n");
      return;
    }
    else
      printf("Success!\n");
  }
  else
  {
    if(verbose)
      printf("Error! The decoder has failed to decode.\n");
    else
      printf("Fail!\n");
    return 1;
  }

  return 0;
}