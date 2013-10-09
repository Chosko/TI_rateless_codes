
// Inizializza i parametri per la distribuzione rsd
void initialize_rsd(double c, double delta, int k);

//Ritorna un intero random preso dalla distribuzione rsd
int rsd();

//Ritorna il numero di blocchi necessario per permettere una trasmissione affidabile
int blocksNeeded();

//Ritorna un double random tra 0 e 1
double next_double();

//Ritorna un int random tra min e max
int next_int(int min, int max);