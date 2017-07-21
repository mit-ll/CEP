#include <stdio.h>
#include <string.h>

const unsigned int CLOCK_PERIOD = 10;
const unsigned int BITS_PER_BYTE = 8;
const unsigned int BYTES_PER_WORD = 4;
const unsigned int GPS_GEN_NEXT_BYTES = BYTES_PER_WORD;
const unsigned int GPS_CA_BYTES = BYTES_PER_WORD;
const unsigned int GPS_P_BYTES = 128 / BITS_PER_BYTE;
const unsigned int GPS_L_BYTES = 128 / BITS_PER_BYTE;

typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned long vluint64_t;

// Base address of the core on the bus
const uint32_t AES_BASE = 0x9C000000;

// Offset of AES data and control registers in device memory map
const uint32_t GPS_GEN_NEXT = AES_BASE;
const uint32_t GPS_GEN_DONE = AES_BASE;
const uint32_t GPS_CA_BASE = GPS_GEN_NEXT + GPS_GEN_NEXT_BYTES;
const uint32_t GPS_P_BASE = GPS_CA_BASE + GPS_CA_BYTES;
const uint32_t GPS_L_BASE = GPS_P_BASE + GPS_P_BYTES;

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
bool codeReady(void);

void runForClockCycles(const unsigned int pCycles) {
    int doubleCycles = pCycles << 2;
    while(doubleCycles > 0) {
        evalModel();
        toggleClock();
        main_time += (CLOCK_PERIOD >> 2);
        --doubleCycles;
    }
}

void waitForValidOutput(void) {
    while (!codeReady()) {
        runForClockCycles(1);
    }
}
