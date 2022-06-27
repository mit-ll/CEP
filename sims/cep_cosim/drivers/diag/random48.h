#ifndef Random48_H
#define Random48_H


#include <stddef.h>
#include <stdlib.h>

#include <stdint.h>

//#include <sys/types.h>
#include "simdiag_global.h"

//
// random generator
//
extern void Random48_srand48(uint64_t seed);
extern uint64_t Random48_mrand48 (void);
extern int Random48_rand(void);
extern uint64_t Random48_mrand48_custom (uint64_t *Random48_customDiagRandomNumber);

#endif // RANDOM48_H

