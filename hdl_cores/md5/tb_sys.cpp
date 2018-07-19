#include "MD5.h"
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

void updateHash(unsigned char *pHash) {
    uint32_t* hPtr = (uint32_t *)pHash;
    
    for(int i = HASH_WORDS - 1; i >= 0; --i) {
        *hPtr++ = readFromAddress(MD5_HASH_BASE + (i * BYTES_PER_WORD));
    }
}

void reportAppended() {
    printf("Padded input:\n");
    for(int i = MESSAGE_WORDS - 1; i >= 0; --i) {
	uint32_t dataPart = readFromAddress(MD5_MSG_BASE + (i * BYTES_PER_WORD));
        printf("0x%08X\n", (unsigned int)dataPart);
    }
}

void waitForReady() {
  while(readFromAddress(MD5_READY) == 0) {
    ;
  }
}

void waitForValidOut() {
  waitForReady();
}

void resetAndReady() {
  writeToAddress(MD5_RST, 0x1);
  writeToAddress(MD5_RST, 0x0);
  waitForReady();
}

void strobeMsgValid() {
  writeToAddress(MD5_READY, 0x1);
  writeToAddress(MD5_READY, 0x0);
}

void loadPaddedMessage(const char* msg_ptr) {
  for(unsigned int i = 0; i < MESSAGE_WORDS; ++i) {
    uint32_t temp = 0;
    for(int j = BYTES_PER_WORD - 1; j >= 0; --j) {
      //printf("%02X ", (unsigned int)(*msg_ptr & 0xFF));
      ((char *)(&temp))[j] = *msg_ptr++;
    }
    //printf("\n 0x%08X written to: 0x%08X\n", temp, MD5_MSG_BASE + (i * 4));
    writeToAddress(MD5_MSG_BASE + (i * BYTES_PER_WORD), temp);
  }
}

int main(int argc, char **argv, char **env) {
  unsigned char hash[HASH_BYTES];

  printf("Resetting the MD5 module...\n");
  resetAndReady();
  printf("Reset complete\n");
  
  hashString("a", hash);
  compareHash(hash, "0cc175b9c0f1b6a831c399e269772661", "single character");
    
  hashString("abc", hash);
  compareHash(hash, "900150983cd24fb0d6963f7d28e17f72", "three character");
    
  // Test a message that requires an extra chunk to be added to hold padding
  hashString("Lincoln LaboratoLincoln LaboratoLincoln LaboratoLincoln Laborato", hash);
  compareHash(hash, "93f3763d2c20d33e5031e20480eaca58", "512-bit sized message");

  // Hash a test file
  hashString(fileInput, hash);
  compareHash(hash, "7e2190ce5041b22ee3e1f82716136750", "text file");

  return 0;
}
