#define _POSIX_C_SOURCE 200809L

#if !defined(DIRECTOR_H)
#define DIRECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "cashier.h"
#include "sharedlib.h"


//**funzione che lancia una signal al direttore
void sig_dir(int cur);

//** descrive ed effettua le operazioni del thread direttore
void op_dir( int i, int k, int s1, int s2, int quit);

//** routine thread direttore
void *director(void *arg);


#endif /*DIRECTOR_H*/