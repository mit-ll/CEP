//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      cepregression.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    Regression test program for the CEP
// Notes:          Buildroot does not seem to like having special
//                 characters in the name.
//************************************************************************
//
// These are for real HW
//
#ifndef SIM_ENV_ONLY
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>

#ifndef BARE_MODE
#include <syslog.h>
#include <sys/mman.h>
#endif

//
// ==========================
// For simulation ONLY
// ===========================
#else
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "simdiag_global.h"
#include "portable_io.h"
#include "simPio.h"

// turn this on to sync with HW output else message might be mangle and not sync due to different processes printing
//
//#define printf LOGI
#endif

#include "cepregression.h"


// override printf
#ifdef LINUX_MODE
#include "simdiag_global.h"
#include "cep_io.h"
#define printf(format, ...) do { \
  if (VERBOSE1()) { THR_LOGI(format, ##__VA_ARGS__); }	\
  } while (0)
#endif


//
// syscalls.c has printf
//
#ifdef BARE_MODE
#undef printf
#include "encoding.h"
#define MAP_FAILED	((void *) -1)
//int munmap (void *__addr, size_t __len) { return 1; }
//long sysconf(int name);
extern int printf(const char *format, ...);
#endif
//
//
//
#include "dft_input.h"
#include "fir_input.h"
#include "idft_input.h"
#include "iir_input.h"
#include "CEP.h"


//************************************************************************
// BEGIN: Ip Core Structure 
//************************************************************************
typedef struct ipCoreData {
    char name[MAX_CONFIG_STRING_K];
    size_t address;
    unsigned char *mem;
    int fd;
    size_t pagesize;
    bool enabled;
} IpCore_st;
//************************************************************************
// END: Ip Core Structure 
//************************************************************************

//************************************************************************
// BEGIN: Global Variables
//************************************************************************
IpCore_st ipCores[CEP_TOTAL_CORES];
//************************************************************************
// END: Global Variables
//************************************************************************

//************************************************************************
// BEGIN: CEP MMAP Utility Functions
//************************************************************************
// cep_read - Read from a CEP address
uint64_t cep_read(int device, uint32_t pAddress) {
#ifdef BARE_MODE
  return *(volatile uint64_t*)(ipCores[device].address + pAddress);
#elif SIM_ENV_ONLY
  uint64_t d64;
  DUT_READ32_64( ipCores[device].address + pAddress, d64);
  return d64;
#elif LINUX_MODE
  return lnx_cep_read(ipCores[device].address + pAddress);
#else  
    return *(uint64_t*)(&(ipCores[device].mem)[pAddress]);
#endif
    
}

// cep_write - Write to a CEP address
void cep_write(int device, uint32_t pAddress, uint64_t pData) {
#ifdef BARE_MODE
  *(volatile uint64_t*)(ipCores[device].address + pAddress) = pData;  
#elif SIM_ENV_ONLY
  DUT_WRITE32_64( ipCores[device].address + pAddress, pData);
#elif LINUX_MODE
  return lnx_cep_write(ipCores[device].address + pAddress, pData);  
#else
  memcpy(ipCores[device].mem + pAddress, &pData, sizeof(pData));
#endif  
}
//************************************************************************
// END: CEP MMAP Utility Functions
//************************************************************************


//************************************************************************
// BEGIN: AES Specific utility functions
//************************************************************************
void aes_setPlaintext(const uint64_t* pPT) {
  char str[256];
  char tmp[32];
  //
  sprintf(str,"AES: Plaintext:\t\t0x");
  for(unsigned int i = 0; i < AES_BLOCK_WORDS; ++i) {
    cep_write(AES_BASE_K, AES_PT_BASE + (((AES_BLOCK_WORDS - 1) - i) * BYTES_PER_WORD), pPT[i]);
    sprintf(tmp,"%016lx", pPT[i]); strcat(str,tmp);
  }
  printf("%s\n",str);
}

void aes_setKey(const uint64_t* pKey) {
  char str[256];
  char tmp[32];
  
  sprintf(str,"AES: Key:\t\t0x");
  for(unsigned int i = 0; i < AES_KEY_WORDS; ++i) {
    cep_write(AES_BASE_K, AES_KEY_BASE + (((AES_KEY_WORDS - 1) - i) * BYTES_PER_WORD), pKey[i]);
    sprintf(tmp,"%016lx", pKey[i]); strcat(str,tmp);
  }
  printf("%s\n",str);
}

void aes_start(void) {
    cep_write(AES_BASE_K, AES_START, 0x1);
    cep_write(AES_BASE_K, AES_START, 0x0);
}

bool aes_ciphertextValid(void) {
    return (cep_read(AES_BASE_K, AES_DONE) != 0) ? true : false;
}

void aes_waitForValidOutput(void) {
    while (!aes_ciphertextValid()) {
        ;
    }
}

void aes_saveCiphertext(uint64_t *pCT) {
    for(unsigned int i = 0; i < AES_BLOCK_WORDS; ++i) {
        pCT[i] = cep_read(AES_BASE_K, AES_CT_BASE + (i * BYTES_PER_WORD));
    }
}

void aes_reportCiphertext(void) {
  
  char str[256];
  char tmp[32];
  
  sprintf(str,"AES: Ciphertext:\t0x");
  uint64_t ct[AES_BLOCK_WORDS];
  
  aes_saveCiphertext(ct);
  for(unsigned int i = 0; i < AES_BLOCK_WORDS; ++i) {
    sprintf(tmp,"%016lx", ct[(AES_BLOCK_WORDS - 1) - i]); strcat(str,tmp);
  }
  printf("%s\n",str);
}

bool aes_compareCiphertext(uint64_t* pCT, const char *pExpectedHexString) {
    char longTemp[(AES_BLOCK_BITS / 4) + 1];
    
    // Convert the calculated ciphertext into a text string
    char *temp = longTemp;
    for(int i = (AES_BLOCK_WORDS - 1); i >= 0; --i) {
        sprintf(temp, "%016lx", ((uint64_t *)pCT)[i]);
        temp += 16;
    }
    
    // Now we can compare them as hex strings
    // 2 string characters per byte
    if(strncmp(longTemp, pExpectedHexString, (AES_BLOCK_BYTES * 2)) == 0) {
        return true;
    }
    
    return false;
}

int aes_verifyCiphertext(const char *pExpectedHexString, const char * pTestString) {
  int errCnt = 0;
    uint64_t ct[AES_BLOCK_WORDS];
    aes_saveCiphertext(ct);
    
    if(aes_compareCiphertext(ct, pExpectedHexString)) {
      printf("AES: PASSED: \t\t%s\n", pTestString);
    } else {
      printf("AES: Expected:\t\t0x%s\n", pExpectedHexString);
      printf("AES: FAILED: \t\t%s\n", pTestString);
      errCnt++;
    }
    return errCnt;
}
//************************************************************************
// END: AES Specific utility functions
//************************************************************************

//************************************************************************
// BEGIN: DES Specific utility functions
//************************************************************************
// Not used
#if 0
void des3_remove_bit32(uint32_t* buffer, int pos){
    uint32_t high_half = 0x00000000;
    uint32_t low_half = 0x00000000;
    if (pos > 0)low_half= *buffer << (32 - pos) >> (32 - pos);
    high_half = *buffer >> (pos+1) << (pos);
    *buffer = high_half | low_half;
}
#endif


bool des3_readyValid(void) {
    return (cep_read(DES3_BASE_K, DES3_DONE) != 0) ? true : false;
}

void des3_start(void) {
    cep_write(DES3_BASE_K, DES3_START, 0x1);
    cep_write(DES3_BASE_K, DES3_START, 0x0);
}

void des3_writeToDecrypt(int i) {
    if(i) cep_write(DES3_BASE_K, DES3_DECRYPT, 0x1);
    else  cep_write(DES3_BASE_K, DES3_DECRYPT, 0x0);
}

void des3_setPlaintext(uint64_t* pPT) {
    for(unsigned int i = 0; i < DES3_BLOCK_WORDS; ++i)
        cep_write(DES3_BASE_K, DES3_IN_BASE + (((DES3_BLOCK_WORDS - 1) - i) * BYTES_PER_WORD), pPT[i]);
}

void des3_setKey(uint64_t* pKey) {
    unsigned int i;
    for(i = 0; i < DES3_KEY_WORDS; ++i)
        cep_write(DES3_BASE_K, DES3_KEY_BASE + (((DES3_KEY_WORDS - 1) - i) * BYTES_PER_WORD), pKey[i]);
}

void des3_saveCiphertext(uint64_t *pCT) {
    for(unsigned int i = 0; i < DES3_BLOCK_WORDS; ++i) {
        pCT[(DES3_BLOCK_WORDS - 1) - i] = cep_read(DES3_BASE_K, DES3_CT_BASE + (i * BYTES_PER_WORD));
    }
}

/*Wait for modexp ready signal*/
void des3_waitForValidOutput(){
    while (!des3_readyValid()) {
        ;
    }
}

/*Check if both inputs are equal*/
bool des3_assertEquals(int index, uint64_t expected_1, uint64_t actual_1){

    if ((expected_1 == actual_1)){
    printf("DES3: *** PASSED   (%0d): %016lx Got %016lx\r\n", index, (uint64_t) expected_1, (uint64_t) actual_1);
     return true;// success
    }else{                   
    printf("DES3: *** Expected (%0d): %016lx Got %016lx\r\n", index, (uint64_t) expected_1, (uint64_t) actual_1);
     return false;// failure
    }
}

/*Verify outcome of testcases*/
bool des3_assertSuccess(bool success){
    if (success){ printf("DES3: *** Test    -> PASSED\r\n");        
                  return true;
    }else{        printf("DES3: *** Test    -> FAILED\r\n");
                  return false;
    }
}

//************************************************************************
// END: DES Specific utility functions
//************************************************************************


//************************************************************************
// BEGIN: DFT Specific utility functions
//************************************************************************
bool dft_readyValid(void) {
    return ((cep_read(DFT_BASE_K, DFT_DONE) && 0x0000000000000004) != 0) ? true : false;
}

void dft_start(void) {
    cep_write(DFT_BASE_K, DFT_START, 0x1);
    cep_write(DFT_BASE_K, DFT_START, 0x0);
}

void dft_setX(uint16_t i, uint16_t pX0, uint16_t pX1, uint16_t pX2, uint16_t pX3){
    cep_write(DFT_BASE_K, DFT_IN_DATA, (uint64_t)pX3 << 48 | (uint64_t) pX2 << 32 | (uint64_t)pX1 << 16 | (uint64_t)pX0);             //Write data
    cep_write(DFT_BASE_K, DFT_IN_ADDR, i);                       //Write addr
    cep_write(DFT_BASE_K, DFT_IN_WRITE, 0x2);                    //Load data
    cep_write(DFT_BASE_K, DFT_IN_WRITE, 0x0);                    //Stop

}

void dft_getY(uint16_t i, uint64_t *pY0) {
    uint64_t temp;
    cep_write(DFT_BASE_K, DFT_OUT_ADDR, i);                //Write addr

    temp=cep_read(DFT_BASE_K, DFT_OUT_DATA);             //Read data
    *pY0=temp;
}

/*Wait for modexp ready signal*/
void dft_waitForValidOutput(){
    while (!dft_readyValid()) {
        ;
    }
}

/*Check if both inputs are equal*/
bool dft_assertEquals(int index, uint64_t expected[1], uint64_t actual[1]){

    if ((expected[0] == actual[0])){
    printf("DFT: *** PASSED   (%02d): %16.16lx Got: %16.16lx\r\n", index, expected[0],actual[0]);
     return true;// success
    }else{                   
    printf("DFT: *** Expected (%02d): %16.16lx Got: %16.16lx\r\n", index, expected[0],actual[0]);
     return false;// failure
    }
}

/*Verify outcome of testcases*/
bool dft_assertSuccess(bool success){
    if (success){ printf("DFT: *** Test    -> PASSED\r\n");        
                  return true;
    }else{        printf("DFT: *** Test    -> FAILED\r\n");
                  return false;
    }
}
//************************************************************************
// END: DFT Specific utility functions
//************************************************************************


//************************************************************************
// BEGIN: IDFT Specific utility functions
//************************************************************************
bool idft_readyValid(void) {
    return ((cep_read(IDFT_BASE_K, IDFT_DONE) && 0x0000000000000004) != 0) ? true : false;
}

void idft_start(void) {
    cep_write(IDFT_BASE_K, IDFT_START, 0x1);
    cep_write(IDFT_BASE_K, IDFT_START, 0x0);
}

void idft_setX(uint16_t i, uint16_t pX0, uint16_t pX1, uint16_t pX2, uint16_t pX3){
    cep_write(IDFT_BASE_K, IDFT_IN_DATA, (uint64_t)pX3 << 48 | (uint64_t)pX2 << 32 | (uint64_t)pX1<<16 | (uint64_t)pX0);             //Write data
    cep_write(IDFT_BASE_K, IDFT_IN_ADDR, i);                       //Write addr
    cep_write(IDFT_BASE_K, IDFT_IN_WRITE, 0x2);                    //Load data
    cep_write(IDFT_BASE_K, IDFT_IN_WRITE, 0x0);                    //Stop
}

void idft_getY(uint16_t i, uint64_t *pY0) {
    uint64_t temp;
    cep_write(IDFT_BASE_K, IDFT_OUT_ADDR, i);                //Write addr

    temp=cep_read(IDFT_BASE_K, IDFT_OUT_DATA);             //Read data
    *pY0=temp;
}

/*Wait for modexp ready signal*/
void idft_waitForValidOutput(){
    while (!idft_readyValid()) {
        ;
    }
}

/*Check if both inputs are equal*/
bool idft_assertEquals(int index, uint64_t expected[1], uint64_t actual[1]){

    if (expected[0] == actual[0]){
    printf("IDFT: *** PASSED   (%02d): %16.16lx Got: %16.16lx\r\n", index, expected[0],actual[0]);
     return true;// success
    }else{                   
    printf("IDFT: *** Expected (%02d): %16.16lx Got: %16.16lx\r\n", index, expected[0],actual[0]);
     return false;// failure
    }
}

/*Verify outcome of testcases*/
bool idft_assertSuccess(bool success){
    if (success){ printf("IDFT: *** Test    -> PASSED\r\n");        
                  return true;
    }else{        printf("IDFT: *** Test    -> FAILED\r\n");
                  return false;
    }
}
// ************************************************************************
// END: IDFT Specific utility functions
// ************************************************************************


//************************************************************************
// BEGIN: FIR Specific utility functions
//************************************************************************
bool fir_readyValid(void) {
    return ((cep_read(FIR_BASE_K, FIR_DONE) && 0x0000000000000004) != 0) ? true : false;
}

void fir_start(void) {
    cep_write(FIR_BASE_K, FIR_START, 0x01);
    cep_write(FIR_BASE_K, FIR_START, 0x00);
}

void fir_setInData(uint64_t i, uint64_t idata){
    cep_write(FIR_BASE_K, FIR_IN_DATA,  idata);  // Write data
    cep_write(FIR_BASE_K, FIR_IN_ADDR,  i);      // Write addr
    cep_write(FIR_BASE_K, FIR_IN_WRITE, 0x02);   // Load data
    cep_write(FIR_BASE_K, FIR_IN_WRITE, 0x00);   // Stop
}

uint64_t fir_getOutData(uint64_t i) {
    uint64_t temp;
    
    cep_write(FIR_BASE_K, FIR_OUT_ADDR, i);      // Read addr
    temp=cep_read(FIR_BASE_K, FIR_OUT_DATA);     // Read data
    return temp;
}

/*Wait for modexp ready signal*/
void fir_waitForValidOutput(){
    while (!fir_readyValid()) {
        ;
    }
}

/*Check if both inputs are equal*/
bool fir_assertEquals(int index, uint64_t expected, uint64_t actual){

    if (expected == actual){
    printf("FIR: *** PASSED   (%02d): %08x Got: %08x\r\n", index, (unsigned int)expected, (unsigned int)actual);
     return true;// success
    } else {                   
    printf("FIR: *** Expected (%02d): %08x Got: %08x\r\n", index, (unsigned int)expected, (unsigned int)actual);
     return false;// failure
    }
}

/*Verify outcome of testcases*/
bool fir_assertSuccess(bool success){
    if (success){ printf("FIR: *** Test    -> PASSED\r\n");        
                  return true;
    }else{        printf("FIR: *** Test    -> FAILED\r\n");
                  return false;
    }
}
//************************************************************************
// END: FIR Specific utility functions
//************************************************************************


//************************************************************************
// BEGIN: IIR Specific utility functions
//************************************************************************
bool iir_readyValid(void) {
    return ((cep_read(IIR_BASE_K, IIR_DONE) && 0x0000000000000004) != 0) ? true : false;
}

void iir_start(void) {
    cep_write(IIR_BASE_K, IIR_START, 0x01);
    cep_write(IIR_BASE_K, IIR_START, 0x00);
}

void iir_bus_reset(void) {
    cep_write(IIR_BASE_K, IIR_RESET, 0x00);
    cep_write(IIR_BASE_K, IIR_RESET, 0x01);
}




void iir_setInData(uint64_t i, uint64_t idata){
    cep_write(IIR_BASE_K, IIR_IN_DATA, idata); //Write data
    cep_write(IIR_BASE_K, IIR_IN_ADDR, i);     //Write addr
    cep_write(IIR_BASE_K, IIR_IN_WRITE, 0x02);  //Load data
    cep_write(IIR_BASE_K, IIR_IN_WRITE, 0x00);  //Stop
}

uint64_t iir_getOutData(uint64_t i) {
    uint64_t temp;
    
    cep_write(IIR_BASE_K, IIR_OUT_ADDR, i);    //Write addr
    temp=cep_read(IIR_BASE_K, IIR_OUT_DATA); //Read data
    return temp;
}

/*Wait for modexp ready signal*/
void iir_waitForValidOutput(){
    while (!iir_readyValid()) {
        ;
    }
}

/*Check if both inputs are equal*/
bool iir_assertEquals(int index, uint64_t expected, uint64_t actual){

    if (expected == actual){
    printf("IIR: *** PASSED   (%02d): %08x Got: %08x\r\n", index, (unsigned int)expected, (unsigned int)actual);
     return true;// success
    }else{                   
    printf("IIR: *** Expected (%02d): %08x Got: %08x\r\n", index, (unsigned int)expected, (unsigned int)actual);
     return false;// failure
    }
}

/*Verify outcome of testcases*/
bool iir_assertSuccess(bool success){
    if (success){ printf("IIR: *** Test    -> PASSED\r\n");        
                  return true;
    }else{        printf("IIR: *** Test    -> FAILED\r\n");
                  return false;
    }
}
//************************************************************************
// END: IIR Specific utility functions
//************************************************************************


//************************************************************************
// BEGIN: GPS Specific utility functions
//************************************************************************

void gps_update_sv_num(void) {
    cep_write(GPS_BASE_K, GPS_SV_NUM, 0xC);
}

void gps_reset_sv_num(void) {
    cep_write(GPS_BASE_K, GPS_SV_NUM, 0x0);
}

void gps_bus_reset(void) {
    cep_write(GPS_BASE_K, GPS_RESET, 0x01);
    cep_write(GPS_BASE_K, GPS_RESET, 0x00);
}
void gps_generateNextCode(void) {
    cep_write(GPS_BASE_K, GPS_GEN_NEXT, 0x1);
    cep_write(GPS_BASE_K, GPS_GEN_NEXT, 0x0);
}



bool gps_codeReady(void) {
    return (cep_read(GPS_BASE_K, GPS_GEN_DONE) != 0) ? true : false;
}

void gps_saveCode(uint64_t *pCT, unsigned pBytes, uint64_t pBaseAddress) {
    for(unsigned int i = 0; i < (pBytes / BYTES_PER_WORD); ++i) {
        pCT[((pBytes / BYTES_PER_WORD) - 1) - i] = cep_read(GPS_BASE_K, pBaseAddress + (i * BYTES_PER_WORD));
    }
}

void gps_waitForValidOutput(void) {
    while (!gps_codeReady()) {
        ;
    }
}

bool gps_compareCode(uint64_t* pC, const char *pExpectedHexString, uint64_t pBytes) {
  char longTemp[GPS_P_BYTES*2 + 1]; // 2 characters per byte
    
  // Convert the calculated code into a text string
  char *temp = longTemp;
  for(int i = (pBytes / BYTES_PER_WORD - 1); i >= 0; --i) {
    sprintf(temp, "%16.16lX", (unsigned long int)(pC[i]));
    temp += 16;
  }

  printf("GPS: Received Code: 0x%s\n", longTemp);

  // Now we can compare them as hex strings
  // 2 string characters per byte
  if(strncmp(longTemp, pExpectedHexString, (pBytes * 2)) == 0) {
    return true;
  }

  return false;
}

void gps_verifyCode(const char *pExpectedHexString, const char * pTestString, uint64_t pBytes, uint64_t pBaseAddress) {
  uint64_t c[GPS_P_BYTES / BYTES_PER_WORD];
  printf("GPS: Expected Code: 0x%s\n", pExpectedHexString);
  
  gps_saveCode(c, pBytes, pBaseAddress);
    
  if(gps_compareCode(c, pExpectedHexString, pBytes)) {
    printf("GPS: PASSED: %s\n", pTestString);
  } else {
    printf("GPS: FAILED: %s\n", pTestString);
    }
}
//************************************************************************
// END: GPS Specific utility functions
//************************************************************************


//************************************************************************
// BEGIN: MD5 Specific utility functions
//************************************************************************


void md5_waitForReady() {
  while(cep_read(MD5_BASE_K, MD5_READY) == 0) {
    ;
  }
}

void md5_waitForValid() {
  while(cep_read(MD5_BASE_K, MD5_MSG_OUT_VALID) == 0) {
    ;
  }
}

void md5_updateHash(unsigned char *pHash) {
    uint64_t* hPtr = (uint64_t *)pHash;
    //    uint64_t temp_num;
    //for(int i = 0; i < MD5_HASH_WORDS; ++i) {
    for(int i = MD5_HASH_WORDS - 1; i >= 0; --i) {
        *hPtr++ = cep_read(MD5_BASE_K, MD5_HASH_BASE + (i * BYTES_PER_WORD));
    }
}

void md5_resetAndReady() {
  cep_write(MD5_BASE_K, MD5_RST, 0x1);
  cep_write(MD5_BASE_K, MD5_RST, 0x0);
  md5_waitForReady();
}

void md5_strobeMsgValid() {
  cep_write(MD5_BASE_K, MD5_MSG_IN_VALID, 0x1);
  cep_write(MD5_BASE_K, MD5_MSG_IN_VALID, 0x0);
}

void md5_loadPaddedMessage(const char* msg_ptr) {
  for(unsigned int i = 0; i < MD5_MESSAGE_WORDS; ++i) {
    uint64_t temp = 0;
    for(int j = 0; j < (int)BYTES_PER_WORD; ++j) {
      ((char *)(&temp))[j] = *msg_ptr++;
    }

    cep_write(MD5_BASE_K, MD5_MSG_BASE + (i * BYTES_PER_WORD), temp);
  }

}

int md5_addPadding(uint64_t pMessageBits, char* buffer) {
  int extraBits = pMessageBits % MD5_MESSAGE_BITS;
  int paddingBits = extraBits > 448 ? (2 * MD5_MESSAGE_BITS) - extraBits :MD5_MESSAGE_BITS - extraBits; // Last 64 bits used to store message size (bytes)
    
  // Add size to end of string
  const int startByte = extraBits / BITS_PER_BYTE;
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

void md5_reportHash() {
    printf("MD5: Hash: 0x");
    unsigned char hash[MD5_HASH_BYTES];
    md5_updateHash(hash);
    for(unsigned int i = 0; i < MD5_HASH_BYTES; ++i) {
        printf("%02X", hash[i]);
    }
    printf("\n");
}

bool md5_compareHash(const unsigned char * pProposedHash, const char* pExpectedHash, const char * pTestString) {
  char longTemp[(MD5_HASH_BITS / 4) + 1];
    
  char *temp = longTemp;
  for(unsigned int i = 0; i < MD5_HASH_WORDS; ++i) {
    sprintf(temp, "%16.16lx", (unsigned long int)((uint64_t *)pProposedHash)[i]);
    temp += 16;
  }
    
  if(strncmp(longTemp, pExpectedHash, MD5_HASH_BITS / 4) == 0) {
    printf("MD5: PASSED\n");
    return true;
  }
  printf("MD5: FAILED\n");
  return false;
}

void md5_hashString(const char *pString, unsigned char *pHash) {
  bool done = false;
  int totalBytes = 0;
    
  printf("MD5: Hashing: %s\n", pString);
    
  // Reset for each message
  md5_resetAndReady();
  while(!done) {
    char message[2 * MD5_MESSAGE_BITS];
    memset(message, 0, sizeof(message));       
    // Copy next portion of string to message buffer
    char *msg_ptr = message;
    unsigned int length = 0;
    while(length < MD5_MESSAGE_BYTES) {
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
      addedBytes = md5_addPadding(totalBytes * BITS_PER_BYTE, message);
    }
        
    // Send the message
    msg_ptr = message;
    //printf("\nMD5 message = %s\n",*message);
    do {
      md5_waitForReady();
      md5_loadPaddedMessage(msg_ptr);
      md5_strobeMsgValid();
      md5_waitForReady();
      md5_reportHash();
      md5_updateHash(pHash);
      addedBytes -= MD5_MESSAGE_BYTES;
      msg_ptr += MD5_MESSAGE_BYTES;
    } while(addedBytes > 0);
  }
}
//************************************************************************
// END: MD5 Specific utility functions
//************************************************************************


//************************************************************************
// BEGIN: RSA Specific utility functions
//************************************************************************
/*Wait for modexp ready signal*/
void rsa_wait_ready(){
    while(cep_read(RSA_ADDR_BASE_K, RSA_ADDR_STATUS)!= 0x0000000000000001);
}

/*Check if both inputs are equal*/
bool rsa_assertEquals(uint32_t expected, uint32_t actual){
    if (expected == actual) {
        printf("RSA: *** Expected: %08lX, got %08lX\r\n", (unsigned long int) expected, (unsigned long int) actual);
        return true;// success
    }
    else{                   printf("RSA: *** Expected: %08lX, got %08lX\r\n", (unsigned long int) expected, (unsigned long int) actual);
                            return false;// failure
    }
}

/*Verify outcome of testcases*/
bool rsa_assertSuccess(bool success){
    if (success){ printf("RSA: *** PASSED\r\n");        
                  return true;
    }else{        printf("RSA: *** FAILED\r\n");
                  return false;
    }
}

/*32-bit modulo exponentiation*/
void rsa_modexp_32bits(uint32_t Wmsg, uint32_t Wexp, uint32_t Wmod, uint32_t Wres){
    uint64_t Rres;
    bool success=true;
    
    printf("\r\n");
    printf("RSA: *** Running -> modexp_32bits()\r\n");
  
    printf("RSA: *** Writing -> MES: %08lX EXP: %08lX MOD: %08lx\r\n", (unsigned long int) Wmsg, (unsigned long int) Wexp, (unsigned long int) Wmod);

    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_CTRL, 0x0000000000000004);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_CTRL, 0x0000000000000000);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_DATA, Wexp);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_CTRL, 0x0000000000000003);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_CTRL, 0x0000000000000000);

    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_CTRL , 0x0000000000000004);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_CTRL , 0x0000000000000000);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_DATA    , Wmod);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_CTRL    , 0x0000000000000003);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_CTRL    , 0x0000000000000000);

    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_CTRL , 0x0000000000000004);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_CTRL , 0x0000000000000000);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_DATA    , Wmsg);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_CTRL   , 0x0000000000000003);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_CTRL   , 0x0000000000000000);

    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_LENGTH , 0x0000000000000001);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_LENGTH  , 0x0000000000000001);

    // Start processing and wait for ready.
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_CTRL            , 0x0000000000000002);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_CTRL            , 0x0000000000000000);
    rsa_wait_ready();

    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_RESULT_CTRL  , 0x0000000000000002);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_RESULT_CTRL  , 0x0000000000000000);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_RESULT_CTRL     , 0x0000000000000001);
    Rres = cep_read(RSA_ADDR_BASE_K, RSA_ADDR_RESULT_DATA);
    cep_write(RSA_ADDR_BASE_K, RSA_ADDR_RESULT_CTRL     , 0x0000000000000000);
     
    success=success & rsa_assertEquals(Wres, Rres);
    rsa_assertSuccess(success);
}

