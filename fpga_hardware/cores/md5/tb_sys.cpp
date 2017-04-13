#include <iostream>
#include <iomanip>
using namespace std;

#include "MD5.h"
#include "input.h"

uint32_t readFromAddress(uint32_t pAddress) {
  return *((uint32_t *)pAddress);
}

void writeToAddress(uint32_t pAddress, uint32_t pData) {
  *((uint32_t *)pAddress) = pData;
}

void updateHash(char *pHash) {
    uint32_t* hPtr = (uint32_t *)pHash;
    
    for(int i = (128 / 32) - 1; i >= 0; --i) {
        *hPtr++ = readFromAddress(MD5_HASH_BASE + i);
    }
}

void reportAppended() {
    cout << "Padded input:" << endl;
    for(int i = (512 / 32) - 1; i >= 0; --i) {
        cout << "0x";
	uint32_t dataPart = readFromAddress(MD5_MSG_BASE + i);
        printf("%08X", dataPart);
        cout << endl;
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
  for(int i = 0; i < ((512 / 8) / 4); ++i) {
    uint32_t temp = 0;
    for(int j = 0; j < 4; ++j) {
      ((char *)(&temp))[j] = *msg_ptr++;
    }
    writeToAddress(MD5_MSG_BASE + i, temp);
  }
}

int main(int argc, char **argv, char **env) {
  char hash[128 /8];

  cout << "Resetting the MD5 module..." << endl;
    
  // Test reset behavior
  resetAndReady();

  cout << "Reset complete" << endl;

  resetAndReady();
  hashString("a", hash);
  compareHash(hash, "0cc175b9c0f1b6a831c399e269772661", "single character");
    
  resetAndReady();
  hashString("abc", hash);
  compareHash(hash, "900150983cd24fb0d6963f7d28e17f72", "three character");
    
  // Test a message that requires an extra chunk to be added to hold padding
  resetAndReady();
  hashString("Lincoln LaboratoLincoln LaboratoLincoln LaboratoLincoln Laborato", hash);
  compareHash(hash, "93f3763d2c20d33e5031e20480eaca58", "512-bit sized message");

  // Hash a test file
  resetAndReady();
  hashString(fileInput, hash);
  compareHash(hash, "7E2190CE5041B22EE3E1F82716136750", "text file");

  exit(0);
}
