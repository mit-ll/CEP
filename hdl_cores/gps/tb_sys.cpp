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

void saveCode(uint32_t *pCT, unsigned pBytes, uint32_t pBaseAddress) {
    for(unsigned int i = 0; i < (pBytes / BYTES_PER_WORD); ++i) {
        pCT[((pBytes / BYTES_PER_WORD) - 1) - i] = readFromAddress(pBaseAddress + (i * BYTES_PER_WORD));
    }
}

void reportCode(const char * pCode, unsigned pBytes, uint32_t pBaseAddress) {
    printf("%s code:\t0x", pCode);
    uint32_t ct[GPS_P_BYTES / BYTES_PER_WORD + 1];

    saveCode(ct, pBytes, pBaseAddress);
    for(unsigned int i = 0; i < (pBytes / BYTES_PER_WORD); ++i) {
      printf("%08X", (unsigned int)(ct[((pBytes / BYTES_PER_WORD) - 1) - i]));
    }
    printf("\n");
}

int main(int argc, char **argv, char **env) {
    printf("Reset complete\n");
    
    printf("Generating next code batch\n");
    generateNextCode();
    waitForValidOutput();
    reportCode("CA", GPS_CA_BYTES, GPS_CA_BASE);
    verifyCode("00001F43", "C/A code", GPS_CA_BYTES, GPS_CA_BASE);
    reportCode("P", GPS_P_BYTES, GPS_P_BASE);
    verifyCode("9DDFDD9CE127D8D95394BF2838E7EF54", "P code", GPS_P_BYTES, GPS_P_BASE);
    reportCode("L", GPS_L_BYTES, GPS_L_BASE);
    verifyCode("FD86C6D34155A750E284B05AC643BC27", "L code", GPS_L_BYTES, GPS_L_BASE);

    return 0;
}