//************************************************************************
// END: RSA Specific utility functions
//************************************************************************


//************************************************************************
// BEGIN: SHA256 Specific utility functions
//************************************************************************
void sha256_waitForReady(void) {
    while (cep_read(SHA256_BASE_K, SHA256_READY) == 0) {
        ;
    }
}

void sha256_waitForValidOutput(void) {
    while (cep_read(SHA256_BASE_K, SHA256_HASH_DONE) == 0) {
        ;
    }
}

void sha256_updateHash(char *pHash) {
    uint64_t * temp = (uint64_t *)pHash;
    
    for(unsigned int i = 0; i < SHA256_HASH_WORDS; ++i) {
        *temp++ = cep_read(SHA256_BASE_K, SHA256_HASH_BASE + (i * BYTES_PER_WORD));
    }
}

void sha256_strobeInit(void) {
    cep_write(SHA256_BASE_K, SHA256_NEXT_INIT, 0x0000000000000002);
    cep_write(SHA256_BASE_K, SHA256_NEXT_INIT, 0x0000000000000000);
}

void sha256_strobeNext(void) {
    cep_write(SHA256_BASE_K, SHA256_NEXT_INIT, 0x0000000000000004);
    cep_write(SHA256_BASE_K, SHA256_NEXT_INIT, 0x0000000000000000);
}

