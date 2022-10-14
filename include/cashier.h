#define _POSIX_C_SOURCE 200809L

#if !defined(CASHIER_H)
#define CASHIER_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "sharedlib.h"

//**strcut tipo cassa
typedef struct cassa {
    int id; //**id cassiere
    size_t sell_pr; //** numero totale di prodotti venduti 
    size_t n_client; //** numero totale di clienti serviti
    float t_time;	//** tempo totale di apertura
    float service_time; //** tempo di servizio per singolo prodotto
    int t_sing_prod; //** tempo variabile per numero di prodotti del cliente
    int n_close; //** numero di chiusure
    int open; //** flag apertura
} cashier_t;

//**funzione che inizializza i dati di un cassiere
cashier_t init_cashier(int i);

//**funzione che stampa i dati di una cassa
void stampa_desk(cashier_t p);

//**funzione timer , serve a sollecitare il direttore ogni w_time secondi
void *timer(void *arg);

//**routine thread cassiere
void *cassiere(void *arg);

#endif /*CASHIER_h*/