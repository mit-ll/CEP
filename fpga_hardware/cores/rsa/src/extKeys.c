#include<stdio.h>
#include<stdlib.h>

#include"define.h"
#include"input.h"

#include"file.h"
#include"extKeys.h"

/*Main*/
int main(void){
    char *FPRI="", *FPUB="";
    int   SPRI=0,   SPUB=0;
    int   i=0;

    //Read Keys into memory
    if(rFile(&FPRI, &SPRI, "./pem/prikey.pem"))exit(EXIT_FAILURE);
    if(rFile(&FPUB, &SPUB, "./pem/pubkey.pem"))exit(EXIT_FAILURE);
    
    //Extra Key information -> Base64 to Hex
    if(priKey(FPRI, SPRI)); else exit(EXIT_FAILURE);
    if(pubKey(FPUB, SPUB)); else exit(EXIT_FAILURE);
    
    //Write desired values to files
    if(PUB_MODULUS)          if(wFile(pubMOD,    64, "./gen/modulus"))exit(EXIT_FAILURE);
    if(PUB_EXPONENT)         if(wFile(pubEXP,    64, "./gen/exponent"))exit(EXIT_FAILURE);
    if(PRI_ALGORITHM_VERSION)if(wFile(priALG,    64, "./gen/pubkey.pem"))exit(EXIT_FAILURE);
    if(PRI_MODULUS)          if(wFile(priMOD,    64, "./gen/priModulus"))exit(EXIT_FAILURE);
    if(PRI_PUBLIC_EXPONENT)  if(wFile(priPUBEXP, 64, "./gen/pubExponent"))exit(EXIT_FAILURE);
    if(PRI_PRIVATE_EXPONENT) if(wFile(priPRIEXP, 64, "./gen/priExponent"))exit(EXIT_FAILURE);
    if(PRI_PRIME_1)          if(wFile(priPRI1,   64, "./gen/prime1"))exit(EXIT_FAILURE);
    if(PRI_PRIME_2)          if(wFile(priPRI2,   64, "./gen/prime2"))exit(EXIT_FAILURE);
    if(PRI_EXPONENT_1)       if(wFile(priEXP1,   64, "./gen/exponent1"))exit(EXIT_FAILURE);
    if(PRI_EXPONENT_2)       if(wFile(priEXP2,   64, "./gen/exponent2"))exit(EXIT_FAILURE);
    if(PRI_COEFFICIENT)      if(wFile(priCOE,    64, "./gen/coefficient"))exit(EXIT_FAILURE);

    #if DISPLAY
    printf("Completed\n\r");
    #endif
    exit(EXIT_SUCCESS);
    return 0;
}

