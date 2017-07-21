#include "GPS.h"

// Need this to fully implement expected interface
void toggleClock() {;}
void evalModel() {;}

// Need volatile to make sure the compiler doesn't make bus writes/read disappear
uint32_t readFromAddress(uint32_t pAddress) {
    return *((volatile uint32_t *)pAddress);
}

void writeToAddress(uint32_t pAddress, uint32_t pData) {
    *((volatile uint32_t *)pAddress) = pData;
}

void generateNextCode(void) {
    writeToAddress(GPS_GEN_NEXT, 0x1);
    writeToAddress(GPS_GEN_NEXT, 0x0);
}

bool codeReady(void) {
    return (readFromAddress(GPS_GEN_DONE) != 0) ? true : false;
}

void saveCACode(uint32_t *pCT) {
    for(unsigned int i = 0; i < (GPS_CA_BYTES / BYTES_PER_WORD); ++i) {
        pCT[((GPS_CA_BYTES / BYTES_PER_WORD) - 1) - i] = readFromAddress(GPS_CA_BASE + (i * BYTES_PER_WORD));
    }
}

void reportCACode(void) {
    printf("C/A code:\t0x");
    uint32_t ct[GPS_CA_BYTES / BYTES_PER_WORD + 1];

    saveCACode(ct);
    for(unsigned int i = 0; i < (GPS_CA_BYTES/ BYTES_PER_WORD); ++i) {
      printf("%08X", ct[((GPS_CA_BYTES / BYTES_PER_WORD) - 1) - i]);
    }
    printf("\n");
}

int main(int argc, char **argv, char **env) {
    printf("Reset complete\n");
    
    printf("Generating next code batch\n");
    
    generateNextCode();
    
    waitForValidOutput();
    reportCACode();
    printf("P code: \n");
    //reportPCode();
    printf("L code: \n");
    //reportLCode();
    
    return 0;
}
