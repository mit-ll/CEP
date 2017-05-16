#include "verilated.h"
#include "Vsha256_top.h"
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

Vsha256_top* top;

void runForClockCycles(const unsigned int pCycles) {
    int doubleCycles = 0;
    while(doubleCycles < (pCycles << 1)) {
        top->eval();
        if((main_time % (CLOCK_PERIOD >> 1)) == 0) {
            ++doubleCycles;
            top->wb_clk_i = ~top->wb_clk_i;
        }
        main_time++;
    }
}

void reset(void) {
    top->wb_rst_i = 1;
    runForClockCycles(10);
    top->wb_rst_i = 0;
}

void waitForACK() {
    while(top->wb_ack_o == 0) {
        runForClockCycles(1);
    }
}

uint32_t readFromAddress(uint32_t pAddress) {
    top->wb_adr_i = pAddress;
    top->wb_dat_i = 0x00000000;
    top->wb_we_i = 0;
    top->wb_stb_i = 1;
    runForClockCycles(10);
    waitForACK();
    uint32_t data = top->wb_dat_o;
    top->wb_stb_i = 0;
    runForClockCycles(10);
    
    return data;
}

void writeToAddress(uint32_t pAddress, uint32_t pData) {
    top->wb_adr_i = pAddress;
    top->wb_dat_i = pData;
    top->wb_we_i = 1;
    top->wb_stb_i = 1;
    runForClockCycles(10);
    waitForACK();
    top->wb_stb_i = 0;
    top->wb_we_i = 0;
    runForClockCycles(10);
}

void waitForReady(void) {
    while (readFromAddress(SHA256_READY) == 0) {
        ;
    }
}

void waitForValidOutput(void) {
    while (readFromAddress(SHA256_HASH_DONE) == 0) {
        ;
    }
}

void reportAppended(void) {
    cout << "Padded input:" << endl;
    for(int i = (MESSAGE_BITS / 32) - 1; i >= 0; --i) {
        cout << "0x";
        printf("%08X", readFromAddress(SHA256_MSG_BASE + (i * 4)));
        cout << endl;
    }
}

void updateHash(char *pHash) {
    uint32_t * temp = (uint32_t *)pHash;
    
    for(int i = 0; i < (HASH_BITS / 32); ++i) {
        *temp++ = readFromAddress(SHA256_HASH_BASE + (i * 4));
    }
}

void strobeInit(void) {
    writeToAddress(SHA256_NEXT_INIT, 0x1);
    writeToAddress(SHA256_NEXT_INIT, 0x0);
}

void strobeNext(void) {
    writeToAddress(SHA256_NEXT_INIT, 0x2);
    writeToAddress(SHA256_NEXT_INIT, 0x0);
}

void loadPaddedMessage(const char* msg_ptr) {
    int temp = 0;
    for(int i = ((MESSAGE_BITS / 8) - 1); i >= 0; --i) {
        temp = (temp << 8) | ((*msg_ptr++) & 0xFF);
        
        if(i % 4 == 0) {
            writeToAddress(SHA256_MSG_BASE + i, temp);
        }
    }
}

int main(int argc, char **argv, char **env) {
    Verilated::commandArgs(argc, argv);
    
    top = new Vsha256_top;
    char hash[HASH_BITS / 8];
    
    cout << "Initializing interface and resetting core" << endl;
    
    // Initialize Inputs
    top->wb_clk_i = 0;
    top->wb_dat_i = 0;
    top->wb_we_i = 0;
    top->wb_adr_i = 0;
    top->wb_stb_i = 0;
    runForClockCycles(100);
    
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
