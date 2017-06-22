#include <stdio.h>
#include <stdlib.h>

//#include "input.h"

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
    //while(read_word(ADDR_STATUS)!= 0x00000001);
}

/*Check if both inputs are equal*/
bool assertEquals(int index, uint64_t expected, uint64_t actual){

    if (expected == actual){
    printf("*** Expected (%0d): %08x%08x Got %08x%08x\r\n", index, expected>>32, expected, actual>>32, actual);
     return true;// success
    }else{                   
    printf("*** Expected (%0d): %08x%08x Got %08x%08x\r\n", index, expected>>32, expected, actual>>32, actual);
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


