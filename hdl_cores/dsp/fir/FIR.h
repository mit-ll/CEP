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

const unsigned int BITS_PER_BYTE   = 8;
const unsigned int BYTES_PER_WORD  = 4;
const unsigned int BLOCK_BITS      = 32;
const unsigned int CLOCK_PERIOD    = 10;

// Automatically generated constants
const unsigned int BLOCK_BYTES = BLOCK_BITS / BITS_PER_BYTE;
const unsigned int BLOCK_WORDS = BLOCK_BYTES / BYTES_PER_WORD;

// Base address of the core on the bus
const uint32_t FIR_BASE = 0x9a000000;

// Offset of FIR data and control registers in device memory map
const uint32_t FIR_START    = FIR_BASE;
const uint32_t FIR_IN_WRITE = FIR_START    + BYTES_PER_WORD;
const uint32_t FIR_IN_ADDR  = FIR_IN_WRITE + BYTES_PER_WORD;
const uint32_t FIR_IN_DATA  = FIR_IN_ADDR  + BLOCK_BYTES;
const uint32_t FIR_OUT_ADDR = FIR_IN_DATA  + BLOCK_BYTES;
const uint32_t FIR_OUT_DATA = FIR_OUT_ADDR + BLOCK_BYTES;
const uint32_t FIR_DONE     = FIR_OUT_DATA + BLOCK_BYTES;

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
    printf("*** PASSED   (%02d): %08x Got: %08x\r\n", index, (unsigned int)expected, (unsigned int)actual);
     return true;// success
    }else{                   
    printf("*** Expected (%02d): %08x Got: %08x\r\n", index, (unsigned int)expected, (unsigned int)actual);
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

