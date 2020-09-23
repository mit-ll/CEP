//
// "random48.c" - description of file
//
#include "random48.h"

static volatile u_long Random48_customDiagRandomNumber= 0;

void Random48_srand48 (u_long seed) {
  Random48_customDiagRandomNumber = seed;
}

u_long Random48_mrand48 (void) {
    u_long a = 48271;             // multiplier (see Fishman & Moore)
    u_long m = 2147483647;        // modulus   = 2^31-1
    u_long q = 44488;             // quotient  = m div a
    u_long r = 3399;              // remainder = m mod a
    u_long divQ = Random48_customDiagRandomNumber/q;
    u_long modQ = Random48_customDiagRandomNumber%q;
    u_long newValue = a*modQ - r*divQ;
    return Random48_customDiagRandomNumber = (newValue > 0)? (u_long)newValue: ((u_long)newValue + m);
}                                   

u_long Random48_mrand48_custom (u_long *Random48_customDiagRandomNumber) {
    u_long a = 48271;             // multiplier (see Fishman & Moore)
    u_long m = 2147483647;        // modulus   = 2^31-1
    u_long q = 44488;             // quotient  = m div a
    u_long r = 3399;              // remainder = m mod a
    u_long divQ = *Random48_customDiagRandomNumber/q;
    u_long modQ = *Random48_customDiagRandomNumber%q;
    u_long newValue = a*modQ - r*divQ;
    return *Random48_customDiagRandomNumber = (newValue > 0)? (u_long)newValue: ((u_long)newValue + m);
}                                   

int Random48_rand(void) {
  //  return abs(Random48_mrand48());
  return Random48_mrand48();
}