void sha256_loadPaddedMessage(const char* msg_ptr) {
  // for(unsigned int i = 0; i < SHA256_MESSAGE_WORDS; ++i) {
    for( int i = 7; i >= 0; --i) {
    uint64_t temp = 0;
    // for(int j = 0; j < BYTES_PER_WORD; ++j) {
    for( int j = 7; j >= 0; --j) {
      ((char *)(&temp))[j] = *msg_ptr++;
     // printf("temp =  0x%016lX,*msg_ptr= 0x%016lX, j=%d\r\n",temp, *msg_ptr, j);
    }

    // printf( "        0x%016lX written to: 0x%016lX\r\n", temp, SHA256_MSG_BASE + (i * BYTES_PER_WORD));
    cep_write(SHA256_BASE_K, SHA256_MSG_BASE + (i * BYTES_PER_WORD), temp);
  }

}

void sha256_reportHash() {
    printf("SHA256: Hash: 0x");
    char hash[SHA256_HASH_BYTES];
    sha256_updateHash(hash);
    for(int i = (SHA256_HASH_BYTES) - 1; i >= 0; --i) {
        printf("%02X", (hash[i] & 0xFF));
    }
    printf("\n");
}

int sha256_addPadding(uint64_t pMessageBits64Bit, char* buffer) {
    int extraBits = pMessageBits64Bit % SHA256_MESSAGE_BITS;
    int paddingBits = extraBits > 448 ? (2 * SHA256_MESSAGE_BITS) - extraBits : SHA256_MESSAGE_BITS - extraBits;
    
    // Add size to end of string
    const int startByte = extraBits / BITS_PER_BYTE;
    const int sizeStartByte =  startByte + ((paddingBits / BITS_PER_BYTE) - 8);
    for(int i = startByte; i < (sizeStartByte + 8); ++i) {
        if(i == startByte) {
            buffer[i] = (unsigned char) 0x80; // 1 followed by many 0's
        } else if( i >= sizeStartByte) {
        buffer[i] = ((char *)(&pMessageBits64Bit))[7 - (i - sizeStartByte)];
    } else {
      buffer[i] = 0x0;
    }
  }
    
  return (paddingBits / 8);
}

