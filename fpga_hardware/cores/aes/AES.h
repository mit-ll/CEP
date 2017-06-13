#include <iostream>
#include <iomanip>
#include <cstdio>
using namespace std;


const unsigned int BLOCK_BITS = 128;
const unsigned int CLOCK_PERIOD = 10;

#ifndef uint32_t
  typedef unsigned int uint32_t;
#endif

#ifndef vluint64_t
  typedef unsigned long vluint64_t;
#endif

// Base address of the core on the bus
const uint32_t AES_BASE = 0x92000000;

// Offset of AES data and control registers in device memory map
const uint32_t AES_START = AES_BASE;
const uint32_t AES_START_BYTES = 4;
const uint32_t AES_PT_BASE = AES_START + AES_START_BYTES;
const uint32_t AES_PT_BYTES = 4 * 4;
const uint32_t AES_KEY_BASE = AES_PT_BASE + AES_PT_BYTES;
const uint32_t AES_KEY_BYTES = 4 * 4;
const uint32_t AES_DONE = AES_KEY_BASE + AES_KEY_BYTES;
const uint32_t AES_DONE_BYTES = 1;
const uint32_t AES_CT_BASE = AES_DONE + AES_DONE_BYTES;

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
bool ciphertextValid(void);
void start(void);
void saveCiphertext(uint32_t *pCT);
void setPlaintext(const char* pPT);
void setPlaintext(const uint32_t* pPT);
void setKey(const uint32_t* pKey);

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
    while (!ciphertextValid()) {
        runForClockCycles(1);
    }
}

bool compareCiphertext(uint32_t* pCT, const char *pExpectedHexString) {
    for(int i = (BLOCK_BITS / 32) - 1; i >= 0; --i) {
        for(int j = 3; j >= 0; --j) {
            if(((char *)(&(pCT[i])))[j] == pExpectedHexString[j + (i * 4)])
                return false;
        }
    }
    
    return true;
}

void verifyCiphertext(const char *pExpectedHexString, const char * pTestString) {
    uint32_t ct[BLOCK_BITS / 32];
    saveCiphertext(ct);
    
    if(compareCiphertext(ct, pExpectedHexString)) {
        cout << "PASSED: " << pTestString << endl;
    } else {
        cout << "FAILED: " << pTestString << endl;
    }
}
