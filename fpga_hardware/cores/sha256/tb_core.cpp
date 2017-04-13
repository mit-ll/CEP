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

void strobeNext(void) {
    top->next = 1;
    runForClockCycles(1);
    top->next = 0;
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
    
    // Test common case 1
    hashString("", hash);
    compareHash(hash, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855", "empty string");
    
    // Test common case 2
    hashString("a", hash);
    compareHash(hash, "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb", "single character");
    
    // Test common case 3
    hashString("abc", hash);
    compareHash(hash, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "three character");
    
    // Test a message that requires an extra chunk to be added to hold padding
    hashString("Lincoln LaboratoLincoln LaboratoLincoln LaboratoLincoln Laborato", hash);
    compareHash(hash, "f88c49e2b696d45a699eb10effafb3c9522df6f7fa68c2509d105e849be605ba", "512-bit message");
    
    // Hash a test file
    ifstream inFile("input.txt", ios::in|ios::binary|ios::ate);
    if(inFile.is_open()) {
        streampos size = inFile.tellg();
        
        inFile.seekg(0, ios::beg);
        
        int messageBits = (size - inFile.tellg()) * 8;
        printf("Message length: %d bits\n", messageBits);
        
        char * block = new char[messageBits / 8];
        memset(block, 0, messageBits/8);
        
        inFile.read(block, size);
        inFile.close();
        
        hashString(block, hash);
        compareHash(hash, "0e053a84aae06a58b677cbda6c2bed32046a56e311faae4393fa2c8201daa2f7", "text file");
    } else {
        cout << "ERROR: unable to open input.txt" << endl;
    }

    top->final();
    delete top;
    exit(0);
}