bool sha256_compareHash(const char * pProposedHash, const char* pExpectedHash, const char * pTestString) {
    char longTemp[(SHA256_HASH_BITS / 4) + 1];
    
    char *temp = longTemp;
    for(int i = (SHA256_HASH_WORDS - 1); i >= 0; --i) {
        sprintf(temp, "%16.16lx", ((uint64_t *)pProposedHash)[i]);
        temp += 16;
    }
    
    if(strncmp(longTemp, pExpectedHash, SHA256_HASH_BITS / 4) == 0) {
        printf("SHA256: PASSED: %s hash test\n", pTestString);
        return true;
    }
    
    printf("SHA256: FAILED: %s hash test with hash: %s\n", pTestString, temp);
    printf("SHA256: EXPECTED: %s\n", pExpectedHash);
    
    return false;
}

void sha256_hashString(const char *pString, char *pHash) {
    bool done = false;
    bool firstTime = true;
    int totalBytes = 0;
    
    printf("SHA256: Hashing: %s\n", pString);
    // Reset for each message
    sha256_waitForReady();
    while(!done) {
        char message[2 * SHA256_MESSAGE_BITS];
        memset(message, 0, sizeof(message));
        
        // Copy next portion of string to message buffer
        char *msg_ptr = message;
        unsigned int length = 0;
        while(length < SHA256_MESSAGE_BYTES) {
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
            addedBytes = sha256_addPadding(totalBytes * BITS_PER_BYTE, message);
        }
        
        // Send the message
        msg_ptr = message;
        //printf("\nSHA256 message = %s\n",*message);
        do {
            sha256_waitForReady();
            sha256_loadPaddedMessage(msg_ptr);
            if(firstTime) {
                sha256_strobeInit();
                firstTime = false;
            } else {
                sha256_strobeNext();
            }
            sha256_waitForValidOutput();
            sha256_waitForReady();
            //reportAppended();
            sha256_reportHash();
            sha256_updateHash(pHash);
            addedBytes -= SHA256_MESSAGE_BYTES;
            msg_ptr += SHA256_MESSAGE_BYTES;
        } while(addedBytes > 0);
    }
}

