//
// "random48.c" - description of file
//
#include "random48.h"

static volatile uint64_t Random48_customDiagRandomNumber= 0;

void Random48_srand48 (uint64_t seed) {
  Random48_customDiagRandomNumber = seed;
}

uint64_t Random48_mrand48 (void) {
    uint64_t a = 48271;             // multiplier (see Fishman & Moore)
    uint64_t m = 2147483647;        // modulus   = 2^31-1
    uint64_t q = 44488;             // quotient  = m div a
    uint64_t r = 3399;              // remainder = m mod a
    uint64_t divQ = Random48_customDiagRandomNumber/q;
    uint64_t modQ = Random48_customDiagRandomNumber%q;
    uint64_t newValue = a*modQ - r*divQ;
    return Random48_customDiagRandomNumber = (newValue > 0)? (uint64_t)newValue: ((uint64_t)newValue + m);
}                                   

uint64_t Random48_mrand48_custom (uint64_t *Random48_customDiagRandomNumber) {
    uint64_t a = 48271;             // multiplier (see Fishman & Moore)
    uint64_t m = 2147483647;        // modulus   = 2^31-1
    uint64_t q = 44488;             // quotient  = m div a
    uint64_t r = 3399;              // remainder = m mod a
    uint64_t divQ = *Random48_customDiagRandomNumber/q;
    uint64_t modQ = *Random48_customDiagRandomNumber%q;
    uint64_t newValue = a*modQ - r*divQ;
    return *Random48_customDiagRandomNumber = (newValue > 0)? (uint64_t)newValue: ((uint64_t)newValue + m);
}                                   

int Random48_rand(void) {
  //  return abs(Random48_mrand48());
  return Random48_mrand48();
}

