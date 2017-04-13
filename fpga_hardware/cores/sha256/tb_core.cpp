#include "verilated.h"
#include "Vsha256.h"
#include <iostream>
#include <fstream>
#include <iomanip>

// Current simulation time
vluint64_t main_time = 0;

// Called by $time in Verilog
// converts to double, to match
// what SystemC does
double sc_time_stamp () {
    return main_time;
}

const unsigned int CLOCK_PERIOD = 10;
const unsigned int HASH_BITS = 256;
const unsigned int MESSAGE_BITS = 512;
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

void printWordAsBytes(uint32_t pWord) {
    cout << hex << uppercase << setfill('0') << setw(2) << ((pWord & 0xFF000000) >> 24);
    cout << hex << uppercase << setfill('0') << setw(2) << ((pWord & 0xFF0000) >> 16);
    cout << hex << uppercase << setfill('0') << setw(2) << ((pWord & 0xFF00) >> 8);
    cout << hex << uppercase << setfill('0') << setw(2) << (pWord & 0xFF);
}

void reportHash() {
    cout << "Hash: 0x";
    for(int i = (HASH_BITS / 32) - 1; i >= 0; --i) {
        printWordAsBytes(top->digest[i]);
    }
    cout << endl;
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

#ifdef tehcuu
void hashString(const char *pString, char *pHash) {
    bool done = false;
    int totalBytes = 0;
    const char* current = pString;
    
    cout << "Hashing: " << pString << endl;
    
    // Reset for each message
    resetAndReady();
    
    while(!done) {
        waitForReady();
        
        // Break into 4 byte chunks
        int numBytes;
        for(numBytes = 0; numBytes < 4; ++numBytes) {
            if(*current == '\0') {
                done = true;
                break;
            }
            ((char *)(&top->in))[3 - numBytes] = *current++;
        }
        
        // Send the word and length to the module
        top->byte_num = numBytes;
        top->is_last = done ? 1 : 0;
        top->in_ready = 1;
        
        printf("Data: 0x%8.8X\n", top->in);
        printf("%d bytes\n", top->byte_num);
        
        runForClockCycles(10);
        top->in_ready = 0;
        runForClockCycles(10);
    }
    
    waitForValidOutput();
    reportHash();
}
#endif

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
    uint32_t message[16] = {0x61626380, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x18};
    for(int i = 0; i < (MESSAGE_BITS / 32); ++i) {
        top->block[i] = message[(MESSAGE_BITS / 32) - 1 - i];
    }
    strobeInit();
    waitForValidOutput();
    reportHash();
    
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