// ===========
// AES Test
// ===========
int cep_AES_test(void) {
  int errCnt = 0;
#ifdef BARE_MODE
  ipCores[AES_BASE_K].address = cep_core_info[AES_BASE_K].base_address;
  ipCores[AES_BASE_K].enabled = true;  
#endif
  //************************************************************************
  // Run the AES Tests
  //************************************************************************
  printf("\r\n");
  printf("AES: *********************************************************\r\n");
  printf("AES: * AES test started ...                                  *\r\n");
  printf("AES: *********************************************************\r\n");
  printf("\r\n");    
  
  // AES-192 Core Test Vectors  
  uint64_t pt1[2]  = {0x3243f6a8885a308d, 0x313198a2e0370734};                         
  uint64_t key1[3] = {0x2b7e151628aed2a6, 0xabf7158809cf4f3c, 0x2b7e151628aed2a6};
  aes_setPlaintext(pt1);
  aes_setKey(key1);
  aes_start();
  aes_waitForValidOutput();
  aes_reportCiphertext();
  errCnt += aes_verifyCiphertext("4fcb8db85784a2c1bb77db7ede3217ac", "test 1");
  
  uint64_t pt2[2]  = {0x0011223344556677, 0x8899aabbccddeeff};
  uint64_t key2[3] = {0x0001020304050607, 0x08090a0b0c0d0e0f, 0x0001020304050607};
  aes_setPlaintext(pt2);
  aes_setKey(key2);
  aes_start();
  aes_waitForValidOutput();
  aes_reportCiphertext();
  aes_verifyCiphertext("65d50128b115a7780981475a6bd64a0e", "test 2");
  
  uint64_t pt3[2]  = {0, 0};
  uint64_t key3[3] = {0, 0, 0};
  aes_setPlaintext(pt3);
  aes_setKey(key3);
  aes_start();
  aes_waitForValidOutput();
  aes_reportCiphertext();
  errCnt += aes_verifyCiphertext("aae06992acbf52a3e8f4a96ec9300bd7", "test 3");
    
  uint64_t pt4[2]  = {0, 0};
  uint64_t key4[3] = {0, 0, 1};
  aes_setPlaintext(pt4);
  aes_setKey(key4);
  aes_start();
  aes_waitForValidOutput();
  aes_reportCiphertext();
  errCnt += aes_verifyCiphertext("8bae4efb70d33a9792eea9be70889d72", "test 4");
  
  uint64_t pt5[2]  = {0, 1};
  uint64_t key5[3] = {0, 0, 0};
  aes_setPlaintext(pt5);
  aes_setKey(key5);
  aes_start();
  aes_waitForValidOutput();
  aes_reportCiphertext();
  errCnt += aes_verifyCiphertext("cd33b28ac773f74ba00ed1f312572435", "test 5");
  
  printf("\r\n");
  printf("AES: *********************************************************\r\n");
  printf("AES: * AES test done ...                                     *\r\n");
  printf("AES: *********************************************************\r\n");
  printf("\r\n");    
  
  return errCnt;
}
// ===========
// DES3 Test
// ===========
int cep_DES3_test(void) {
  bool    success = true;  
  // to support multi-thread in bare metal
#ifdef BARE_MODE
  ipCores[DES3_BASE_K].address = cep_core_info[DES3_BASE_K].base_address;
  ipCores[DES3_BASE_K].enabled = true;
#endif
      //************************************************************************
      // Run the Triple-DES tests
      //************************************************************************
      printf("\r\n");
      printf("DES3: *********************************************************\r\n");
      printf("DES3: * Triple-DES test started                               *\r\n");
      printf("DES3: *********************************************************\r\n");
      printf("\r\n");    

      int decrypt=0, select=0;
      uint64_t ct[DES3_BLOCK_WORDS];

      const uint64_t x[10][5]={
        {0x0000000000000000, 0x0000000000000000, 0x0000000000000000 , 0x95F8A5E5DD31D900 ,0x8000000000000000},
        {0x0000000000000000, 0x0000000000000000, 0x0000000000000000 , 0x9D64555A9A10B852 ,0x0000001000000000},
        {0x0038919A62604C4F, 0x0038919A62604C4F, 0x0038919A62604C4F , 0x51454B582DDF440A ,0x7178876E01F19B2A},
        {0x00057055D43FED5B, 0x00057055D43FED5B, 0x00057055D43FED5B , 0x42FD443059577FA2 ,0xAF37FB421F8C4095},
        {0x0000451338957377, 0x0000451338957377, 0x0000451338957377 , 0x736F6D6564617461 ,0x3D124FE2198BA318},
        {0x0000451338957377, 0x0054A952A54A952A, 0x0000451338957377 , 0x736F6D6564617461 ,0xFBABA1FF9D05E9B1},
        {0x0000451338957377, 0x0054A952A54A952A, 0x00FFBAECC76A8C88 , 0x736F6D6564617461 ,0x18d748e563620572},
        {0x0002A4083664208B, 0x0086061B3581084C, 0x006409ADEFF50D2B , 0x7371756967676C65 ,0xc07d2a0fa566fa30},
        {0x0000000000000000, 0x0080000000000000, 0x0000000000000001 , 0x0000000000000000 ,0xe6e6dd5b7e722974},
        {0x00108C41A8932010, 0x00900F40A8830000, 0x00180E4889834000 , 0x0000000000000000 ,0xe1ef62c332fe825b}}; 


      for(decrypt=0;decrypt<2;decrypt=decrypt+1){

          des3_writeToDecrypt(decrypt);
          
          if(decrypt) printf("DES3: Running Encrypt test ...\r\n\r\n");
          else        printf("DES3: Running Decrypt test ...\r\n\r\n");
  
          for(select=0; select<10 ; select=select+1){
              uint64_t key[3]={(x[select][2]),(x[select][1]),(x[select][0])};
              des3_setKey(key);
              uint64_t pt[1]={(x[select][3+decrypt])};
              des3_setPlaintext(pt);
              des3_start();
              des3_waitForValidOutput();
              des3_saveCiphertext(ct);

              success = success & des3_assertEquals (select, (x[select][4-decrypt]), ct[0]);
            }
        }

      printf("\r\n");
      printf("DES3: Checking for sucess ...\r\n\r\n");
      des3_assertSuccess(success);
      printf("\r\n");
      printf("\r\n");
      printf("DES3: *********************************************************\r\n");
      printf("DES3: * Triple-DES test done ...                              *\r\n");
      printf("DES3: *********************************************************\r\n");
      printf("\r\n");    
  
  return success ? 0 : 1;
}

