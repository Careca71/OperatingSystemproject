#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "cashier.h"
#include "client.h"
#include "director.h"
#include "utils.h"

pthread_mutex_t c_mtx = PTHREAD_MUTEX_INITIALIZER;    //**mutex per la sincronizzazione tra cliente-cassiere
pthread_mutex_t dir_mtx = PTHREAD_MUTEX_INITIALIZER;  //**mutex per la sincronizzazione tra cliente-direttore

int k, c, p, t, e, s1, s2, w_time, t_sing_prod;  //**variabili globali che prenderanno i valori dal configfile
int total_clients = 0;                           //** definisce il numero totale dei clienti all'interno del supermercato
queue_t **q;                                     //** code condivise da tutti gli elementi del supermercato
int p_dir = 0;                                   //** posizione riservata alla coda del direttore

//**variabile globale per catturare i segnali di terminazione per il processo supermercato.
volatile sig_atomic_t quit = 0;

//**effettua una nanosleep calcolando in secondi e in nanosecondi i millisecondi dati in input
void acquista(long ms) {
    struct timespec ts;
    int ris = 1;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    while (ris) {
        ris = nanosleep(&ts, NULL);
    }
    return;
}

//funzione per la gestione del segnale SIGUP.
void manager(int sig) {
    if (sig == SIGHUP) {
        quit = 1;
    }
    return;
}
//funzione per la gestione del segnale SIGQUIT.
void manager1(int sig) {
    if (sig == SIGQUIT) {
        quit = 2;
    }
    return;
}

//**routine thread security_counter:
//1) prende il lock di c_mtx , controlla il numero di clienti all'interno del supermercato
//2) in caso ci fosse bisogno di inserire nuovi clienti calcola l'id dei nuovi clienti e lancia
//altri E thread clienti.
//3)aumenta il valore di total_clients
//4) con l'arrivo di qualsiasi segnale termina.

void *security_counter(void *arg) {
    client_t *newpt;
    pthread_t *newc;
    int n_entry = 0;
    int cur = 0;
    newpt = malloc(e * sizeof(client_t));
    if (!newpt) {
        perror("errore nell'allocazione di memoria client_t\n");
        exit(EXIT_FAILURE);
    }
    newc = malloc(e * sizeof(pthread_t));
    while (quit == 0) {
        pthread_mutex_lock(&c_mtx);
        if (total_clients <= (c - e) && quit == 0) {
            cur = 0;
            //printf("\n\nSTO FACENDO ENTRARE %d clienti\n\n", e);
            for (int i = (c + n_entry); i < (e + c + n_entry); i++) {
                newpt[cur].id = i;
                if (pthread_create(&newc[cur], NULL, cliente, &newpt[cur]) != 0) {
                    perror("pthread_create");
                    exit(EXIT_FAILURE);
                }
                cur += 1;
            }
            n_entry += e;
            total_clients += e;
            fprintf(stdout, "total_clients %d\n", total_clients);
        }
        pthread_mutex_unlock(&c_mtx);
    }
    printf("\n\nl'uomo della sicurezza è uscito dal supermercato ,dato che non entrerà più nessuno\n\n");
    free(newpt);
    free(newc);
    return NULL;
}

//*********MAIN**********
int main(int argc, char const *argv[]) {
    //**controllo i dati in input
    if (argc < 2) {
        fprintf(stderr, "use: %s configfile.txt", argv[0]);
        return EXIT_FAILURE;
    }

    //leggo il nome del config file e lo mando alla funzione read_config_file
    const char *infile = argv[1];

    int fd;
    if ((fd = open(infile, O_RDWR)) == -1) {
        perror("errore in apertura del file");
        return -1;
    }
    if (read_config_file(fd) != 1) exit(EXIT_FAILURE);

    //**aumento la variabile K di una posizione dato che ho una coda riservata al direttore
    k += 1;

    //**assegno al numero totale di clienti nel supermercato la variabile C.
    total_clients = c;

    //**inizializzo un array di code di dim K.
    //**i controlli sulle malloc vengono effettuate all'interno di MakeNull.
    q = MakeNull(k);

    //**definisco un array di k elementi di tipo cassa
    cashier_t *i_desk;
    i_desk = malloc(k * sizeof(cashier_t));
    if (!i_desk) {
        perror("errore nell'allocazione della memoria su cashier_t\n");
        exit(EXIT_FAILURE);
    }

    //**definisco un array di C elementi di tipo cliente
    client_t *i_cli;
    i_cli = malloc(c * sizeof(client_t));
    if (!i_cli) {
        perror("errore nell'allocazione della memoria su client_t\n");
        exit(EXIT_FAILURE);
    }

    //**definisco K+C thread , sia per i clienti che per le casse
    pthread_t *tic;
    tic = malloc((k + c) * sizeof(pthread_t));

    //**thread_direttore
    pthread_t dir;

    //**thread_contatore
    pthread_t counter;

    //**definisco due tipi sigation S e S1 per la ricezione dei due segnali
    struct sigaction s;
    struct sigaction s1;
    memset(&s, 0, sizeof(s));
    memset(&s1, 0, sizeof(s1));
    s.sa_handler = manager;
    s1.sa_handler = manager1;
    if (sigaction(SIGHUP, &s, NULL) == -1) {
        fprintf(stderr, " Errore gestore ");
    }
    if (sigaction(SIGQUIT, &s1, NULL) == -1) {
        fprintf(stderr, " Errore gestore ");
    }

    //**Inizializzo le casse
    //passando al campo d della coda la cassa appena inizializzata
    for (int i = 0; i < k; i++) {
        if (i == p_dir) {
            i_desk[i].open = 1;
            i_desk[i].id = i;
        } else {
            i_desk[i] = init_cashier(i);
        }
        q[i]->d = &i_desk[i];
    }

    //**inizializzo id dei primi c clienti
    for (int i = 0; i < c; i++) {
        i_cli[i].id = i;
    }

    //**creo thread contatore
    if (pthread_create(&counter, NULL, security_counter, &q) != 0) {
        perror("pthread_create\n");
        exit(EXIT_FAILURE);
    }
    //**creo thread clienti
    for (int i = 0; i < c; i++) {
        if (pthread_create(&tic[i], NULL, cliente, &i_cli[i]) != 0) {
            perror("pthread_create\n");
            exit(EXIT_FAILURE);
        }
    }

    //**creao thread cassieri
    for (int i = 1; i < k; i++) {
        if (pthread_create(&tic[i], NULL, cassiere, &i_desk[i]) != 0) {
            perror("pthread_create\n");
            exit(EXIT_FAILURE);
        }
    }

    //**creo thread direttore
    if (pthread_create(&dir, NULL, director, NULL) != 0) {
        perror("pthread_create\n");
        exit(EXIT_FAILURE);
    }

    //**join thread clienti
    for (int i = 0; i < c; i++)
        pthread_join(tic[k + i], NULL);

    //**join thread contatore
    pthread_join(counter, NULL);

    //**join thrad casse
    for (int i = 0; i < k; i++)
        pthread_join(tic[i], NULL);

    //**join thread direttore
    pthread_join(dir, NULL);

    //**libero la memoria
    free(i_desk);
    free_clients(i_cli);
    free(tic);
    pthread_mutex_destroy(&c_mtx);
    pthread_mutex_destroy(&dir_mtx);
    delete_queue(q, k);

    printf("\nil supermercato è chiuso.. grazie di averci scelto , a domani! \n");

    return 0;
}