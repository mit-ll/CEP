const unsigned int HASH_BITS = 256;
const unsigned int MESSAGE_BITS = 512;
const unsigned int CLOCK_PERIOD = 10;

// Base address of the core on the bus
const uint32_t SHA256_BASE = 0xF0000000;

// Offset of SHA256 data and control registers in device memory map
const uint32_t SHA256_READY = SHA256_BASE + 0;
const uint32_t SHA256_MSG_BASE = SHA256_BASE + 1;
const uint32_t SHA256_HASH_DONE = SHA256_BASE + 17;
const uint32_t SHA256_HASH_BASE = SHA256_BASE + 18;
const uint32_t SHA256_NEXT_INIT = SHA256_BASE + 0;

// Level-dependent functions
void reset(void);
void waitForReady(void);
void updateHash(char *pHash);
void waitForValidOutput(void);
void strobeInit(void);
void strobeNext(void);
void loadPaddedMessage(const char* msg_ptr);
void reportAppended(void);

void resetAndReady(void) {
    reset();
    waitForReady();
}

void reportHash() {
    printf("Hash: 0x");
    char hash[HASH_BITS / 8];
    updateHash(hash);
    for(int i = (HASH_BITS / 8) - 1; i >= 0; --i) {
        printf("%02X", (hash[i] & 0xFF));
    }
    printf("\n");
}

int addPadding(vluint64_t pMessageBits64Bit, char* buffer) {
    int extraBits = pMessageBits64Bit % 512;
    int paddingBits = extraBits > 448 ? 1024 - extraBits : 512 - extraBits;
    
    // Add size to end of string
    const int startByte = extraBits / 8;
    const int sizeStartByte =  startByte + ((paddingBits / 8) - 8);
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
    
    return (paddingBits / 8);
}

bool compareHash(const char * pProposedHash, const char* pExpectedHash, const char * pTestString) {
    char longTemp[(HASH_BITS / 4) + 1];
    
    char *temp = longTemp;
    for(int i = ((HASH_BITS / 32) - 1); i >= 0; --i) {
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
    
    cout << "Hashing: " << pString << endl;
    
    // Reset for each message
    resetAndReady();
    while(!done) {
        char message[1024];
        memset(message, 0, sizeof(message));
        
        // Copy next portion of string to message buffer
        char *msg_ptr = message;
        int length = 0;
        while(length < (MESSAGE_BITS / 8)) {
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
            addedBytes = addPadding(totalBytes * 8, message);
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
            reportAppended();
            reportHash();
            updateHash(pHash);
            addedBytes -= (MESSAGE_BITS / 8);
            msg_ptr += (MESSAGE_BITS / 8);
        } while(addedBytes > 0);
    }
}