// ===========
// DFT Test
// ===========
int cep_DFT_test(void) {
  bool    success = true;  
  int i,j;
  // to support multi-thread in bare metal
#ifdef BARE_MODE
  ipCores[DFT_BASE_K].address = cep_core_info[DFT_BASE_K].base_address;
  ipCores[DFT_BASE_K].enabled = true;
#endif

  //************************************************************************
  // Run the DFT tests
  //************************************************************************
  uint64_t dft_dout[1]={0x0000000000000000};
  
  success=true;
  
  printf("\r\n");
  printf("DFT: *********************************************************\r\n");
  printf("DFT: * DFT test started ...                                  *\r\n");
  printf("DFT: *********************************************************\r\n");
  printf("\r\n");
  
  //Set Inputs
  for(i=0; i< 2; i++){
    //start();
    // The 64 complex data points enter the system over 32 cycles
    for (j=0; j < 32; j++){
      switch(i){
      case 0: dft_setX(j, j*4, j*4+1, j*4+2, j*4+3); 
	//printf("dft_setX case0 completed, Press Any Key to Continue\n");  
	//getchar();   
	break;// Input: 2 complex words per cycle
      case 1: dft_setX(j, 128+j*4, 128+j*4+1, 128+j*4+2, 128+j*4+3); 
	//printf("dft_setX case0 completed, Press Any Key to Continue\n");  
	//getchar();   
	break;// Input 4 words per cycle
      }
    }
    //printf("dft_setX completed, Press Any Key to Continue\n");  
    //getchar();   
    
    dft_start();
    // printf("dft_start completed, Press Any Key to Continue\n");  
    //getchar();   
    //Check Outputs
    dft_waitForValidOutput();
    
    printf("DFT: --- begin output %02d---\r\n", i+1);
    for(j=0;j<32;j++){
      // dft_getY(j, &dft_dout[0], &dft_dout[1], &dft_dout[2], &dft_dout[3]);
      dft_getY(j, &dft_dout[0]);
      switch(i){
      case 0: success = success &dft_assertEquals(j, dft_exp_1[j], dft_dout); break;
      case 1: success = success &dft_assertEquals(j, dft_exp_2[j], dft_dout); break;
      }
    }
  }
  
  printf("\r\n");
  dft_assertSuccess(success);
  
  printf("\r\n");
  printf("DFT: *********************************************************\r\n");
  printf("DFT: * DFT test done ...                                     *\r\n");
  printf("DFT: *********************************************************\r\n");
  printf("\r\n");
  
  return success ? 0 : 1;
}

// ===========
// IDFT Test
// ===========
int cep_IDFT_test(void) {
  bool    success = true;  
  int i,j;
  // to support multi-thread in bare metal
#ifdef BARE_MODE
  ipCores[IDFT_BASE_K].address = cep_core_info[IDFT_BASE_K].base_address;
  ipCores[IDFT_BASE_K].enabled = true;
#endif

  //************************************************************************
  // Run the IDFT tests
  //************************************************************************
  uint64_t idft_dout[1]={0x0000000000000000};
  
  success=true;
  
  printf("\r\n");
  printf("IDFT: *********************************************************\r\n");
  printf("IDFT: * IDFT test started ...                                 *\r\n");
  printf("IDFT: *********************************************************\r\n");
  printf("\r\n");
  
  //Set Inputs
  for(i=0; i < 2; i++){
    //start();
    // The 64 complex data points enter the system over 32 cycles
    for (j=0; j < 32; j++){
      switch(i){
      case 0: idft_setX(j, j*4, j*4+1, j*4+2, j*4+3); break;// Input: 2 complex words per cycle
      case 1: idft_setX(j, 128+j*4, 128+j*4+1, 128+j*4+2, 128+j*4+3); break;// Input 4 words per cycle
      }
    }
    
    idft_start();
    
    //Check Outputs
    idft_waitForValidOutput();
    
    printf("IDFT: --- begin output %02d---\r\n", i+1);
    for(j=0;j<32;j++){
      idft_getY(j, &idft_dout[0]);
      switch(i){
      case 0: success=success&idft_assertEquals(j, idft_exp_1[j], idft_dout); break;
      case 1: success=success&idft_assertEquals(j, idft_exp_2[j], idft_dout); break;
      }
    }
  }
  
  printf("\r\n");
  idft_assertSuccess(success);
  
  printf("\r\n");
  printf("IDFT: *********************************************************\r\n");
  printf("IDFT: * IDFT test done ...                                    *\r\n");
  printf("IDFT: *********************************************************\r\n");
  printf("\r\n");
  
  return success ? 0 : 1;
}

// ===========
// FIR Test
// ===========
int cep_FIR_test(void) {
  bool    success = true;  
  int i;
  // to support multi-thread in bare metal
#ifdef BARE_MODE
  ipCores[FIR_BASE_K].address = cep_core_info[FIR_BASE_K].base_address;
  ipCores[FIR_BASE_K].enabled = true;
#endif

  //************************************************************************
  // Run the FIR tests
  //************************************************************************
  printf("\r\n");
  printf("FIR: *********************************************************\r\n");
  printf("FIR: * FIR test started ...                                  *\r\n");
  printf("FIR: *********************************************************\r\n");
  printf("\r\n");
  
  //Set Inputs and check Outputs
  for (i=0; i < 32; i++)
    fir_setInData(i, i*4);
  
  fir_start();
  fir_waitForValidOutput();
  
  for (i=0; i < 32; i++){
    uint64_t fir_dout=0x0000000000000000;
    fir_dout=fir_getOutData(i);
    success=success&fir_assertEquals(i, fir_exp_1[i], fir_dout);
  }
  
  printf("\r\n");
  fir_assertSuccess(success);
  
  printf("\r\n");
  printf("FIR: *********************************************************\r\n");
  printf("FIR: * FIR test done ...                                     *\r\n");
  printf("FIR: *********************************************************\r\n");
  printf("\r\n");
  
  return success ? 0 : 1;
}

