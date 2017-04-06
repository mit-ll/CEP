// State and functionality common to all tests

// Base address of the core on the bus
const uint32_t AES_BASE = 0xF0000000;

// Offset of AES data and control registers in device memory map
const uint32_t AES_START = 0;
const uint32_t PT_BASE = 1;
const uint32_t KEY_BASE = 5;
const uint32_t AES_DONE = 9;
const uint32_t CT_BASE = 10;

// Level-dependent functions
void runForClockCycles(const unsigned int pCycles);
bool ciphertextValid(void);
void start(void);
void saveCiphertext(uint32_t *pCT);
void setPlaintext(const char* pPT);
void setPlaintext(const uint32_t* pPT);
void setKey(const uint32_t* pKey);

void waitForValidOutput(void) {
    while (!ciphertextValid()) {
        runForClockCycles(1);
    }
}

bool compareCiphertext(uint32_t* pCT, const char *pExpectedHexString) {
    for(int i = (128 / 32) - 1; i >= 0; --i) {
        for(int j = 3; j >= 0; --j) {
            if(((char *)(&(pCT[i])))[j] == pExpectedHexString[j + (i * 4)])
                return false;
        }
    }
    
    return true;
}

void verifyCiphertext(const char *pExpectedHexString, const char * pTestString) {
    uint32_t ct[128 / 32];
    saveCiphertext(ct);
    
    if(compareCiphertext(ct, pExpectedHexString)) {
        cout << "PASSED: " << pTestString << endl;
    } else {
        cout << "FAILED: " << pTestString << endl;
    }
}

void printWordAsBytes(uint32_t pWord) {
    cout << hex << uppercase << setfill('0') << setw(2) << ((pWord & 0xFF000000) >> 24);
    cout << hex << uppercase << setfill('0') << setw(2) << ((pWord & 0xFF0000) >> 16);
    cout << hex << uppercase << setfill('0') << setw(2) << ((pWord & 0xFF00) >> 8);
    cout << hex << uppercase << setfill('0') << setw(2) << (pWord & 0xFF);
}
