#include <stdio.h>
#include <string.h>

const unsigned int BITS_PER_BYTE = 8;
const unsigned int BYTES_PER_WORD = 4;
const unsigned int KEY_BITS = 192;
const unsigned int BLOCK_BITS = 128;
const unsigned int CLOCK_PERIOD = 10;
const unsigned int AES_START_BYTES = BYTES_PER_WORD;
const unsigned int AES_DONE_BYTES = BYTES_PER_WORD;

// Automatically generated constants
const unsigned int KEY_BYTES = KEY_BITS / BITS_PER_BYTE;
const unsigned int KEY_WORDS = KEY_BYTES / BYTES_PER_WORD;
const unsigned int BLOCK_BYTES = BLOCK_BITS / BITS_PER_BYTE;
const unsigned int BLOCK_WORDS = BLOCK_BYTES / BYTES_PER_WORD;

typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned long vluint64_t;

// Base address of the core on the bus
const uint32_t AES_BASE = 0x93000000;

// Offset of AES data and control registers in device memory map
const uint32_t AES_START = AES_BASE;
const uint32_t AES_PT_BASE = AES_START + AES_START_BYTES;
const uint32_t AES_KEY_BASE = AES_PT_BASE + BLOCK_BYTES;
const uint32_t AES_DONE = AES_KEY_BASE + KEY_BYTES;
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
    char longTemp[(BLOCK_BITS / 4) + 1];
    
    // Convert the calculated ciphertext into a text string
    char *temp = longTemp;
    for(int i = (BLOCK_WORDS - 1); i >= 0; --i) {
        sprintf(temp, "%8.8x", ((uint32_t *)pCT)[i]);
        temp += 8;
    }
    
    // Now we can compare them as hex strings
    // 2 string characters per byte
    if(strncmp(longTemp, pExpectedHexString, (BLOCK_BYTES * 2)) == 0) {
        return true;
    }
    
    return false;
}

void verifyCiphertext(const char *pExpectedHexString, const char * pTestString) {
    uint32_t ct[BLOCK_WORDS];
    saveCiphertext(ct);
    
    if(compareCiphertext(ct, pExpectedHexString)) {
      printf("PASSED: %s\n", pTestString);
    } else {
      printf("Expected:\t0x%s\n", pExpectedHexString);
      printf("FAILED: %s\n", pTestString);
    }
}
