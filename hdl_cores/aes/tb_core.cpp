#include "verilated.h"
#include "Vaes_192.h"
#include "AES.h"

Vaes_192* top;

void evalModel() {
    top->eval();
}

void toggleClock() {
    top->clk = ~top->clk;
}

bool ciphertextValid(void) {
    return (top->out_valid != 0) ? true : false;
}

void start(void) {
    top->start = 1;
    runForClockCycles(5);
    top->start = 0;
}

void reportCiphertext(void) {
    printf("Ciphertext:\t0x");
    for(int i = BLOCK_WORDS - 1; i >= 0; --i) {
        printf("%08X", top->out[i]);
    }
    printf("\n");
}

void saveCiphertext(uint32_t *pCT) {
    for(int i = 0; i < BLOCK_WORDS; ++i) {
        *pCT++ = top->out[i];
    }
}

void setPlaintext(const char* pPT) {
    printf("Plaintext:\t0x");
    for(int i = 0; i < BLOCK_BYTES; ++i) {
        for(int j = 0; j < 4; ++j) {
            ((char *)(&(top->state[BLOCK_WORDS - 1 - i])))[3 - j] = *pPT++;
        }
        printf("%08X", top->state[3-i]);
    }
    printf("\n");
}

void setPlaintext(const uint32_t* pPT) {
    printf("Plaintext:\t0x");
    for(int i = 0; i < BLOCK_WORDS; ++i) {
        top->state[(BLOCK_WORDS - 1) - i] = pPT[i];
        printf("%08X", pPT[i]);
    }
    printf("\n");
}

void setKey(const uint32_t* pKey) {
    printf("Key:\t\t0x");
    for(int i = 0; i < KEY_WORDS; ++i) {
        top->key[(KEY_WORDS - 1) - i] = pKey[i];
        printf("%08X", pKey[i]);
    }
    printf("\n");
}

int main(int argc, char **argv, char **env) {
    Verilated::commandArgs(argc, argv);
    top = new Vaes_192;
    
    uint32_t ct[BLOCK_WORDS];
    
    printf("Initializing interface and resetting core\n");
    
    // Initialize Inputs
    top->clk = 0;
    top->start = 0;
    runForClockCycles(100);
    
    printf("Reset complete\n");
    
    uint32_t pt1[4]  = {0x3243f6a8, 0x885a308d, 0x313198a2, 0xe0370734};
    uint32_t key1[6] = {0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c, 0x2b7e1516, 0x28aed2a6};
    setPlaintext(pt1);
    setKey(key1);
    start();
    waitForValidOutput();
    reportCiphertext();
    verifyCiphertext("4fcb8db85784a2c1bb77db7ede3217ac", "test 1");
    
    
    
    uint32_t pt2[4]  = {0x00112233, 0x44556677, 0x8899aabb, 0xccddeeff};
    uint32_t key2[6] = {0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f, 0x00010203, 0x04050607};
    setPlaintext(pt2);
    setKey(key2);
    start();
    waitForValidOutput();
    reportCiphertext();
    verifyCiphertext("65d50128b115a7780981475a6bd64a0e", "test 2");
    
    
    
    uint32_t pt3[4]  = {0, 0, 0, 0};
    uint32_t key3[6] = {0, 0, 0, 0, 0, 0};
    setPlaintext(pt3);
    setKey(key3);
    start();
    waitForValidOutput();
    reportCiphertext();
    verifyCiphertext("aae06992acbf52a3e8f4a96ec9300bd7", "test 3");
    
    
    
    uint32_t pt4[4]  = {0, 0, 0, 0};
    uint32_t key4[6] = {0, 0, 0, 0, 0, 1};
    setPlaintext(pt4);
    setKey(key4);
    start();
    waitForValidOutput();
    reportCiphertext();
    verifyCiphertext("8bae4efb70d33a9792eea9be70889d72", "test 4");
    
    
    
    uint32_t pt5[4]  = {0, 0, 0, 1};
    uint32_t key5[6] = {0, 0, 0, 0, 0, 0};
    setPlaintext(pt5);
    setKey(key5);
    start();
    waitForValidOutput();
    reportCiphertext();
    verifyCiphertext("cd33b28ac773f74ba00ed1f312572435", "test 5");
    
    top->final();
    delete top;
    exit(0);
}
