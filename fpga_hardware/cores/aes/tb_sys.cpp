#include "AES.h"

// Need this to fully implement expected interface
void toggleClock() {;}
void evalModel() {;}

uint32_t readFromAddress(uint32_t pAddress) {
    return *((uint32_t *)pAddress);
}

void writeToAddress(uint32_t pAddress, uint32_t pData) {
    *((uint32_t *)pAddress) = pData;
}

bool ciphertextValid(void) {
    return (readFromAddress(AES_DONE) != 0) ? true : false;
}

void start(void) {
    writeToAddress(AES_START, 0x1);
    writeToAddress(AES_START, 0x0);
}

void reportCiphertext(void) {
    printf("Ciphertext:\t0x");
    for(int i = (BLOCK_BITS / 32) - 1; i >= 0; --i) {
        printf("%08X", readFromAddress(AES_CT_BASE + (i * 4)));
    }
    printf("\n");
}

void saveCiphertext(uint32_t *pCT) {
    for(int i = 0; i < (BLOCK_BITS / 32); ++i) {
        *pCT++ = readFromAddress(AES_CT_BASE + (i * 4));
    }
}

void setPlaintext(const char* pPT) {
    cout << "Plaintext:\t0x";
    for(int i = 0; i < (BLOCK_BITS / 8); ++i) {
        uint32_t temp;
        for(int j = 0; j < 4; ++j) {
            ((char *)(&temp))[3 - j] = *pPT++;
        }
        writeToAddress(AES_PT_BASE + ((3 - i) * 4), temp);
        cout << hex << setfill('0') << setw(8) << temp;
    }
    cout << endl;
}

void setPlaintext(const uint32_t* pPT) {
    cout << "Plaintext:\t0x";
    for(int i = 0; i < (BLOCK_BITS / 32); ++i) {
        writeToAddress(AES_PT_BASE + ((3 - i) * 4), pPT[i]);
        cout << hex << setfill('0') << setw(8) << pPT[i];
    }
    cout << endl;
}

void setKey(const uint32_t* pKey) {
    cout << "Key:\t\t0x";
    for(int i = 0; i < (BLOCK_BITS / 32); ++i) {
        writeToAddress(AES_KEY_BASE + ((3 - i) * 4), pKey[i]);
        cout << hex << setfill('0') << setw(8) << pKey[i];
    }
    cout << endl;
}

int main(int argc, char **argv, char **env) {
    uint32_t pt1[4]  = {0x3243f6a8, 0x885a308d, 0x313198a2, 0xe0370734};
    uint32_t key1[4] = {0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c};
    setPlaintext(pt1);
    setKey(key1);
    start();
    waitForValidOutput();
    reportCiphertext();
    verifyCiphertext("3925841d02dc09fbdc118597196a0b32", "test 1");
    
    
    
    uint32_t pt2[4]  = {0x00112233, 0x44556677, 0x8899aabb, 0xccddeeff};
    uint32_t key2[4] = {0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f};
    setPlaintext(pt2);
    setKey(key2);
    start();
    waitForValidOutput();
    reportCiphertext();
    verifyCiphertext("69c4e0d86a7b0430d8cdb78070b4c55a", "test 2");
    
    
    
    uint32_t pt3[4]  = {0, 0, 0, 0};
    uint32_t key3[4] = {0, 0, 0, 0};
    setPlaintext(pt3);
    setKey(key3);
    start();
    waitForValidOutput();
    reportCiphertext();
    verifyCiphertext("66e94bd4ef8a2c3b884cfa59ca342b2e", "test 3");
    
    
    
    uint32_t pt4[4]  = {0, 0, 0, 0};
    uint32_t key4[4] = {0, 0, 0, 1};
    setPlaintext(pt4);
    setKey(key4);
    start();
    waitForValidOutput();
    reportCiphertext();
    verifyCiphertext("0545aad56da2a97c3663d1432a3d1c84", "test 4");
    
    
    
    uint32_t pt5[4]  = {0, 0, 0, 1};
    uint32_t key5[4] = {0, 0, 0, 0};
    setPlaintext(pt5);
    setKey(key5);
    start();
    waitForValidOutput();
    reportCiphertext();
    verifyCiphertext("58e2fccefa7e3061367f1d57a4e7455a", "test 5");
    
    return 0;
}
