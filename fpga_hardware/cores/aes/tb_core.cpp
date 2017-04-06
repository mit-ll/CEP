#include "verilated.h"
#include "Vaes_128.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include "AES.h"

// Current simulation time
vluint64_t main_time = 0;

// Called by $time in Verilog
// converts to double, to match
// what SystemC does
double sc_time_stamp () {
    return main_time;
}

const unsigned int CLOCK_PERIOD = 10;
Vaes_128* top;

void runForClockCycles(const unsigned int pCycles) {
    int doubleCycles = 0;
    while(doubleCycles < (pCycles << 1)) {
        top->eval();
        if((main_time % (CLOCK_PERIOD >> 1)) == 0) {
            ++doubleCycles;
            top->clk = ~top->clk;
        }
        main_time++;
    }
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
    cout << "Ciphertext:\t0x";
    for(int i = (128 / 32) - 1; i >= 0; --i) {
        printWordAsBytes(top->out[i]);
    }
    cout << endl;
}

void saveCiphertext(uint32_t *pCT) {
    for(int i = 0; i < (128 / 32); ++i) {
        *pCT++ = top->out[i];
    }
}

void setPlaintext(const char* pPT) {
    cout << "Plaintext:\t0x";
    for(int i = 0; i < (128 / 8); ++i) {
        for(int j = 0; j < 4; ++j) {
            ((char *)(&(top->state[(128 / 32) - 1 - i])))[3 - j] = *pPT++;
        }
        cout << hex << setfill('0') << setw(8) << top->state[3-i];
    }
    cout << endl;
}

void setPlaintext(const uint32_t* pPT) {
    cout << "Plaintext:\t0x";
    for(int i = 0; i < (128 / 32); ++i) {
        top->state[3-i] = pPT[i];
        cout << hex << setfill('0') << setw(8) << pPT[i];
    }
    cout << endl;
}

void setKey(const uint32_t* pKey) {
    cout << "Key:\t\t0x";
    for(int i = 0; i < (128 / 32); ++i) {
        top->key[3-i] = pKey[i];
        cout << hex << setfill('0') << setw(8) << pKey[i];
    }
    cout << endl;
}

int main(int argc, char **argv, char **env) {
    Verilated::commandArgs(argc, argv);
    top = new Vaes_128;
    
    uint32_t ct[128 / 32];
    
    cout << "Initializing interface and resetting core" << endl;
    
    // Initialize Inputs
    top->clk = 0;
    top->start = 0;
    runForClockCycles(100);
    
    cout << "Reset complete" << endl;
    
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
    
    top->final();
    delete top;
    exit(0);
}
