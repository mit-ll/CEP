// State and functionality common to all tests

// Base address of the core on the bus
const uint32_t MD5_BASE = 0xF0000000;

// Offset of MD5 data and control registers in device memory map
const uint32_t MD5_READY = 0;
const uint32_t MSG_BASE = 1;
const uint32_t HASH_DONE = 17;
const uint32_t HASH_BASE = 18;
const uint32_t MD5_RST = 22;

// Level-dependent functions
void resetAndReady(void);
void waitForReady(void);
void loadPaddedMessage(const char* msg_ptr);
void strobeMsgValid(void);
void waitForValidOut(void);
void updateHash(char *pHash);
void reportHash(void);
void reportAppended(void);

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
      buffer[i] = ((char *)(&pMessageBits64Bit))[i - sizeStartByte];
    } else {
      buffer[i] = 0x0;
    }
  }
    
  return (paddingBits / 8);
}

void printWordAsBytes(uint32_t pWord) {
  cout << hex << uppercase << setfill('0') << setw(2) << ((pWord & 0xFF000000) >> 24);
  cout << hex << uppercase << setfill('0') << setw(2) << ((pWord & 0xFF0000) >> 16);
  cout << hex << uppercase << setfill('0') << setw(2) << ((pWord & 0xFF00) >> 8);
  cout << hex << uppercase << setfill('0') << setw(2) << (pWord & 0xFF);
}

void printWordAsBytesRev(uint32_t pWord) {
  cout << hex << uppercase << setfill('0') << setw(2) << (pWord & 0xFF);
  cout << hex << uppercase << setfill('0') << setw(2) << ((pWord & 0xFF00) >> 8);
  cout << hex << uppercase << setfill('0') << setw(2) << ((pWord & 0xFF0000) >> 16);
  cout << hex << uppercase << setfill('0') << setw(2) << ((pWord & 0xFF000000) >> 24);
}

bool compareHash(const char * pProposedHash, const char* pExpectedHash, const char * pTestString) {
  char longTemp[(128 / 4) + 1];
    
  char *temp = longTemp;
  for(int i = 0; i < ((128 / 8) / 4); ++i) {
    sprintf(temp, "%8.8x", ((uint32_t *)pProposedHash)[i]);
    temp += 8;
  }
    
  if(strncmp(longTemp, pExpectedHash, 128 / 4) == 0) {
    printf("PASSED: %s hash test\n", pTestString);
    return true;
  }
    
  printf("FAILED: %s hash test\n", pTestString);
  return false;
}

void hashString(const char *pString, char *pHash) {
  bool done = false;
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
    while(length < (512 / 8)) {
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
      strobeMsgValid();
      waitForValidOut();
      waitForReady();
      //reportAppended();
      reportHash();
      updateHash(pHash);
      addedBytes -= (512 / 8);
      msg_ptr += (512 / 8);
    } while(addedBytes > 0);
  }
}
