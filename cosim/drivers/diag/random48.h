#ifndef Random48_H
#define Random48_H


#include <stddef.h>
#include <stdlib.h>

//#include <sys/types.h>
#include "simdiag_global.h"

//
// random generator
//
extern void Random48_srand48(u_long seed);
extern u_long Random48_mrand48 (void);
extern int Random48_rand(void);
extern u_long Random48_mrand48_custom (u_long *Random48_customDiagRandomNumber);

#endif // RANDOM48_H

