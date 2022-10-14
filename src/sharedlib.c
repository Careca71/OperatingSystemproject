#define _POSIX_C_SOURCE 200809L

#include "sharedlib.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#include "utils.h"

#define EMPTY_T NULL

extern int k, c, p, t, e, s1, s2, w_time, t_sing_prod;  //**variabili dal configfile definite su supermercato.c

//**funzione che inizializza una coda e le sue variabili head tail ,mutex e condition variables.
queue_t** MakeNull(int k) {
    queue_t** q;
    q = malloc(k * sizeof(queue_t*));
    for (int i = 0; i < k; i++) {
        q[i] = malloc(sizeof(queue_t));
        if (!q[i]) {
            perror("malloc\n");
            return NULL;
        }
        q[i]->head = NULL;
        q[i]->tail = NULL;
        q[i]->size = 0;
        if (pthread_mutex_init(&q[i]->mux, NULL) != 0) {
            perror("mutex init\n");
            pthread_mutex_destroy(&q[i]->mux);
        }
        if (pthread_cond_init(&q[i]->empty_t, NULL) != 0) {
            perror("cond init\n");
            pthread_cond_destroy(&q[i]->empty_t);
        }
        if (pthread_cond_init(&q[i]->opdir_t, NULL) != 0) {
            perror("cond init\n");
            pthread_cond_destroy(&q[i]->opdir_t);
        }
    }

    return q;
}

//**funzione che verifica che una coda sia vuota

int Empty(queue_t* q) {
    if (q->head == NULL)
        return 1;
    else
        return 0;
}
//**funzione push che inserisce un elemento in coda alla coda , lavora in mutua esclusione
//su ogni coda, manda una signal alla  posizione i delle code una volta inserito un elemento
// aumenta la size della coda rilasciando infine il lock
void push(queue_t** q, int i, void* info_c) {
    struct Nodo* temp;
    temp = malloc(sizeof(struct Nodo));
    if (!temp) {
        perror("malloc nodo\n");
        return;
    }
    temp->info = info_c;
    temp->next = NULL;
    lockqueue(q[i]);
    if (Empty(q[i])) {
        q[i]->head = q[i]->tail = temp;
        q[i]->size += 1;
    } else {
        q[i]->tail->next = temp;
        q[i]->tail = temp;
        q[i]->size += 1;
    }
    signalcashier(q[i]);
    unlockqueue(q[i]);

    return;
}
//**funzione pop che estrae un elemento dalla coda in posizione i,attende
//su una variabile di condizione che qualche elemento venga inserito tramite una pop
//decrementa la size della coda i , rilasciando infine il lock
void* pop(queue_t** q, int i) {
    struct Nodo* temp;
    void** info_c;
    lockqueue(q[i]);
    while (Empty(q[i])) {
        waitcashier(q[i]);
    }
    info_c = q[i]->head->info;
    temp = q[i]->head->next;
    free(q[i]->head);
    q[i]->head = temp;
    if (q[i]->head == NULL) {
        q[i]->tail = NULL;
    }
    q[i]->size -= 1;
    unlockqueue(q[i]);
    return info_c;
}

//**libera la memoria dalle code allocate per il supermercato
void delete_queue(queue_t** q, int k) {
    for (int i = 0; i < k; i++) {
        pthread_mutex_destroy(&q[i]->mux);
        pthread_cond_destroy(&q[i]->empty_t);
        pthread_cond_destroy(&q[i]->opdir_t);
        free(q[i]);
    }
    free(q);
    return;
}

//**controlla i dati in input del configfile
int cofig_control(int k, int c, int p, int t, int e, int s1, int s2, int w_time, int t_sing_prod) {
    fprintf(stdout, "K=%d C=%d P=%d T=%d E=%d S1=%d S2=%d W_TIME=%d T_PROD=%d\n\n", k, c, p, t, e, s1, s2, w_time, t_sing_prod);

    if (k < 1 || c < 1 || p < 0 || t < 10 || e < 1 || s1 < 1 || (s2 < s1 || s2 < 1) || w_time < 1 || t_sing_prod <= 0) {
        fprintf(stderr, "modificare nel modo corretto il file di configurazione!");
        fprintf(stderr, "use :\nK=<value> C=<value> P=<value> T=<NS> E=<value> S1=<value> S2=<value> W_TIME=<NS> T_PROD=<NS>\n MS=millisecond\n\n");
        return 0;
    }

    return 1;
}

//**legge il file di configurazione e ne controlla il contenuto
int read_config_file(int fd) {
    size_t bufsize = 101;
    char* buf = (char*)malloc(bufsize * sizeof(char));
    if (!buf) {
        perror("malloc");
        return -1;
    }
    size_t len;
    if ((len = read(fd, buf, bufsize)) < 0) {
        fprintf(stdout, "errore nella lettura del file\n");
        exit(EXIT_FAILURE);
    }
    sscanf(buf, "K=%d C=%d P=%d T=%d E=%d S1=%d S2=%d W_TIME=%d T_PROD=%d", &k, &c, &p, &t, &e, &s1, &s2, &w_time, &t_sing_prod);
    if (close(fd) != 0) {
        perror("errore nella chiusura\n");
        return -1;
    }
    free(buf);
    if (cofig_control(k, c, p, t, e, s1, s2, w_time, t_sing_prod) == 0) {
        return -1;
    }

    return 1;
}
