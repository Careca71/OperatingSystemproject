#define _POSIX_C_SOURCE 200809L

#include "director.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#include "cashier.h"
#include "client.h"
#include "sharedlib.h"

//**variabili definite "in supermercato.c"
extern queue_t** q;                 //** code condivise da tutti gli elementi del supermercato
extern pthread_mutex_t dir_mtx;     //**mutex per la sincronizzazione tra direttore-cliente
extern int k, s1, s2;               //** variabili definite da config.file
extern int p_dir;                   //** definisce la posizione della coda del direttore
extern volatile sig_atomic_t quit;  //**variabile sig_atomic per la ricezione di un segnale.

//**variabile per mandare una signal dalle casse al direttore
static int sig = -1;

//**variabile che segnala una coda con size > s2
static int danger_tail = 0;

//** definisce il numero di code aperte definita su cashier.c
extern int desk_open;

//**indica l'ultima cassa aperta
static int prec_desk = 0;

//** funzione invocata dal timer del cassiere per mandare una signal al direttore
void sig_dir(int cur) {
    lockqueue(q[cur]);

    sig = 0;
    signaldirector(q[cur]);

    unlockqueue(q[cur]);
    return;
}

//**FUNZIONE ESEGUITA DAL DIRETTORE:
//  opera in mutua esclusione sulla coda quando riceve una signal
//dal timer in mutua esclusione va a controllare se ci fosse bisogno di
//aprire/chiudere una cassa

void op_dir(int i, int k, int s1, int s2, int quit) {
    lockqueue(q[i]);
    while (sig == -1) {
        waitdirector(q[i]);
    }
    cashier_t* cassa = (cashier_t*)(q[i]->d);
    if (cassa->open == 0 && danger_tail && quit == 0) {
        ((cashier_t*)(q[i]->d))->open = 1;
        danger_tail = 0;
        prec_desk = i;
        desk_open += 1;
    }
    if (q[i]->size > s2 && quit == 0) {
        danger_tail = 1;
    } else {
        if (q[i]->size <= s1 && cassa->open == 1 && quit == 0 && i != prec_desk && desk_open > 1) {
            cassa->open = 0;
            cassa->n_close += 1;
            desk_open -= 1;
        }
    }
    sig = -1;
    signalcashier(q[i]);
    unlockqueue(q[i]);

    return;
}

//routine thread direttore:
//1)effettua controlli  su tutte e tramite la op_dir decide se aprirne o chiuderne qualcuna
//2)controlla se qualche cliente è nella sua coda e in caso affermativo prende il lock
//di dir_mtx e lancia una signal sulla variabile di condizione del cliente facendolo uscire
//3) sia in caso di SIGUP che di SIGQUIT , fa uscire i suoi clienti in coda per poi terminare
void* director(void* arg) {
    while (quit == 0) {
        for (int i = 1; i < k && !quit; i++) {
            op_dir(i, k, s1, s2, quit);
        }
        if (!Empty(q[p_dir])) {
            client_t* t = (client_t*)pop(q, p_dir);
            t->can_quit = 1;
            printf("\n\nil direttore ha fatto uscire il client %d che aveva acquistato %ld prodotti\n\n", t->id, t->products);
            pthread_mutex_lock(&dir_mtx);
            pthread_cond_signal(&(t->cond));
            pthread_mutex_unlock(&dir_mtx);
        }
    }
    while (!Empty(q[p_dir])) {
        client_t* t = (client_t*)pop(q, p_dir);
        t->can_quit = 1;
        printf("\n\nil direttore ha fatto uscire il cliente %d che aveva acquistato %ld prodotti\n\n", t->id, t->products);
        pthread_mutex_lock(&dir_mtx);
        pthread_cond_signal(&(t->cond));
        pthread_mutex_unlock(&dir_mtx);
    }
    //   printf("il direttore è uscito!\n\n\n");
    pthread_exit(NULL);
    return NULL;
}