// ===========
// IIR Test
// ===========
int cep_IIR_test(void) {
  bool    success = true;  
  int i;
  // to support multi-thread in bare metal
#ifdef BARE_MODE
  ipCores[IIR_BASE_K].address = cep_core_info[IIR_BASE_K].base_address;
  ipCores[IIR_BASE_K].enabled = true;
#endif
  // 12/17/19 tony: if dataIn to IIR_filter module is randomized (from memory), test will fail if not issue a bus reset
  iir_bus_reset(); // must issue a bus reset to clear the left/rigthOut else will not work
  //
  
  //************************************************************************
  // Run the IIR tests
  //************************************************************************
  success=true;
  
  printf("\r\n");
  printf("IIR: *********************************************************\r\n");
  printf("IIR: * IIR test started ...                                  *\r\n");
  printf("IIR: *********************************************************\r\n");
  printf("\r\n");
  
  //Set Inputs and check Outputs
  for (i=0; i < 32; i++)
    iir_setInData(i, i*4);
  iir_setInData(0,0);
  iir_start();
  iir_waitForValidOutput();
  for (i=0; i < 32; i++){
    uint64_t iir_dout=0x0000000000000000;
    iir_dout=iir_getOutData(i);
    success=success&iir_assertEquals(i, iir_exp_1[i], iir_dout);
  }
  printf("Done with iir_getOutData\r\n");
  
  printf("\r\n");
  iir_assertSuccess(success);
  iir_bus_reset();
  printf("\r\n");
  printf("IIR: *********************************************************\r\n");
  printf("IIR: * IIR test done ...                                     *\r\n");
  printf("IIR: *********************************************************\r\n");
  printf("\r\n");
  
  return success ? 0 : 1;
}

// ===========
// GPS Test
// ===========
int cep_GPS_test(void) {
  bool    success = true;  
  // to support multi-thread in bare metal
#ifdef BARE_MODE
  ipCores[GPS_BASE_K].address = cep_core_info[GPS_BASE_K].base_address;
  ipCores[GPS_BASE_K].enabled = true;
#endif
  
  //************************************************************************
  // Run the GPS tests
  //************************************************************************
  printf("\r\n");
  printf("GPS: *********************************************************\r\n");
  printf("GPS: * GPS test started ...                                  *\r\n");
  printf("GPS: *********************************************************\r\n");
  printf("\r\n");
  gps_bus_reset();
  gps_update_sv_num();
  gps_generateNextCode();
  gps_waitForValidOutput();
  gps_verifyCode("0000000000001F43", "C/A code", GPS_CA_BYTES, GPS_CA_BASE);
  gps_verifyCode("9DDFDD9CE127D8D95394BF2838E7EF54", "P code", GPS_P_BYTES, GPS_P_BASE);
  gps_verifyCode("FD86C6D34155A750E284B05AC643BC27", "L code", GPS_L_BYTES, GPS_L_BASE);
  gps_bus_reset();
  gps_reset_sv_num();
  printf("\r\n");
  printf("GPS: *********************************************************\r\n");
  printf("GPS: * GPS test done ...                                     *\r\n");
  printf("GPS: *********************************************************\r\n");
  printf("\r\n");
  
  return success ? 0 : 1; // @
}

// ===========
// MD5 Test
// ===========
int cep_MD5_test(void) {
  bool    success = true;  
  // to support multi-thread in bare metal
#ifdef BARE_MODE
  ipCores[MD5_BASE_K].address = cep_core_info[MD5_BASE_K].base_address;
  ipCores[MD5_BASE_K].enabled = true;
#endif

  //************************************************************************
  // Run the MD5 tests
  //************************************************************************
  printf("\r\n");
  printf("MD5: *********************************************************\r\n");
  printf("MD5: * MD5 test started                                      *\r\n");
  printf("MD5: *********************************************************\r\n");
  printf("\r\n");
  
  unsigned char md5_hash[MD5_HASH_BYTES];
  
  md5_resetAndReady();
  
  md5_hashString("a", md5_hash);
  md5_compareHash(md5_hash, "0cc175b9c0f1b6a831c399e269772661", "single character");
  
  md5_hashString("abc", md5_hash);
  md5_compareHash(md5_hash, "900150983cd24fb0d6963f7d28e17f72", "three character");
  
  //Test a message that requires an extra chunk to be added to hold padding
  
  md5_hashString("Lincoln LaboratoLincoln LaboratoLincoln LaboratoLincoln Laborato", md5_hash);
  md5_compareHash(md5_hash, "93f3763d2c20d33e5031e20480eaca58", "512-bit sized message");
  
  printf("\r\n");
  printf("MD5: *********************************************************\r\n");
  printf("MD5: * MD5 test done ...                                     *\r\n");
  printf("MD5: *********************************************************\r\n");
  printf("\r\n");
  
  return success ? 0 : 1; //@
}

// ===========
// RSA Test
// ===========
int cep_RSA_test(void) {
  bool    success = true;  
  // to support multi-thread in bare metal
#ifdef BARE_MODE
  ipCores[RSA_ADDR_BASE_K].address = cep_core_info[RSA_ADDR_BASE_K].base_address;
  ipCores[RSA_ADDR_BASE_K].enabled = true;
#endif

  //************************************************************************
  // Run the RSA tests
  //************************************************************************
  
  printf("\r\n");
  printf("RSA: *********************************************************\r\n");
  printf("RSA: * RSA test started ...                                  *\r\n");
  printf("RSA: *********************************************************\r\n");
  printf("\r\n");
  rsa_modexp_32bits(0x00000001, 0x00000002, 0x00000005, 0x00000001); //msg^exp < mod -> 1^2 < 5
  rsa_modexp_32bits(0x00000001, 0x00000002, 0x00000003, 0x00000001); //msg^exp < mod -> 1^2 < 3
  rsa_modexp_32bits(0x00000002, 0x00000002, 0x00000005, 0x00000004); //msg^exp < mod -> 2^2 < 5
  rsa_modexp_32bits(0x00000002, 0x00000002, 0x00000003, 0x00000001); //msg^exp > mod -> 2^2 > 3
  rsa_modexp_32bits(0x00000004, 0x0000000D, 0x000001F1, 0x000001bd); //msg^exp > mod -> 4^13 > 497
  rsa_modexp_32bits(0x01234567, 0x89ABCDEF, 0x11111111, 0x0D9EF081); //msg^exp > mod -> 19088743^2309737967 > 286331153
  rsa_modexp_32bits(0x30000000, 0xC0000000, 0x00A00001, 0x0000CC3F); //msg^exp > mod -> 
  printf("\r\n");
  printf("RSA: *********************************************************\r\n");
  printf("RSA: * RSA test done ...                                     *\r\n");
  printf("RSA: *********************************************************\r\n");
  printf("\r\n");
  
  return success ? 0 : 1; // @
}

