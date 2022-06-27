#include <stdio.h>
#include <stdlib.h>

#include "input.h"
#include "define.h"
#include "extKeys.h"
#include "ascii2hex.h"
#include "hex2ascii.h"

#include "../pem/pubkey.h"
#include "../pem/prikey.h"

const unsigned int CLOCK_PERIOD = 10;

// Current simulation time
vluint64_t main_time = 0;

// Called by $time in Verilog
// converts to double, to match
// what SystemC does
double sc_time_stamp () {
    return main_time;
}

// Level-dependent functions
void evalModel();
void toggleClock(void);
uint32_t read_word(uint32_t pAddress);
void write_word(uint32_t pAddress, uint32_t pData);

/*Execute for input cycles*/
void runForClockCycles(const unsigned int pCycles) {
    int doubleCycles = pCycles << 2;
    while(doubleCycles > 0) {
        evalModel();
        toggleClock();
        main_time += (CLOCK_PERIOD >> 2);
        --doubleCycles;
    }
}

/*Wait for modexp ready signal*/
void wait_ready(){
    while(read_word(ADDR_STATUS)!= 0x00000001);
}

/*Check if both inputs are equal*/
bool assertEquals(uint32_t expected, uint32_t actual){
    if (expected == actual) return true;// success
    else{                   printf("*** Expected: %08X, got %08X\r\n", (unsigned int) expected, (unsigned int) actual);
                            return false;// failure
    }
}

/*Verify outcome of testcases*/
bool assertSuccess(bool success){
    if (success){ printf("*** Test    -> passed\r\n");        
                  return true;
    }else{        printf("*** Test    -> FAILED\r\n");
                  return false;
    }
}

/*Preset Test 32-bit exponent with 2048-bit Modulus*/
void exp32bit_mod2048bit_test(){
    int i;
    bool success=true;

    printf("\r\n");
    printf("Test with e = 65537 and 2048 bit modulus -- Encrypting\r\n");

    write_word(ADDR_EXPONENT_PTR_RST, 0x00000000);
    write_word(ADDR_EXPONENT_DATA   , EXP);

    write_word(ADDR_MODULUS_PTR_RST, 0x00000000);
    write_word(ADDR_MODULUS_DATA   , 0x00000000);

    for(i=0; i<64; i=i+1) {
        write_word(ADDR_MODULUS_DATA, MOD[i]);
        if(DEBUG) printf("writing: %d -> %08X\r\n", i, (unsigned int) MOD[i]);
    }

    write_word(ADDR_MESSAGE_PTR_RST, 0x00000000);
    write_word(ADDR_MESSAGE_DATA   , 0x00000000);

    for(i=0; i<64; i=i+1) {
        write_word(ADDR_MESSAGE_DATA, MSG[i]);
        if(DEBUG) printf("Writing: %d -> %08X\r\n", i, (unsigned int) MSG[i]);
    }

    write_word(ADDR_EXPONENT_LENGTH, 0x00000001);
    write_word(ADDR_MODULUS_LENGTH , 0x00000041);

    // Start processing and wait for ready.
    write_word(ADDR_CTRL, 0x00000001);
    wait_ready();

    write_word(ADDR_RESULT_PTR_RST, 0x00000000);
    RES[0]=read_word(ADDR_RESULT_DATA);
    success=success&assertEquals(0x00000000, (unsigned int) RES[0]);

    for(i=0; i<64; i=i+1) {
        RES[i]=read_word(ADDR_RESULT_DATA);
        success=success&assertEquals(RES[i], OUT[i]);
        if(DEBUG) printf("Reading: %d -> %08X -> %08X\r\n", i, (unsigned int) RES[i], (unsigned int) OUT[i]);
    }

    assertSuccess(success);
}

/*32-bit modulo exponentiation*/
void modexp_32bits(uint32_t Wmsg, uint32_t Wexp, uint32_t Wmod, uint32_t Wres){
    uint32_t Rres;
    bool success=true;
    
    printf("\r\n");
    printf("*** Running -> modexp_32bits()\r\n");
  
    printf("*** Writing -> MES: %08X EXP: %08X MOD: %08X\r\n", (unsigned int) Wmsg, (unsigned int) Wexp, (unsigned int) Wmod);

    write_word(ADDR_EXPONENT_PTR_RST, 0x00000000);
    write_word(ADDR_EXPONENT_DATA   , Wexp);

    write_word(ADDR_MODULUS_PTR_RST , 0x00000000);
    write_word(ADDR_MODULUS_DATA    , Wmod);

    write_word(ADDR_MESSAGE_PTR_RST , 0x00000000);
    write_word(ADDR_MESSAGE_DATA    , Wmsg);

    write_word(ADDR_EXPONENT_LENGTH , 0x00000001);
    write_word(ADDR_MODULUS_LENGTH  , 0x00000001);

    // Start processing and wait for ready.
    write_word(ADDR_CTRL            , 0x00000001);
    wait_ready();

    write_word(ADDR_RESULT_PTR_RST  , 0x00000000);
    Rres=read_word(ADDR_RESULT_DATA);
     
    success=success&assertEquals(Wres, Rres);
    assertSuccess(success);
}

