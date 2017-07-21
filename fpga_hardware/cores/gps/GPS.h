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
void saveCode(uint32_t *pCT, unsigned pBytes, uint32_t pBaseAddress);

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

bool compareCode(uint32_t* pC, const char *pExpectedHexString, uint32_t pBytes) {
  char longTemp[GPS_P_BYTES + 1];
    
  // Convert the calculated code into a text string
  char *temp = longTemp;
  for(int i = (pBytes / BYTES_PER_WORD - 1); i >= 0; --i) {
    sprintf(temp, "%8.8x", (unsigned int)(pC[i]));
    temp += 8;
  }
    
  // Now we can compare them as hex strings
  // 2 string characters per byte
  if(strncmp(longTemp, pExpectedHexString, (pBytes * 2)) == 0) {
    return true;
  }
    
  return false;
}

void verifyCode(const char *pExpectedHexString, const char * pTestString, uint32_t pBytes, uint32_t pBaseAddress) {
  uint32_t c[GPS_P_BYTES / BYTES_PER_WORD];
  printf("Expected:\t0x%s\n", pExpectedHexString);
  /*saveCode(c, pBytes, pBaseAddress);
    
  if(compareCode(c, pExpectedHexString, pBytes)) {
    printf("PASSED: %s\n", pTestString);
  } else {
    printf("Expected:\t0x%s\n", pExpectedHexString);
    printf("FAILED: %s\n", pTestString);
    }*/
}
