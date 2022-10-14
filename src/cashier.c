#define _POSIX_C_SOURCE 200809L

#include "cashier.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "director.h"
#include "sharedlib.h"
#include "utils.h"

//** variabili definite in "supermercato.c"
extern volatile sig_atomic_t quit;  //**variabile sig_atomic per la ricezione di un segnale.
extern queue_t **q;                 //** code condivisa da tutti gli elementi del supermercato.
extern pthread_mutex_t c_mtx;       //**mutex per la sincronizzazione tra cassa-cliente.
extern int w_time, t_sing_prod;     //**  definisce il tempo di chiamata al direttore(NS).
extern void acquista(long ms);      //** funzione acquista che effettua una nanosleep.
int desk_open = 0;                  //**definisce il numero di casse aperte

//**inizializza i dati di una cassa ,
//NOTA: in modo random vengono calcolate le casse che saranno aperte al tempo 0.(almeno una)
cashier_t init_cashier(int i) {
    cashier_t info_c;
    info_c.id = i;
    info_c.sell_pr = 0;
    info_c.n_client = 0;
    info_c.t_time = 0;
    unsigned int seed = i;
    info_c.service_time = GENERATE_RANDOM(&seed, 80, 20);
    info_c.t_sing_prod = t_sing_prod;
    if (i == 1) {
        info_c.open = 1;
        desk_open += 1;
    } else {
        info_c.open = GENERATE_RANDOM(&seed, 1, 0);
        if (info_c.open == 1) desk_open += 1;
    }
    info_c.n_close = 0;

    return info_c;
}

//**funzione che stampa i dati di una cassa
void stampa_desk(cashier_t p) {
    printf("id cassa %d | n. prodotti elaborati %ld | n. di clienti %ld | tempo tot. di apertura %.3f s | tempo medio di servizio %.3f s| n.di chiusure %d| \n\n", p.id, p.sell_pr, p.n_client, MS_TO_S(p.t_time), MS_TO_S(p.t_time / p.n_client), ((cashier_t *)(q[p.id])->d)->n_close);
}

//**funzione che sollecita il direttore ogni w_time secondi
void *timer(void *arg) {
    cashier_t tim_cas = (*(cashier_t *)arg);
    while (quit == 0) {
        acquista(w_time);
        sig_dir(tim_cas.id);
    }
    pthread_exit(NULL);
}

//**routine thread cassiere:
//1)Inizialmente crea il thread timer per le sollecitazioni del direttore.
//2)successivamente inzia a servire dei clienti , viene calcolato il tempo
//che ci impiegherà il cassiere per effettuare gli acquisti del cliente in base
// al suo tempo di servizio e al numero di prodotti del cliente.
//3) lancia una signal al cliente servito sulla sua variabile di condizione.
//4) stampa il cliente.
//5) Al termine stampa i suoi dati
//SIGUP)Non entra più nessuno in coda alla cassa , tutti i clienti vengono serviti
//e successivamente termina.
//SIGQUIT) I clienti in coda non vengono serviti ,
// la cassa lancia una signal a tutti i clienti in fila che succesivamente termineranno
void *cassiere(void *arg) {
    cashier_t t_cas = (*(cashier_t *)arg);
    int id = t_cas.id;
    client_t *client;
    long r = 0;
    pthread_t caller;
    if (pthread_create(&caller, NULL, timer, &t_cas) != 0) {
        perror("pthread create\n");
        exit(EXIT_FAILURE);
    }
    while (quit == 0) {
        if (!Empty(q[id]) && t_cas.open == 1) {
            client = (client_t *)pop(q, id);
            r = t_cas.service_time + t_cas.t_sing_prod * client->products;
            acquista(r);
            client->time_w = r;
            client->t_time += client->time_w;
            client->can_quit = 1;
            t_cas.t_time += r;
            t_cas.sell_pr += client->products;
            t_cas.n_client += 1;
            stampa_c(*client);
            pthread_mutex_lock(&c_mtx);
            pthread_cond_signal(&(client->cond));
            pthread_mutex_unlock(&c_mtx);
        }
    }
    pthread_join(caller, NULL);
    if (quit == 1 && !Empty(q[id])) {
        while (!Empty(q[id])) {
            t_cas.open = 1;
            client = (client_t *)pop(q, id);
            r = t_cas.service_time + t_cas.t_sing_prod * client->products;
            client->time_w = r;
            client->t_time += client->time_w;
            t_cas.sell_pr += client->products;
            t_cas.n_client += 1;
            t_cas.t_time += r;
            stampa_c(*client);
            pthread_mutex_lock(&c_mtx);
            pthread_cond_signal(&(client->cond));
            pthread_mutex_unlock(&c_mtx);
        }
    }

    if (quit == 2) {
        while (!Empty(q[id])) {
            client = (client_t *)pop(q, id);
            pthread_mutex_lock(&c_mtx);
            pthread_cond_signal(&(client->cond));
            pthread_mutex_unlock(&c_mtx);
        }
    }
    stampa_desk(t_cas);
    return NULL;
}