/*RSA Encrypt*/
void modexp_encrypt(){
    int i;
    bool success=true;

    printf("\r\n");
    printf("Encrypting -- exp = 65537 and 2048 bit mod\r\n");

    write_word(ADDR_EXPONENT_PTR_RST, 0x00000000);
    write_word(ADDR_EXPONENT_DATA   , pubEXP[0]);
    if(DEBUG) printf("Writing EXP: %d %08X\r\n", (int) pubEXP[0], (unsigned int) pubEXP[0]);

    write_word(ADDR_MODULUS_PTR_RST, 0x00000000);
    write_word(ADDR_MODULUS_DATA   , 0x00000000);

    for(i=0; i<64; i=i+1) {
        write_word(ADDR_MODULUS_DATA, pubMOD[i]);
        if(DEBUG) printf("Writing MOD: %d -> %08X\r\n", i, (unsigned int) pubMOD[i]);
    }

    write_word(ADDR_MESSAGE_PTR_RST, 0x00000000);
    write_word(ADDR_MESSAGE_DATA   , 0x00000000);

    for(i=0; i<64; i=i+1) {
        write_word(ADDR_MESSAGE_DATA, PMSG[i]);
        if(DEBUG) printf("Writing MSG: %d -> %08X\r\n", i, (unsigned int) PMSG[i]);
    }

    write_word(ADDR_EXPONENT_LENGTH, 0x00000001);
    write_word(ADDR_MODULUS_LENGTH , 0x00000041);

    // Start processing and wait for ready.
    write_word(ADDR_CTRL, 0x00000001);
    wait_ready();

    write_word(ADDR_RESULT_PTR_RST, 0x00000000);
    CMSG[0]=read_word(ADDR_RESULT_DATA);
    success=success&assertEquals(0x00000000, CMSG[0]);

    for(i=0; i<64; i=i+1) {
        CMSG[i]=read_word(ADDR_RESULT_DATA);
        if(DEBUG) printf("Reading: %d -> %08X \r\n", i, (unsigned int) CMSG[i]);
    }

    assertSuccess(success);
}

/*RSA Decrypt*/
void modexp_decrypt(){
    int i;
    bool success=true;

    printf("\r\n");
    printf("*** Running -> modexp_decrypt()\r\n");

    printf("Decrypting -- 2048 bit exp and 2048 bit mod\r\n");

    write_word(ADDR_EXPONENT_PTR_RST, 0x00000000);
    write_word(ADDR_EXPONENT_DATA   , 0x00000000);
    for(i=0; i<64; i=i+1) {
        write_word(ADDR_EXPONENT_DATA, priPRIEXP[i]);
        if(DEBUG) printf("Writing EXP: %d -> %08X\r\n", i, (unsigned int) priPRIEXP[i]);
    }

    write_word(ADDR_MODULUS_PTR_RST, 0x00000000);
    write_word(ADDR_MODULUS_DATA   , 0x00000000);

    for(i=0; i<64; i=i+1) {
        write_word(ADDR_MODULUS_DATA, priMOD[i]);
        if(DEBUG) printf("Writing MOD: %d -> %08X\r\n", i, (unsigned int) priMOD[i]);
    }

    write_word(ADDR_MESSAGE_PTR_RST, 0x00000000);
    write_word(ADDR_MESSAGE_DATA   , 0x00000000);

    for(i=0; i<64; i=i+1) {
        write_word(ADDR_MESSAGE_DATA, CMSG[i]);
        if(DEBUG) printf("Writing MSG: %d -> %08X\r\n", i, (unsigned int) CMSG[i]);
    }

    write_word(ADDR_EXPONENT_LENGTH, 0x00000041);
    write_word(ADDR_MODULUS_LENGTH , 0x00000041);

    // Start processing and wait for ready.
    write_word(ADDR_CTRL, 0x00000001);
    wait_ready();

    write_word(ADDR_RESULT_PTR_RST, 0x00000000);
    GMSG[0]=read_word(ADDR_RESULT_DATA);
    success=success&assertEquals(0x00000000, GMSG[0]);

    for(i=0; i<64; i=i+1) {
        GMSG[i]=read_word(ADDR_RESULT_DATA);
        if(DEBUG) printf("Reading: %d -> %08X \r\n", i, (unsigned int) GMSG[i]);
        success=success&assertEquals(PMSG[i], GMSG[i]);
    }
    assertSuccess(success);
}

