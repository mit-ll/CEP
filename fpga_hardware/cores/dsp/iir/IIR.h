#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "input.h"

//#ifndef uint32_t
//typedef unsigned int uint32_t;
//#endif
//#ifndef uint64_t
//typedef unsigned long long uint64_t;
//#endif
#ifndef vluint64_t
typedef unsigned long vluint64_t;
#endif

const unsigned int BITS_PER_BYTE = 8;
const unsigned int BYTES_PER_WORD = 4;
const unsigned int KEY_BITS = 64*3;
const unsigned int BLOCK_BITS = 64;
const unsigned int CLOCK_PERIOD = 10;
const unsigned int DES3_START_BYTES = BYTES_PER_WORD;
const unsigned int DES3_DECRYPT_BYTES = BYTES_PER_WORD;
const unsigned int DES3_DONE_BYTES = BYTES_PER_WORD;

// Automatically generated constants
const unsigned int KEY_BYTES = KEY_BITS / BITS_PER_BYTE;
const unsigned int KEY_WORDS = KEY_BYTES / BYTES_PER_WORD;
const unsigned int BLOCK_BYTES = BLOCK_BITS / BITS_PER_BYTE;
const unsigned int BLOCK_WORDS = BLOCK_BYTES / BYTES_PER_WORD;

// Base address of the core on the bus
const uint32_t DES3_BASE = 0x9b000000;

// Offset of DES3 data and control registers in device memory map
const uint32_t DES3_START = DES3_BASE;
const uint32_t DES3_DECRYPT = DES3_START + DES3_START_BYTES;
const uint32_t DES3_IN_BASE = DES3_DECRYPT + DES3_DECRYPT_BYTES;
const uint32_t DES3_KEY_BASE = DES3_IN_BASE + BLOCK_BYTES;
const uint32_t DES3_DONE = 0x14;//DES3_KEY_BASE + KEY_BYTES;
const uint32_t DES3_CT_BASE = DES3_DONE + DES3_DONE_BYTES;

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
bool readyValid(void);

/*Execute for input cycles*/
void runForClockCycles(const unsigned int pCycles) {
    int doubleCycles = pCycles << 1;
    while(doubleCycles > 0) {
        evalModel();
        toggleClock();
        main_time += (CLOCK_PERIOD >> 1);
        --doubleCycles;
    }
}

/*Wait for modexp ready signal*/
void waitForValidOutput(){
    while (!readyValid()) {
        runForClockCycles(1);
    }
}

/*Check if both inputs are equal*/
bool assertEquals(int index, uint32_t expected, uint32_t actual){

    if (expected == actual){
    printf("*** PASSED   (%02d): %08x Got: %08x\r\n", index, expected, actual);
     return true;// success
    }else{                   
    printf("*** Expected (%02d): %08x Got: %08x\r\n", index, expected, actual);
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

