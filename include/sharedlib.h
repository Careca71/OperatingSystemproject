#if !defined(SHARED_LIB_H)
#define SHARED_LIB_H

#include <pthread.h>

#include "sharedlib.h"
#include "utils.h"

//****** Struct Nodo
struct Nodo {
    void **info;
    struct Nodo *next;
};

//****** Struct Queue , definizione di coda con due elementi Nodo , uno che punta alla testa e uno alla coda.
typedef struct queue {
    struct Nodo *head;	//**puntatore alla testa della cosa
    struct Nodo *tail;	//**puntatore all'ultimo elemento in coda
    void *d;			//**campo void che porterà i valori di una cassa
    int size;			//**lunghezza della coda
    pthread_mutex_t mux;	//**mutex per la sincronizzazione tra le operazioni di push e pop per una coda
    pthread_cond_t empty_t;	//** variabile di condizione sulle quali si fermano le code vuote
    pthread_cond_t opdir_t;	//** variabile di consizione per la sincronizzazione con il direttore
} queue_t;

//** funzioni di utilità , descrivono le operazioni su mutex e variabili di condizioni sulla coda
static inline void lockqueue(queue_t *q) { pthread_mutex_lock(&q->mux); }
static inline void unlockqueue(queue_t *q) { pthread_mutex_unlock(&q->mux); }
static inline void waitcashier(queue_t *q) { pthread_cond_wait(&q->empty_t, &q->mux); }
static inline void signalcashier(queue_t *q) { pthread_cond_signal(&q->empty_t); }
static inline void waitdirector(queue_t *q) { pthread_cond_wait(&q->opdir_t, &q->mux); }
static inline void signaldirector(queue_t *q) { pthread_cond_signal(&q->opdir_t); }

//**funzione che inizializza la mia coda e inizializza le mutex e le variabili di condizione .
queue_t **MakeNull(int k);

//**funzione che ritorna 1 se la coda è vuota 0 altrimenti
int Empty(queue_t *q);

//**funzione push che inserisce un elemento in coda
void push(queue_t **q, int i, void *info_c);

//**funzione pop che estrae un elemento dalla cima della coda
void *pop(queue_t **q, int i);

//**funzione che libera tutta la memoria allocata della coda
void delete_queue(queue_t **q, int k);

//**funzione che controlla i dati dal file di configurazione
int cofig_control(int k, int c, int p, int t, int e, int s1, int s2, int w_time, int t_sing_prod);

//** funzione che legge il file di configurazione e ne controlla i dati
int read_config_file(int fd);

#endif