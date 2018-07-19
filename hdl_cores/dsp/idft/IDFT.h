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
const uint32_t IDFT_BASE = 0x99000000;

// Offset of IDFT data and control registers in device memory map
const uint32_t IDFT_START    = IDFT_BASE;
const uint32_t IDFT_IN_WRITE = IDFT_START    + BYTES_PER_WORD;
const uint32_t IDFT_IN_ADDR  = IDFT_IN_WRITE + BYTES_PER_WORD;
const uint32_t IDFT_IN_DATA  = IDFT_IN_ADDR  + BLOCK_BYTES;
const uint32_t IDFT_OUT_ADDR = IDFT_IN_DATA  + BLOCK_BYTES + BLOCK_BYTES;
const uint32_t IDFT_OUT_DATA = IDFT_OUT_ADDR + BLOCK_BYTES;
const uint32_t IDFT_DONE     = IDFT_OUT_DATA + BLOCK_BYTES + BLOCK_BYTES;

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
bool assertEquals(int index, uint16_t expected[4], uint16_t actual[4]){

    if ((expected[0] == actual[0])&
        (expected[1] == actual[1])&
        (expected[2] == actual[2])&
        (expected[3] == actual[3])){
    printf("*** PASSED   (%02d): %04x %04x %04x %04x Got: %04x %04x %04x %04x\r\n", index, expected[0], expected[1], expected[2], expected[3], actual[0], actual[1], actual[2], actual[3]);
     return true;// success
    }else{                   
    printf("*** Expected (%02d): %04x %04x %04x %04x Got: %04x %04x %04x %04x\r\n", index, expected[0], expected[1], expected[2], expected[3], actual[0], actual[1], actual[2], actual[3]);
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

