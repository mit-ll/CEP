#include "verilated.h"
#include "Vpancham.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include "MD5.h"

Vpancham* top;

void evalModel() {
    top->eval();
}

void toggleClock() {
    top->clk = ~top->clk;
}

void updateHash(unsigned char *pHash) {
    uint32_t* hPtr = (uint32_t *)pHash;
    
    for(int i = HASH_WORDS - 1; i >= 0; --i) {
        *hPtr++ = top->msg_output[i];
    }
}

void reportAppended() {
    cout << "Padded input:" << endl;
    for(int i = MESSAGE_WORDS - 1; i >= 0; --i) {
        cout << "0x";
        printf("%08X", top->msg_padded[i]);
        cout << endl;
    }
}

void waitForReady() {
    while(top->ready == 0) {
        runForClockCycles(10);
    }
}

void waitForValidOut() {
    while(top->msg_out_valid == 0) {
        runForClockCycles(1);
    }
}

void resetAndReady() {
    top->rst = 1;
    runForClockCycles(10);
    top->rst = 0;
    waitForReady();
}

void strobeMsgValid() {
    top->msg_in_valid = 1;
    runForClockCycles(10);
    top->msg_in_valid = 0;
}

void loadPaddedMessage(const char* msg_ptr) {
    for(int i = 0; i < MESSAGE_WORDS; ++i) {
        for(int j = 0; j < BYTES_PER_WORD; ++j) {
            ((char *)(&top->msg_padded[i]))[j] = *msg_ptr++;
        }
    }
}

int main(int argc, char **argv, char **env) {
    Verilated::commandArgs(argc, argv);
    
    top = new Vpancham;
    char message[MESSAGE_BITS * 2];
    unsigned char hash[HASH_BYTES];
    
    // Test reset behavior
    top->clk = 0;
    top->rst = 1;
    for(int i = 0; i < MESSAGE_WORDS; ++i) {
        top->msg_padded[i] = 0;
    }
    top->msg_in_valid = 0;
    runForClockCycles(100);
    if(top->msg_out_valid == 0) {
        cout << "PASSED: expected reset behavior" << endl;
    } else {
        cout << "FAILED: unexpected reset behavior" << endl;
    }
    reportHash();
    
    // Save the hash value to check for stabilization
    vluint32_t savedHash[HASH_WORDS];
    for(int i = 0; i < HASH_WORDS; ++i) {
        savedHash[i] = top->msg_output[i];
    }
    
    top->msg_in_valid = 1;
    runForClockCycles(100);
    if(top->msg_out_valid == 0) {
        cout << "PASSED: expected reset behavior" << endl;
    } else {
        cout << "FAILED: unexpected reset behavior" << endl;
    }
    
    // Check against saved hash
    for(int i = 0; i < HASH_WORDS; ++i) {
        if(savedHash[i] != top->msg_output[i]) {
            cout << "FAILED: output not stabilized" << endl;
            break;
        }
    }
    reportHash();
    
    // Test after reset, but before valid input
    top->rst = 0;
    waitForReady();
    top->msg_in_valid = 0;
    runForClockCycles(100);
    if(top->msg_out_valid == 0) {
        cout << "PASSED: expected reset behavior" << endl;
    } else {
        cout << "FAILED: unexpected reset behavior" << endl;
    }
    
    // Check against saved hash
    for(int i = 0; i < HASH_WORDS; ++i) {
        if(savedHash[i] != top->msg_output[i]) {
            cout << "FAILED: output not stabilized" << endl;
            break;
        }
    }
    reportHash();
    
    // Test common case behavior
    waitForReady();
    addPadding(0, message);
    loadPaddedMessage(message);
    
    strobeMsgValid();
    waitForValidOut();
    
    // Save the hash value to check for stabilization
    for(int i = 0; i < HASH_WORDS; ++i) {
        savedHash[i] = top->msg_output[i];
    }
    
    // Verify hash
    if(savedHash[3] != 0xd41d8cd9 || savedHash[2] != 0x8f00b204 || savedHash[1] != 0xe9800998 || savedHash[0] != 0xecf8427e) {
        cout << "FAILED: empty string hash" << endl;
    }
    else {
        cout << "PASSED: empty string hash" << endl;
    }
    
    waitForReady();
    
    // Check against saved hash
    for(int i = 0; i < HASH_WORDS; ++i) {
        if(savedHash[i] != top->msg_output[i]) {
            cout << "FAILED: hash not maintained after ready" << endl;
            break;
        }
        if(i == 3)
            cout << "PASSED: hash maintained after ready" << endl;
    }
    
    reportHash();
    
    strobeMsgValid();
    waitForValidOut();
    
    // Check against saved hash
    for(int i = 0; i < HASH_WORDS; ++i) {
        if(savedHash[i] != top->msg_output[i]) {
            cout << "PASSED: hash not consistent across consecutive hashes" << endl;
            break;
        }
        if(i == 3)
            cout << "FAILED: hash consistent across consecutive hashes" << endl;
    }
    
    reportHash();
    
    // Test reset again: Reset to clear constants
    resetAndReady();
    strobeMsgValid();
    waitForValidOut();
    
    // Verify hash
    if(top->msg_output[3] != 0xd41d8cd9 || top->msg_output[2] != 0x8f00b204 || top->msg_output[1] != 0xe9800998 || top->msg_output[0] != 0xecf8427e) {
        cout << "FAILED: hash is not consistent across resets" << endl;
    }
    else {
        cout << "PASSED: hash is consistent across resets" << endl;
    }
    
    waitForReady();
    reportHash();
    
    // Test common case 2
    hashString("a", hash);
    compareHash(hash, "0cc175b9c0f1b6a831c399e269772661", "single character");
    
    // Test common case 3
    hashString("abc", hash);
    compareHash(hash, "900150983cd24fb0d6963f7d28e17f72", "three character");
    
    // Test a message that requires an extra chunk to be added to hold padding
    hashString("Lincoln LaboratoLincoln LaboratoLincoln LaboratoLincoln Laborato", hash);
    compareHash(hash, "93f3763d2c20d33e5031e20480eaca58", "512-bit sized message");
    
    // Hash a test file
    ifstream inFile("input.txt", ios::in|ios::binary|ios::ate);
    if(inFile.is_open()) {
        streampos size = inFile.tellg();
        
        inFile.seekg(0, ios::beg);
        
        int messageBits = (size - inFile.tellg()) * BITS_PER_BYTE;
        printf("Message length: %d bits\n", messageBits);
        
        char * block = new char[messageBits / BITS_PER_BYTE];
        memset(block, 0, messageBits/BITS_PER_BYTE);
        
        inFile.read(block, size);
        inFile.close();
        
        hashString(block, hash);
        compareHash(hash, "a3af880db6067a6c7592fb01e877e767", "text file");
    } else {
        cout << "ERROR: unable to open input.txt" << endl;
    }
    
    top->final();
    delete top;
    exit(0);
}
