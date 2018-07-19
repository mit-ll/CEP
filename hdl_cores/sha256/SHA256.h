#include <stdio.h>
#include <string.h>

const unsigned int BITS_PER_BYTE = 8;
const unsigned int BYTES_PER_WORD = 4;
const unsigned int HASH_BITS = 256;
const unsigned int MESSAGE_BITS = 512;
const unsigned int CLOCK_PERIOD = 10;
const unsigned int SHA256_READY_BYTES = BYTES_PER_WORD;
const unsigned int SHA256_HASH_DONE_BYTES = BYTES_PER_WORD;

// Automatically generated constants
const unsigned int MESSAGE_BYTES = MESSAGE_BITS / BITS_PER_BYTE;
const unsigned int MESSAGE_WORDS = MESSAGE_BYTES / BYTES_PER_WORD;
const unsigned int HASH_BYTES = HASH_BITS / BITS_PER_BYTE;
const unsigned int HASH_WORDS = HASH_BYTES / BYTES_PER_WORD;

typedef long unsigned int uint32_t;
typedef long unsigned long uint64_t;
typedef long unsigned long vluint64_t;

// Base address of the core on the bus
const uint32_t SHA256_BASE = 0x95000000;

// Offset of SHA256 data and control registers in device memory map
const uint32_t SHA256_READY = SHA256_BASE;
const uint32_t SHA256_MSG_BASE = SHA256_READY + SHA256_READY_BYTES;
const uint32_t SHA256_HASH_DONE = SHA256_MSG_BASE + MESSAGE_BYTES;
const uint32_t SHA256_HASH_BASE = SHA256_HASH_DONE + SHA256_HASH_DONE_BYTES;
const uint32_t SHA256_NEXT_INIT = SHA256_BASE;

// Current simulation time
vluint64_t main_time = 0;

// Called by $time in Verilog
// converts to double, to match
// what SystemC does
double sc_time_stamp () {
    return main_time;
}

// Level-dependent functions
void evalModel();
void toggleClock(void);
void reset(void);
void waitForReady(void);
void updateHash(char *pHash);
void waitForValidOutput(void);
void strobeInit(void);
void strobeNext(void);
void loadPaddedMessage(const char* msg_ptr);
void reportAppended(void);

void runForClockCycles(const unsigned int pCycles) {
    int doubleCycles = pCycles << 2;
    while(doubleCycles > 0) {
        evalModel();
        toggleClock();
        main_time += (CLOCK_PERIOD >> 2);
        --doubleCycles;
    }
}

void resetAndReady(void) {
    waitForReady();
}

void reportHash() {
    printf("Hash: 0x");
    char hash[HASH_BYTES];
    updateHash(hash);
    for(int i = (HASH_BYTES) - 1; i >= 0; --i) {
        printf("%02X", (hash[i] & 0xFF));
    }
    printf("\n");
}

int addPadding(uint64_t pMessageBits64Bit, char* buffer) {
    int extraBits = pMessageBits64Bit % MESSAGE_BITS;
    int paddingBits = extraBits > 448 ? (2 * MESSAGE_BITS) - extraBits : MESSAGE_BITS - extraBits;
    
    // Add size to end of string
    const int startByte = extraBits / BITS_PER_BYTE;
    const int sizeStartByte =  startByte + ((paddingBits / BITS_PER_BYTE) - 8);
    for(int i = startByte; i < (sizeStartByte + 8); ++i) {
        if(i == startByte) {
            buffer[i] = 0x80; // 1 followed by many 0's
        } else if( i >= sizeStartByte) {
            int offset = i - sizeStartByte;
            int shftAmnt = 56 - (8 * offset);
            buffer[i] = (pMessageBits64Bit >> shftAmnt) & 0xFF;
        } else {
            buffer[i] = 0x0;
        }
    }
    
    return (paddingBits / BITS_PER_BYTE);
}

bool compareHash(const char * pProposedHash, const char* pExpectedHash, const char * pTestString) {
    char longTemp[(HASH_BITS / 4) + 1];
    
    char *temp = longTemp;
    for(int i = (HASH_WORDS - 1); i >= 0; --i) {
        sprintf(temp, "%8.8x", ((uint32_t *)pProposedHash)[i]);
        temp += 8;
    }
    
    if(strncmp(longTemp, pExpectedHash, HASH_BITS / 4) == 0) {
        printf("PASSED: %s hash test\n", pTestString);
        return true;
    }
    
    printf("FAILED: %s hash test with hash: %s\n", pTestString, temp);
    printf("EXPECTED: %s\n", pExpectedHash);
    
    return false;
}

void hashString(const char *pString, char *pHash) {
    bool done = false;
    bool firstTime = true;
    int totalBytes = 0;
    
    printf("Hashing: %s\n", pString);
    
    // Reset for each message
    resetAndReady();
    while(!done) {
        char message[2 * MESSAGE_BITS];
        memset(message, 0, sizeof(message));
        
        // Copy next portion of string to message buffer
        char *msg_ptr = message;
        int length = 0;
        while(length < MESSAGE_BYTES) {
            // Check for end of input
            if(*pString == '\0') {
                done = true;
                break;
            }
            *msg_ptr++ = *pString++;
            ++length;
            ++totalBytes;
        }
        
        // Need to add padding if done
        int addedBytes = 0;
        if(done) {
            addedBytes = addPadding(totalBytes * BITS_PER_BYTE, message);
        }
        
        // Send the message
        msg_ptr = message;
        do {
            waitForReady();
            loadPaddedMessage(msg_ptr);
            if(firstTime) {
                strobeInit();
                firstTime = false;
            } else {
                strobeNext();
            }
            waitForValidOutput();
            waitForReady();
            //reportAppended();
            reportHash();
            updateHash(pHash);
            addedBytes -= MESSAGE_BYTES;
            msg_ptr += MESSAGE_BYTES;
        } while(addedBytes > 0);
    }
}
