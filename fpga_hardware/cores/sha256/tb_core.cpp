#include "verilated.h"
#include "Vsha256.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include "SHA256.h"

// Current simulation time
vluint64_t main_time = 0;

// Called by $time in Verilog
// converts to double, to match
// what SystemC does
double sc_time_stamp () {
    return main_time;
}

Vsha256* top;

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

void reset(void) {
    top->rst = 1;
    runForClockCycles(10);
    top->rst = 0;
}

void waitForReady(void) {
    while (top->ready == 0) {
        runForClockCycles(1);
    }
}

void waitForValidOutput(void) {
    while (top->digest_valid == 0) {
        runForClockCycles(1);
    }
}

void reportAppended(void) {
    cout << "Padded input:" << endl;
    for(int i = (MESSAGE_BITS / 32) - 1; i >= 0; --i) {
        cout << "0x";
        printf("%08X", top->block[i]);
        cout << endl;
    }
}

void updateHash(char *pHash) {
    uint32_t * temp = (uint32_t *)pHash;
    
    for(int i = 0; i < (HASH_BITS / 32); ++i) {
        *temp++ = top->digest[i];
    }
}

void strobeInit(void) {
    top->init = 1;
    runForClockCycles(1);
    top->init = 0;
}

void loadPaddedMessage(const char* msg_ptr) {
    int temp = 0;
    for(int i = ((MESSAGE_BITS / 8) - 1); i >= 0; --i) {
        temp = (temp << 8) | ((*msg_ptr++) & 0xFF);
        
        if(i % 4 == 0) {
            top->block[i / 4] = temp;
        }
    }
}

int main(int argc, char **argv, char **env) {
    Verilated::commandArgs(argc, argv);
    
    top = new Vsha256;
    char hash[HASH_BITS / 8];
    
    cout << "Initializing interface and resetting core" << endl;
    
    // Initialize Inputs
    top->clk = 0;
    top->rst = 0;
    top->init = 0;
    top->next = 0;
    for(int i = 0; i < (HASH_BITS / 32); ++i) {
        top->block[i] = 0;
    }
    runForClockCycles(10);
    
    reset();
    
    cout << "Reset complete" << endl;
    
    cout << "Waiting for ready signal..." << endl;
    
    waitForReady();
    
    cout << "Starting..." << endl;
    
    // abc
    hashString("abc", hash);
    compareHash(hash, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "abc");
    
    
#ifdef theouthe
    
    //compareHash("d135bb84d0439dbac432247ee573a23ea7d3c9deb2a968eb31d47c4fb45f1ef4422d6c531b5b9bd6f449ebcc449ea94d0a8f05f62130fda612da53c79659f609", hash, "short string");
    hashString("LL", hash);
    
    hashString("The quick brown fox jumps over the lazy dog", hash);
    
    hashString("The quick brown fox jumps over the lazy dog.", hash);
    compareHash("ab7192d2b11f51c7dd744e7b3441febf397ca07bf812cceae122ca4ded6387889064f8db9230f173f6d1ab6e24b6e50f065b039f799f5592360a6558eb52d760", hash, "short string plus one char");
    
    hashString("\xA1\xA2\xA3\xA4\xA5", hash);
    compareHash("12f4a85b68b091e8836219e79dfff7eb9594a42f5566515423b2aa4c67c454de83a62989e44b5303022bfe8c1a9976781b747a596cdab0458e20d8750df6ddfb", hash, "5 characeter");
    
    hashString("", hash);
    compareHash("0eab42de4c3ceb9235fc91acffe746b29c29a8c366b7c60e4e67c466f36a4304c00fa9caf9d87976ba469bcbe06713b435f091ef2769fb160cdab33d3670680e", hash, "empty string");
#endif
    
    top->final();
    delete top;
    exit(0);
}
