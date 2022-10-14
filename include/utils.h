#define _POSIX_C_SOURCE 200809L
#if !defined(UTIL_H)
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sharedlib.h"

//** generatore di numeri casuali
#define GENERATE_RANDOM(seed, max, min) \
    ((rand_r(seed) % (max - min + 1)) + min)

//** convertitore da millisecondi a secondi
#define MS_TO_S(s) \
    ((s) / 1000)

#endif /* UTIL_H */