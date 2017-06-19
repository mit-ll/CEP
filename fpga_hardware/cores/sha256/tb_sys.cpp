#include <cstdio>
#include <cstring>

#include "SHA256.h"
#include "input.h"

// Need this to fully implement expected interface
void toggleClock() {;}
void evalModel() {;}

// Need volatile to make sure the compiler doesn't make bus writes/read disappear
uint32_t readFromAddress(uint32_t pAddress) {
  return *((volatile uint32_t *)pAddress);
}

// Need volatile to make sure the compiler doesn't make bus writes/read disappear
void writeToAddress(uint32_t pAddress, uint32_t pData) {
  *((volatile uint32_t *)pAddress) = pData;
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
    printf("Padded input:\n");
    for(int i = MESSAGE_WORDS - 1; i >= 0; --i) {
        printf("0x%08X\n", readFromAddress(SHA256_MSG_BASE + (i * BYTES_PER_WORD)));
    }
}

void updateHash(char *pHash) {
    uint32_t * temp = (uint32_t *)pHash;
    
    for(unsigned int i = 0; i < HASH_WORDS; ++i) {
        *temp++ = readFromAddress(SHA256_HASH_BASE + (i * BYTES_PER_WORD));
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
    for(int i = (MESSAGE_BYTES - 1); i >= 0; --i) {
        temp = (temp << 8) | ((*msg_ptr++) & 0xFF);
        
        if(i % BYTES_PER_WORD == 0) {
            writeToAddress(SHA256_MSG_BASE + i, temp);
        }
    }
}

int main(int argc, char **argv, char **env) {
    char hash[HASH_BYTES];
    
    printf("Waiting for ready signal...\n");
    
    waitForReady();
    
    printf("Starting...\n");
    
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
    hashString(fileInput, hash);
    compareHash(hash, "5c1a28d16a0fda902337c9ab6630da4f8bf8ebec40e21d4089912c338d37011a", "text file");

    return 0;
}
