#define _POSIX_C_SOURCE 200809L

#include "client.h"

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "cashier.h"
#include "sharedlib.h"
#include "utils.h"

//**variabili definite in supermercato.c.
extern int p, t, k, c;              //**var globali definite sul file di configurazione
extern volatile sig_atomic_t quit;  //** variabile sig_atomic per la ricezione di un segnale
extern int total_clients;           //** variabile che descrive il numero totale dei clienti nel supermercato
extern int p_dir;                   //** variabile che definisce la posizione della coda direttore nel supermercato
extern pthread_mutex_t dir_mtx;     //** mutex per la sincronizzazione tra direttore e cliente
extern pthread_mutex_t c_mtx;       //** mutex per la sincronizzazione tra cliente e cassa
extern queue_t **q;                 //** code convise da tutti gli elementi nel supermercato
extern void acquista(long ms);      //** funzione acquista che effettua una nanosleep.

//**inzializza i dati di un cliente
client_t init_client(int i, int P, int T) {
    client_t info_t;
    info_t.id = i;
    unsigned int seed = i;
    info_t.products = GENERATE_RANDOM(&seed, P, 0);
    info_t.time = GENERATE_RANDOM(&seed, T, 10);
    info_t.t_time = info_t.time;
    if (pthread_cond_init(&(info_t.cond), NULL) != 0) {
        perror("pthread_cond_init\n");
        exit(EXIT_FAILURE);
    }
    info_t.can_quit = 0;

    return info_t;
}

//**funzione che stampa i dati di un cliente
void stampa_c(client_t p) {
    fprintf(stdout, " id cliente %d | n. prodotti acquistati %ld | tempo tota nel super. %.3f s| tempo tot. speso in coda %.3f s| n. di code visitate %d \n\n", p.id, p.products, MS_TO_S(p.t_time), MS_TO_S(p.time_w), p.visited_c);
    return;
}

//**routine thread cliente:
//1)inizializza i suoi dati e il tempo per gli acquisti
//2)tramite la funzione acquisa dorme per t_cli.time nanosecondi
//3)decide un modo random una cassa libera e si mette in coda .
//(si mette in coda in p_dir se ha acquistato 0 prodotti)
//4)acquisisce il lock di c_mtx (dir_mtx se il coda dal direttore) e attende di essere servito
//5)esce decrementando il valore di total_clients

void *cliente(void *arg) {
    client_t t_cli = (*(client_t *)arg);
    t_cli = init_client(t_cli.id, p, t);
    if (quit != 0) pthread_exit(NULL);

    acquista(t_cli.time);

    unsigned int seed = t_cli.id;

    int cassa = GENERATE_RANDOM(&seed, k - 1, 1);
    if (t_cli.products != 0 && quit == 0) {
        while ((((cashier_t *)(q[cassa]->d))->open == 0 && !quit) || cassa == p_dir) {
            if (t_cli.visited_c < k - 1) {
                t_cli.visited_c += 1;
            }
            cassa = GENERATE_RANDOM(&seed, k - 1, 1);
        }

        if (quit != 0) pthread_exit(NULL);
        push(q, cassa, &(t_cli));
    } else {
        if (quit != 0) pthread_exit(NULL);
        push(q, p_dir, &(t_cli));
        pthread_mutex_lock(&dir_mtx);
        while (t_cli.can_quit == 0) {
            pthread_cond_wait(&(t_cli.cond), &dir_mtx);
        }
        pthread_mutex_unlock(&dir_mtx);
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&c_mtx);
    while ((t_cli.can_quit) == 0 && !quit) {
        pthread_cond_wait(&(t_cli.cond), &c_mtx);
    }
    // printf("cliente %d esce %d \n",t_cli.id,t_cli.can_quit);
    total_clients -= 1;
    pthread_mutex_unlock(&c_mtx);
    pthread_exit(NULL);
    return NULL;
}

void free_clients(client_t *cli) {
    for (int i = 0; i < c; i++) (pthread_cond_destroy(&(cli[i].cond)));

    free(cli);

    return;
}