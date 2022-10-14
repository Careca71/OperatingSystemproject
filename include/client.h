#define _POSIX_C_SOURCE 200809L

#if !defined(CLIENT_H)
#define CLIENT_H

#include <stdio.h>
#include <signal.h> 
#include <stdlib.h>
#include <pthread.h>


#include "sharedlib.h"
#include "utils.h"
#include "cashier.h"


//**struttura dati per il tipo cliente
typedef struct client {
    int id;	//** id cliente
    size_t products; //** numero di prodotti
    float t_time;	//** tempo totale all'interno del supermercato
    int time;		//** tempo iniziale per gli acquisti generato in modo casuale
    float time_w;	//** tempo di attesa in cassa
    int visited_c;	//** numero di casse visitate
	pthread_cond_t cond;	//** variabile di condizione per l'uscita del cliente
    int can_quit;		//** flag per l'uscita
} client_t;

//**funzione che inizializza i dati cliente
client_t init_client(int i, int P, int T);

//**funzione che stampa i dati del cliente
void stampa_c(client_t p);

//**routine thread cliente 
void *cliente(void *arg) ;

//**libera la memoria occupata dalla strcut cliente
void free_clients(client_t * cli);

#endif /*CLIENT_H*/