// ===========
// SHA256 Test
// ===========
int cep_SHA256_test(void) {
  bool    success = true;  
  // to support multi-thread in bare metal
#ifdef BARE_MODE
  ipCores[SHA256_BASE_K].address = cep_core_info[SHA256_BASE_K].base_address;
  ipCores[SHA256_BASE_K].enabled = true;
#endif

  //************************************************************************
  // Run the SHA256 tests
  //************************************************************************
  char sha256_hash[SHA256_HASH_BYTES];
  
  printf("\r\n");
  printf("SHA256: *********************************************************\r\n");
  printf("SHA256: * SHA256 test started ...                               *\r\n");
  printf("SHA256: *********************************************************\r\n");
  printf("\r\n");
  
  printf("SHA256: Waiting for ready signal...\n");
  
  sha256_waitForReady();
  
  // Test a message that requires an extra chunk to be added to hold padding
  
  // // Test common case 1
  printf("\r\n");
  sha256_hashString("", sha256_hash);
  sha256_compareHash(sha256_hash, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855", "empty string");
  
  // // // Test common case 2
  printf("\r\n");
  sha256_hashString("a", sha256_hash);
  sha256_compareHash(sha256_hash, "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb", "single character");
  
  // // Test common case 3
  printf("\r\n");
  sha256_hashString("abc", sha256_hash);
  sha256_compareHash(sha256_hash, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "three character");
  
  printf("\r\n");
  sha256_hashString("Lincoln LaboratoLincoln LaboratoLincoln LaboratoLincoln Laborato", sha256_hash);
  sha256_compareHash(sha256_hash, "f88c49e2b696d45a699eb10effafb3c9522df6f7fa68c2509d105e849be605ba", "512-bit message");
  
  printf("\r\n");
  printf("SHA256: *********************************************************\r\n");
  printf("SHA256: * SHA256 test done ...                                  *\r\n");
  printf("SHA256: *********************************************************\r\n");
  printf("\r\n");
  
  return success ? 0 : 1; //@
}



//************************************************************************
// BEGIN: Test function
//************************************************************************
int test(int mask) {

  int errCnt = 0;
  // use the mask to shutoff the test
  for (int i=0;i<11;i++) {
    if (((1 << i) & mask) == 0) { ipCores[i].enabled = false; }
  }
  //
  // General use variables
  //
  if (ipCores[AES_BASE_K].enabled == true) 
    {
      errCnt += cep_AES_test();
    }
    if (errCnt) { return 1; }
    
    if (ipCores[DES3_BASE_K].enabled == true)
    {
      errCnt += cep_DES3_test();
    }
    if (errCnt) { return 1; }
    
    if (ipCores[DFT_BASE_K].enabled == true)
    {
      errCnt += cep_DFT_test();      
    }
    if (errCnt) { return 1; }
    
    if (ipCores[IDFT_BASE_K].enabled == true)
    {
      errCnt += cep_IDFT_test();      
    }
    if (errCnt) { return 1; }
    

    if (ipCores[FIR_BASE_K].enabled == true)
    {
      errCnt += cep_FIR_test();
    }
    if (errCnt) { return 1; }
    
    if (ipCores[IIR_BASE_K].enabled == true)
    {
      errCnt += cep_IIR_test();
    }
    if (errCnt) { return 1; }
    
    if (ipCores[GPS_BASE_K].enabled == true)
    {
      errCnt += cep_GPS_test();      
    }
    if (errCnt) { return 1; }
    
    if (ipCores[MD5_BASE_K].enabled == true)
    {
      errCnt += cep_MD5_test();      
    }
    if (errCnt) { return 1; }
    
    if (ipCores[RSA_ADDR_BASE_K].enabled == true)
    {
      errCnt += cep_RSA_test();      
    }
    if (errCnt) { return 1; }
    
    if (ipCores[SHA256_BASE_K].enabled == true)
    {
      errCnt += cep_SHA256_test();      
    }
    //Check Outputs
    return errCnt;

}   // end of test
//************************************************************************
// END: test function
//************************************************************************



//************************************************************************
// BEGIN: Config File Functions
//************************************************************************

int __initConfigDone = 0;
// Initializes the ipCore_st structure for all cores
void initConfig()
{

#ifndef BARE_MODE
    size_t pagesize = sysconf(_SC_PAGE_SIZE); // Assuming every core uses 4k

#else
    size_t pagesize = 1 << 14;
#endif

    for (int i = 0; i < CEP_TOTAL_CORES; i++) {
        ipCores[i].address          = cep_core_info[i].base_address;
        strncpy(ipCores[i].name, cep_core_info[i].name, strlen(cep_core_info[i].name));
        ipCores[i].fd               = -1;
        ipCores[i].mem              = (unsigned char *)MAP_FAILED;
        ipCores[i].pagesize         = pagesize;
        ipCores[i].enabled          = cep_core_info[i].enabled;		
    }

    __initConfigDone = 1;    

}

//************************************************************************
// END: Config File Functions
//************************************************************************



//************************************************************************
// BEGIN: IpCore Cleanup Function
//************************************************************************
void cleanUp()
{
  #ifndef BARE_MODE  
    int i = 0;
    int result;

    for (i = 0; i < CEP_TOTAL_CORES; i++)
    {
        if (ipCores[i].fd != -1)
        {
            close(ipCores[i].fd);
        }
        if (ipCores[i].mem != MAP_FAILED)
        {
            result = munmap(ipCores[i].mem, ipCores[i].pagesize);
            if ( result != 0 )
            {
                printf("Could not unmap %s\n", ipCores[i].name);
            }
        }
    }
    #endif
}
//************************************************************************
// END: IpCore Cleanup Function
//************************************************************************



//************************************************************************
// BEGIN: IpCore Initialization Function
//************************************************************************
int initializeInputs()
{
    int result = 0;

    // skip this in sim...
#if (SIM_ENV_ONLY || BARE_MODE || LINUX_MODE)
#else    
    int i = 0;
    printf("Initializing CEP Cores\n");
    for (i = 0; i < CEP_TOTAL_CORES; i++)
    {

        // Initialize the core only if it has been enabled
        if (ipCores[i].enabled == true) {

            printf("%-16s @ 0x%08lx\n", ipCores[i].name, (int)ipCores[i].address);
            ipCores[i].fd = open("/dev/mem", O_RDWR|O_SYNC);
            if (ipCores[i].fd < 0) {
                printf("Can't open /dev/mem for ip core %s\n", ipCores[i].name);
                cleanUp();
                result = 1;
            }

            // Map memory on device to virtual memory
            ipCores[i].mem = (unsigned char *)mmap(0, ipCores[i].pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, ipCores[i].fd, ipCores[i].address);
            if (ipCores[i].mem == MAP_FAILED) {
                printf("Can't map memory %zu for %s\n", ipCores[i].address, ipCores[i].name);
                cleanUp();
                result = 2;
            }

        } // end if ipCores[i].enabled == true
    }
#endif
    
    return result;
}

//************************************************************************
// END: IpCore Initialization Function
//************************************************************************

//************************************************************************
// BEGIN: Main Function
//************************************************************************
//
// can't have *main* in sim
//
#if defined(SIM_ENV_ONLY) || defined(BFM_MODE) || defined(BARE_MODE) || defined(LINUX_MODE)
  int cepregression_test(int mask)
  {
#else 
    int main(int argc, char *argv[])
    {
      int mask = 0xFFFFFFFF;
#endif

    int result = 0;

    printf("\r\n");
    printf("****************************************************************\r\n");
    printf("*    Common Evaluation Platform (v%02x.%02x) Regression Suite      *\r\n", CEP_MAJOR_VERSION, CEP_MINOR_VERSION);
    printf("****************************************************************\r\n");
    printf("\r\n");

    // Initialize the core configuration data structure
    initConfig();

    // Initialize core I/O
    result = initializeInputs();
    if ( result != 0 ) { 
        printf("Core Initialization Error\r\n");
        return result; 
    }

    // Verify the Software Version equals the Hardware Versionlk

    // Execute tests on enabled cores
    result += test(mask);

    // Be nice, Clean up resources
    cleanUp();

    printf("\r\n");
    printf("******************************************************************\r\n");
    printf("* Common Evaluation Platform (v%02x.%02x) Regression Suite Complete  *\r\n", CEP_MAJOR_VERSION, CEP_MINOR_VERSION);
    printf("******************************************************************\r\n");
    printf("\r\n");


    return result;
}
//************************************************************************
// END: Main Function
//************************************************************************
