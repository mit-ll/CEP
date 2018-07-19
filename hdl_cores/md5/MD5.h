#include <stdio.h>
#include <string.h>

const unsigned int BITS_PER_BYTE = 8;
const unsigned int BYTES_PER_WORD = 4;
const unsigned int HASH_BITS = 128;
const unsigned int MESSAGE_BITS = 512;
const unsigned int CLOCK_PERIOD = 10;
const unsigned int MD5_READY_BYTES = 4;
const unsigned int MD5_HASH_DONE_BYTES = 4;

// Automatically generated constants
const unsigned int HASH_BYTES = HASH_BITS / BITS_PER_BYTE;
const unsigned int HASH_WORDS = HASH_BYTES / BYTES_PER_WORD;
const unsigned int MESSAGE_BYTES = MESSAGE_BITS / BITS_PER_BYTE;
const unsigned int MESSAGE_WORDS = MESSAGE_BYTES / BYTES_PER_WORD;

typedef long unsigned int uint32_t;
typedef long unsigned long uint64_t;
typedef unsigned long vluint64_t;

// Base address of the core on the bus
const uint32_t MD5_BASE = 0x94000000;

// Offset of MD5 data and control registers in device memory map
const uint32_t MD5_READY = MD5_BASE;
const uint32_t MD5_MSG_BASE = MD5_READY + MD5_READY_BYTES;
const uint32_t MD5_HASH_DONE = MD5_MSG_BASE + MESSAGE_BYTES;
const uint32_t MD5_HASH_BASE = MD5_HASH_DONE + MD5_HASH_DONE_BYTES;
const uint32_t MD5_RST = MD5_HASH_BASE + HASH_BYTES;

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
void resetAndReady(void);
void waitForReady(void);
void loadPaddedMessage(const char* msg_ptr);
void strobeMsgValid(void);
void waitForValidOut(void);
void updateHash(unsigned char *pHash);
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

void printWordAsBytesRev(uint32_t pWord) {
    printf("%02X", (unsigned int)(pWord & 0xFF));
    printf("%02X", (unsigned int)((pWord & 0xFF00) >> 8));
    printf("%02X", (unsigned int)((pWord & 0xFF0000) >> 16));
    printf("%02X", (unsigned int)((pWord & 0xFF000000) >> 24));
}

void reportHash() {
    printf("Hash: 0x");
    unsigned char hash[HASH_BYTES];
    updateHash(hash);
    for(unsigned int i = 0; i < HASH_BYTES; ++i) {
        printf("%02X", hash[i]);
    }
    printf("\n");
}

int addPadding(uint64_t pMessageBits, char* buffer) {
  int extraBits = pMessageBits % MESSAGE_BITS;
  int paddingBits = extraBits > 448 ? (2 * MESSAGE_BITS) - extraBits : MESSAGE_BITS - extraBits; // Last 64 bits used to store message size (bytes)
    
  // Add size to end of string
  const int startByte = extraBits / 8;
  const int sizeStartByte =  startByte + ((paddingBits / 8) - 8);
  for(int i = startByte; i < (sizeStartByte + 8); ++i) {
    if(i == startByte) {
      buffer[i] = (unsigned char)0x80; // 1 followed by many 0's
    } else if(i >= sizeStartByte) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      buffer[i] = ((char *)(&pMessageBits))[i - sizeStartByte];
#else
      buffer[i] = ((char *)(&pMessageBits))[7 - (i - sizeStartByte)];
#endif
    } else {
      buffer[i] = 0x0;
    }
  }
    
  return (paddingBits / 8);
}

bool compareHash(const unsigned char * pProposedHash, const char* pExpectedHash, const char * pTestString) {
  char longTemp[(HASH_BITS / 4) + 1];
    
  char *temp = longTemp;
  for(unsigned int i = 0; i < HASH_WORDS; ++i) {
    sprintf(temp, "%8.8x", (unsigned int)((uint32_t *)pProposedHash)[i]);
    temp += 8;
  }
    
  if(strncmp(longTemp, pExpectedHash, HASH_BITS / 4) == 0) {
    printf("PASSED: %s hash test\n", pTestString);
    return true;
  }
    
  printf("FAILED: %s hash test\n", pTestString);
  return false;
}

void hashString(const char *pString, unsigned char *pHash) {
  bool done = false;
  int totalBytes = 0;
    
  printf("Hashing: %s\n", pString);
    
  // Reset for each message
  resetAndReady();
  while(!done) {
    char message[2 * MESSAGE_BITS];
    memset(message, 0, sizeof(message));
        
    // Copy next portion of string to message buffer
    char *msg_ptr = message;
    unsigned int length = 0;
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
      strobeMsgValid();
      waitForValidOut();
      waitForReady();
      reportAppended();
      reportHash();
      updateHash(pHash);
      addedBytes -= MESSAGE_BYTES;
      msg_ptr += MESSAGE_BYTES;
    } while(addedBytes > 0);
  }
}
