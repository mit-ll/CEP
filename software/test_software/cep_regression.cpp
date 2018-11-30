//************************************************************************
// Copyright (C) 2018 Massachusetts Institute of Technology
//
// File Name:      cep_regression.cpp 
// Program:        Common Evaluation Platform (CEP)
// Description:    Regression test program for the CEP
// Notes:          
//************************************************************************

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "CEP.h"
#include "dft_input.h"
#include "idft_input.h"
#include "fir_input.h"
#include "iir_input.h"
#include "rsa_input.h"

//************************************************************************
// BEGIN: Simple greeting function
//************************************************************************
void print_greeting() {

    printf("\n");
    printf("\n");
    printf("(&&&&&&&&&&&&&&&&&&&&&&&&&/..    .*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
    printf("./&&&&&&&&&&&&&&&&&&&&&&&/.    ..*#&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&/\n");
    printf(" ,(&&&&&&&&&&&&&&&&&&&&((,     ,,(&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&((,\n");
    printf("  .//&&&&&&&&&&&&&&&&&/..     ,((&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&(,, \n");
    printf("   ,,(&&&&&&&&&&&&&&&(,      ./&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*     \n");
    printf("     ,(&&&&&&&&&&&&((.     ../&&&&&&&&############################################################&&&&&&&&&&&&#*      \n");
    printf("     .(&&&&&&&&&&&&**.     **#&&&&&((/************************************************************(&&&&&&&&&&&&((.    \n");
    printf("     ./&&&&&&&&&&&&**     *##&&##/,                                                              .*&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**    .(&&&(**.         Common Evaluation Platform %s                         .*&&&&&&&&&&&&//.    \n", CEP_VERSION);
    printf("     ./&&&&&&&&&&&&**  ,,/(**.                                                                    *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**  ,,*,..     .,,*,,,..        ....,,,,,,,,....        ...,,,,,..             *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**          ../###########(///(###################((//(########&##/.           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**          ,,(&((*,**(#&&&&&&&&((/***,,,,,***/(&&&&&&&&##/*,,*(&&(,           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**          ..(#//..**(&##(/((#&##/,..     .,,/#&&#(//(#&&(*.../##(.           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&**          ../###**((&#//,.,,*(##&(**.   ,/((&#((*...,/##&(***###*.           *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&**            ./&&&&((*.        ..,(##&###&#//,.        ..*#&&&&//.            *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&**             *##&&//.            ,((&&&&&(,,            ./&&**             *&&&&&&&&&&&&//.        \n");
    printf("     ./&&&&&&&&&&**            .(&&&&##*.        ..*(&&#///#&((*.        ..*#&&&&//.            *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&**            *###((##(,       .**(#((*...*(&&#*..      ,,(#(((###,            *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&&&**          ..(#//..**##//. ..*###(,       .,,(###*.  ./###*.../##(.           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**          ,,(#**. ../###/,**##((,           ,(###*,,/###/.  ./##(,           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**          ,,##**     ,((&&&&(,                ,,(&&&&(,,     *###,           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&**          ,,##**     .//&&&&/.                ../&&&&/..     *###,           *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&**          ,,(#//.   ,/###(###(,,             ,((#(((##((,   ./##(,           *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&**          ..(#//. ../###*,**(#((,           ,(##(*,,*###/.  ./##(.           *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&**            *###*,((#(,,    .*###(,,     ,((##**.    ,((#(***###*            *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&**            ,(##((###*       .**##((,   ,(###*..      ,,(#(((###,            *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&**             *##&&((,           ./###(((##//.           ,(&&**             *&&&&&&&&&&&&//.        \n");
    printf("     ./&&&&&&&&&&**             ,((&&//.            .//&&&&#/..            .*##&#,,             *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&&&**             ,((&&//.            .//#&&&#/..            .*##&#,,             *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**             *##&&((,           ./###(((##//.           ,(&&**             *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&&&**            ,(##(/###*..     .**##((,   ,(###*..     .**##//(#((,            *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&**            *###*,//#(,,    .*###(,,     ,((##**.    ,((#(,,*###*            *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&&&**          ../#((. ..*###/,**(#((,           ,(##(*,,/###*.  ./##/.           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**          ..(#//.   ./###(###(,,             *((##((##//.   ./##(,           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**          ,,##**     .//&&&&/.                ..(&&&&/..     *###,           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**          ,,##**     ,((&&&&(,                **(&&&&(,,     *###,           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**          ,,(#//. ../###/,**##((,           ,(##(*,,/###/.  ./##(,           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&**          ..(#//..**##//. ..*###(,       .,,(###*.  ,/###*.../##(.           *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&**            *###((##(,       .**##((*...*(&&(*..      ,,(#(((###,            *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&&&**            .(&&&&((*.        ..*(###///##((*.        ..*#&&&&((.            *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**             *##&&//.            ,((&&&&&(,,            ./&&**             *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&&&**            ./&&&&((*.        ..,(##&###&#//,.        ..*#&&&&//.            *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&**          ../###**((&#//,.,,*(##&(**.   ,/((&#((*...,/##&(***###*.           *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&**          ..(#//..**(&##(/((#&##/,..     .,,/#&&#(//(#&&(*.../&&(.           *&&&&&&&&&&&&//.      \n");
    printf("     ./&&&&&&&&&&&&**          ,,(&((*,**(#&&&&&&&&((/***,,,,,***/(&&&&&&&&##/*,,*(&&(,           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**          ../#&&########(/(((###################((//(########&##/.           *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**             .,,,,,,..        ....,,,,,,,,....        ...,,,,,..     .,,*,  .*&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**                                                                    .*((/,  .*&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**                             Copyright (c) 2018                 .*((#&((.    *&&&&&&&&&&&&//.    \n");
    printf("     ./&&&&&&&&&&&&**                    Massachusetts Institute of Technology     ,,/#&&&#**     *&&&&&&&&&&&&//.    \n");
    printf("     .(&&&&&&&&&&&&((**                                                          /(&&&&&*      .*&&&&&&&&&&&&((.      \n");
    printf("     ,#&&&&&&&&&&&&&&############################################################&&&&&&&&/.      .(&&&&&&&&&&&&((,    \n");
    printf("   **#&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//.     ,,(&&&&&&&&&&&&&&&(,   \n");
    printf("  ,((&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&(,,     .//&&&&&&&&&&&&&&&&&/.. \n");
    printf(" ,(&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&(,      ,(&&&&&&&&&&&&&&&&&&&&((,\n");
    printf("./&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&#*     ../&&&&&&&&&&&&&&&&&&&&&&&/  \n");
    printf("\n");
    printf("\n");

} // print_greeting
//************************************************************************
// END: Simple greeting function
//************************************************************************


//************************************************************************
// BEGIN: CEP Utility Functions
//************************************************************************
// cep_read - Read from a CEP address
uint32_t cep_read(uint32_t pAddress) {
    return *((volatile uint32_t *)pAddress);
}

// cep_write - Write to a CEP address
void cep_write(uint32_t pAddress, uint32_t pData) {
    *((volatile uint32_t *)pAddress) = pData;
}
//************************************************************************
// END: CEP Utility Functions
//************************************************************************


//************************************************************************
// BEGIN: AES Specific utility functions
//************************************************************************
void aes_setPlaintext(const uint32_t* pPT) {
    printf("AES: Plaintext:\t\t0x");
    for(unsigned int i = 0; i < AES_BLOCK_WORDS; ++i) {
        cep_write(AES_PT_BASE + (((AES_BLOCK_WORDS - 1) - i) * BYTES_PER_WORD), pPT[i]);
    printf("%08X", pPT[i]);
    }
    printf("\n");
}

void aes_setKey(const uint32_t* pKey) {
    printf("AES: Key:\t\t0x");
    for(unsigned int i = 0; i < AES_KEY_WORDS; ++i) {
        cep_write(AES_KEY_BASE + (((AES_KEY_WORDS - 1) - i) * BYTES_PER_WORD), pKey[i]);
    printf("%08X", pKey[i]);
    }
    printf("\n");
}

void aes_start(void) {
    cep_write(AES_START, 0x1);
    cep_write(AES_START, 0x0);
}

bool aes_ciphertextValid(void) {
    return (cep_read(AES_DONE) != 0) ? true : false;
}

void aes_waitForValidOutput(void) {
    while (!aes_ciphertextValid()) {
        ;
    }
}

void aes_saveCiphertext(uint32_t *pCT) {
    for(unsigned int i = 0; i < AES_BLOCK_WORDS; ++i) {
        pCT[(AES_BLOCK_WORDS - 1) - i] = cep_read(AES_CT_BASE + (i * BYTES_PER_WORD));
    }
}

void aes_reportCiphertext(void) {
    printf("AES: Ciphertext:\t%0x");
    uint32_t ct[AES_BLOCK_WORDS];

    aes_saveCiphertext(ct);
    for(unsigned int i = 0; i < AES_BLOCK_WORDS; ++i) {
      printf("%08X", ct[(AES_BLOCK_WORDS - 1) - i]);
    }
    printf("\n");
}

bool aes_compareCiphertext(uint32_t* pCT, const char *pExpectedHexString) {
    char longTemp[(AES_BLOCK_BITS / 4) + 1];
    
    // Convert the calculated ciphertext into a text string
    char *temp = longTemp;
    for(int i = (AES_BLOCK_WORDS - 1); i >= 0; --i) {
        sprintf(temp, "%8.8x", ((uint32_t *)pCT)[i]);
        temp += 8;
    }
    
    // Now we can compare them as hex strings
    // 2 string characters per byte
    if(strncmp(longTemp, pExpectedHexString, (AES_BLOCK_BYTES * 2)) == 0) {
        return true;
    }
    
    return false;
}

void aes_verifyCiphertext(const char *pExpectedHexString, const char * pTestString) {
    uint32_t ct[AES_BLOCK_WORDS];
    aes_saveCiphertext(ct);
    
    if(aes_compareCiphertext(ct, pExpectedHexString)) {
      printf("AES: PASSED: \t\t%s\n", pTestString);
    } else {
      printf("AES: Expected:\t\t0x%s\n", pExpectedHexString);
      printf("AES: FAILED: \t\t%s\n", pTestString);
    }
}
//************************************************************************
// END: AES Specific utility functions
//************************************************************************


//************************************************************************
// BEGIN: DES Specific utility functions
//************************************************************************
void des3_remove_bit32(uint32_t* buffer, int pos){
    uint32_t high_half = 0x00000000;
    uint32_t low_half = 0x00000000;
    if (pos > 0)low_half= *buffer << (32 - pos) >> (32 - pos);
    high_half = *buffer >> (pos+1) << (pos);
    *buffer = high_half | low_half;
}

bool des3_readyValid(void) {
    return (cep_read(DES3_DONE) != 0) ? true : false;
}

void des3_start(void) {
    cep_write(DES3_START, 0x1);
    cep_write(DES3_START, 0x0);
}

void des3_writeToDecrypt(int i) {
    if(i) cep_write(DES3_DECRYPT, 0x1);
    else  cep_write(DES3_DECRYPT, 0x0);
}

void des3_setPlaintext(uint32_t* pPT) {
    for(unsigned int i = 0; i < DES3_BLOCK_WORDS; ++i)
        cep_write(DES3_IN_BASE + (((DES3_BLOCK_WORDS - 1) - i) * BYTES_PER_WORD), pPT[i]);
}

void des3_setKey(uint32_t* pKey) {
    unsigned int i;
    for(i = 0; i < DES3_KEY_WORDS; ++i)
        cep_write(DES3_KEY_BASE + (((DES3_KEY_WORDS - 1) - i) * BYTES_PER_WORD), pKey[i]);
}

void des3_saveCiphertext(uint32_t *pCT) {
    for(unsigned int i = 0; i < DES3_BLOCK_WORDS; ++i) {
        pCT[(DES3_BLOCK_WORDS - 1) - i] = cep_read(DES3_CT_BASE + (i * BYTES_PER_WORD));
    }
}

/*Wait for modexp ready signal*/
void des3_waitForValidOutput(){
    while (!des3_readyValid()) {
        ;
    }
}

/*Check if both inputs are equal*/
bool des3_assertEquals(int index, uint32_t expected_1, uint32_t expected_2, uint32_t actual_1, uint32_t actual_2){

    if ((expected_1 == actual_1)&(expected_2 == actual_2)){
    printf("DES3: *** PASSED   (%0d): %08x%08x Got %08x%08x\r\n", index, (unsigned int) expected_1, (unsigned int) expected_2, (unsigned int) actual_1, (unsigned int) actual_2);
     return true;// success
    }else{                   
    printf("DES3: *** Expected (%0d): %08x%08x Got %08x%08x\r\n", index, (unsigned int) expected_1, (unsigned int) expected_2, (unsigned int) actual_1, (unsigned int) actual_2);
     return false;// failure
    }
}

/*Verify outcome of testcases*/
bool des3_assertSuccess(bool success){
    if (success){ printf("DES3: *** Test    -> passed\r\n");        
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
    return (cep_read(DFT_DONE) != 0) ? true : false;
}

void dft_start(void) {
    cep_write(DFT_START, 0x1);
    cep_write(DFT_START, 0x0);
}

void dft_setX(uint16_t i, uint16_t pX0, uint16_t pX1, uint16_t pX2, uint16_t pX3){
    cep_write(DFT_IN_DATA, pX1<<16|pX0);             //Write data
    cep_write(DFT_IN_ADDR, i);                       //Write addr
    cep_write(DFT_IN_WRITE, 0x1);                    //Load data
    cep_write(DFT_IN_WRITE, 0x0);                    //Stop
    
    cep_write(DFT_IN_DATA+DFT_BLOCK_BYTES, pX3<<16|pX2); //Write data
    cep_write(DFT_IN_ADDR, i);                       //Write addr
    cep_write(DFT_IN_WRITE, 0x1);                    //Load data
    cep_write(DFT_IN_WRITE, 0x0);                    //Stop
}

void dft_getY(uint16_t i, uint16_t *pY0, uint16_t *pY1, uint16_t *pY2, uint16_t *pY3) {
    uint32_t temp;
    cep_write(DFT_OUT_ADDR, i);                //Write addr

    temp=cep_read(DFT_OUT_DATA);             //Read data
    *pY0=temp;
    *pY1=temp>>16;

    temp=cep_read(DFT_OUT_DATA+DFT_BLOCK_BYTES); //Read data
    *pY2=temp;
    *pY3=temp>>16;
}

/*Wait for modexp ready signal*/
void dft_waitForValidOutput(){
    while (!dft_readyValid()) {
        ;
    }
}

/*Check if both inputs are equal*/
bool dft_assertEquals(int index, uint16_t expected[4], uint16_t actual[4]){

    if ((expected[0] == actual[0])&
        (expected[1] == actual[1])&
        (expected[2] == actual[2])&
        (expected[3] == actual[3])){
    printf("DFT: *** PASSED   (%02d): %04x %04x %04x %04x Got: %04x %04x %04x %04x\r\n", index, expected[0], expected[1], expected[2], expected[3], actual[0], actual[1], actual[2], actual[3]);
     return true;// success
    }else{                   
    printf("DFT: *** Expected (%02d): %04x %04x %04x %04x Got: %04x %04x %04x %04x\r\n", index, expected[0], expected[1], expected[2], expected[3], actual[0], actual[1], actual[2], actual[3]);
     return false;// failure
    }
}

/*Verify outcome of testcases*/
bool dft_assertSuccess(bool success){
    if (success){ printf("DFT: *** Test    -> passed\r\n");        
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
    return (cep_read(IDFT_DONE) != 0) ? true : false;
}

void idft_start(void) {
    cep_write(IDFT_START, 0x1);
    cep_write(IDFT_START, 0x0);
}

void idft_setX(uint16_t i, uint16_t pX0, uint16_t pX1, uint16_t pX2, uint16_t pX3){
    cep_write(IDFT_IN_DATA, pX1<<16|pX0);             //Write data
    cep_write(IDFT_IN_ADDR, i);                       //Write addr
    cep_write(IDFT_IN_WRITE, 0x1);                    //Load data
    cep_write(IDFT_IN_WRITE, 0x0);                    //Stop
    
    cep_write(IDFT_IN_DATA+IDFT_BLOCK_BYTES, pX3<<16|pX2); //Write data
    cep_write(IDFT_IN_ADDR, i);                       //Write addr
    cep_write(IDFT_IN_WRITE, 0x1);                    //Load data
    cep_write(IDFT_IN_WRITE, 0x0);                    //Stop
}

void idft_getY(uint16_t i, uint16_t *pY0, uint16_t *pY1, uint16_t *pY2, uint16_t *pY3) {
    uint32_t temp;
    cep_write(IDFT_OUT_ADDR, i);                //Write addr

    temp=cep_read(IDFT_OUT_DATA);             //Read data
    *pY0=temp;
    *pY1=temp>>16;

    temp=cep_read(IDFT_OUT_DATA+IDFT_BLOCK_BYTES); //Read data
    *pY2=temp;
    *pY3=temp>>16;
}

/*Wait for modexp ready signal*/
void idft_waitForValidOutput(){
    while (!idft_readyValid()) {
        ;
    }
}

/*Check if both inputs are equal*/
bool idft_assertEquals(int index, uint16_t expected[4], uint16_t actual[4]){

    if ((expected[0] == actual[0])&
        (expected[1] == actual[1])&
        (expected[2] == actual[2])&
        (expected[3] == actual[3])){
    printf("IDFT: *** PASSED   (%02d): %04x %04x %04x %04x Got: %04x %04x %04x %04x\r\n", index, expected[0], expected[1], expected[2], expected[3], actual[0], actual[1], actual[2], actual[3]);
     return true;// success
    }else{                   
    printf("IDFT: *** Expected (%02d): %04x %04x %04x %04x Got: %04x %04x %04x %04x\r\n", index, expected[0], expected[1], expected[2], expected[3], actual[0], actual[1], actual[2], actual[3]);
     return false;// failure
    }
}

/*Verify outcome of testcases*/
bool idft_assertSuccess(bool success){
    if (success){ printf("IDFT: *** Test    -> passed\r\n");        
                  return true;
    }else{        printf("IDFT: *** Test    -> FAILED\r\n");
                  return false;
    }
}
//************************************************************************
// END: IDFT Specific utility functions
//************************************************************************


//************************************************************************
// BEGIN: FIR Specific utility functions
//************************************************************************
bool fir_readyValid(void) {
    return (cep_read(FIR_DONE) != 0) ? true : false;
}

void fir_start(void) {
    cep_write(FIR_START, 0x1);
    cep_write(FIR_START, 0x0);
}

void fir_setInData(uint32_t i, uint32_t idata){
    cep_write(FIR_IN_DATA, idata); //Write data
    cep_write(FIR_IN_ADDR, i);     //Write addr
    cep_write(FIR_IN_WRITE, 0x1);  //Load data
    cep_write(FIR_IN_WRITE, 0x0);  //Stop
}

uint32_t fir_getOutData(uint32_t i) {
    uint32_t temp;
    
    cep_write(FIR_OUT_ADDR, i);    //Write addr
    temp=cep_read(FIR_OUT_DATA); //Read data
    return temp;
}

/*Wait for modexp ready signal*/
void fir_waitForValidOutput(){
    while (!fir_readyValid()) {
        ;
    }
}

/*Check if both inputs are equal*/
bool fir_assertEquals(int index, uint32_t expected, uint32_t actual){

    if (expected == actual){
    printf("FIR: *** PASSED   (%02d): %08x Got: %08x\r\n", index, (unsigned int)expected, (unsigned int)actual);
     return true;// success
    }else{                   
    printf("FIR: *** Expected (%02d): %08x Got: %08x\r\n", index, (unsigned int)expected, (unsigned int)actual);
     return false;// failure
    }
}

/*Verify outcome of testcases*/
bool fir_assertSuccess(bool success){
    if (success){ printf("FIR: *** Test    -> passed\r\n");        
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
    return (cep_read(IIR_DONE) != 0) ? true : false;
}

void iir_start(void) {
    cep_write(IIR_START, 0x1);
    cep_write(IIR_START, 0x0);
}

void iir_setInData(uint32_t i, uint32_t idata){
    cep_write(IIR_IN_DATA, idata); //Write data
    cep_write(IIR_IN_ADDR, i);     //Write addr
    cep_write(IIR_IN_WRITE, 0x1);  //Load data
    cep_write(IIR_IN_WRITE, 0x0);  //Stop
}

uint32_t iir_getOutData(uint32_t i) {
    uint32_t temp;
    
    cep_write(IIR_OUT_ADDR, i);    //Write addr
    temp=cep_read(IIR_OUT_DATA); //Read data
    return temp;
}

/*Wait for modexp ready signal*/
void iir_waitForValidOutput(){
    while (!iir_readyValid()) {
        ;
    }
}

/*Check if both inputs are equal*/
bool iir_assertEquals(int index, uint32_t expected, uint32_t actual){

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
    if (success){ printf("IIR: *** Test    -> passed\r\n");        
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
void gps_generateNextCode(void) {
    cep_write(GPS_GEN_NEXT, 0x1);
    cep_write(GPS_GEN_NEXT, 0x0);
}

bool gps_codeReady(void) {
    return (cep_read(GPS_GEN_DONE) != 0) ? true : false;
}

void gps_saveCode(uint32_t *pCT, unsigned pBytes, uint32_t pBaseAddress) {
    for(unsigned int i = 0; i < (pBytes / BYTES_PER_WORD); ++i) {
        pCT[((pBytes / BYTES_PER_WORD) - 1) - i] = cep_read(pBaseAddress + (i * BYTES_PER_WORD));
    }
}

void gps_reportCode(const char * pCode, unsigned pBytes, uint32_t pBaseAddress) {
    printf("%sGPS:  code:\t0\n", pCode);
    uint32_t ct[GPS_P_BYTES / BYTES_PER_WORD + 1];

    gps_saveCode(ct, pBytes, pBaseAddress);
    for(unsigned int i = 0; i < (pBytes / BYTES_PER_WORD); ++i) {
      printf("%08X", (unsigned int)(ct[((pBytes / BYTES_PER_WORD) - 1) - i]));
    }
    printf("\n");
}

void gps_waitForValidOutput(void) {
    while (!gps_codeReady()) {
        ;
    }
}

bool gps_compareCode(uint32_t* pC, const char *pExpectedHexString, uint32_t pBytes) {
  char longTemp[GPS_P_BYTES*2 + 1]; // 2 characters per byte
    
  // Convert the calculated code into a text string
  char *temp = longTemp;
  for(int i = (pBytes / BYTES_PER_WORD - 1); i >= 0; --i) {
    sprintf(temp, "%8.8X", (unsigned int)(pC[i]));
    temp += 8;
  }
  
  printf("GPS: received code: %s\n", longTemp);

  // Now we can compare them as hex strings
  // 2 string characters per byte
  if(strncmp(longTemp, pExpectedHexString, (pBytes * 2)) == 0) {
    return true;
  }
    
  return false;
}

void gps_verifyCode(const char *pExpectedHexString, const char * pTestString, uint32_t pBytes, uint32_t pBaseAddress) {
  uint32_t c[GPS_P_BYTES / BYTES_PER_WORD];
  printf("GPS: Expected:\t0x%s\n", pExpectedHexString);
  
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
void md5_updateHash(unsigned char *pHash) {
    uint32_t* hPtr = (uint32_t *)pHash;
    
    for(int i = MD5_HASH_WORDS - 1; i >= 0; --i) {
        *hPtr++ = cep_read(MD5_HASH_BASE + (i * BYTES_PER_WORD));
    }
}

void md5_reportAppended() {
    printf("Padded input:\n");
    for(int i = MD5_MESSAGE_WORDS - 1; i >= 0; --i) {
    uint32_t dataPart = cep_read(MD5_MSG_BASE + (i * BYTES_PER_WORD));
        printf("0x%08X\n", (unsigned int)dataPart);
    }
}

void md5_waitForReady() {
  while(cep_read(MD5_READY) == 0) {
    ;
  }
}

void md5_resetAndReady() {
  cep_write(MD5_RST, 0x1);
  cep_write(MD5_RST, 0x0);
  md5_waitForReady();
}

void md5_strobeMsgValid() {
  cep_write(MD5_READY, 0x1);
  cep_write(MD5_READY, 0x0);
}

void md5_loadPaddedMessage(const char* msg_ptr) {
  for(unsigned int i = 0; i < MD5_MESSAGE_WORDS; ++i) {
    uint32_t temp = 0;
    for(int j = BYTES_PER_WORD - 1; j >= 0; --j) {
      //printf("%02X ", (unsigned int)(*msg_ptr & 0xFF));
      ((char *)(&temp))[j] = *msg_ptr++;
    }
    //printf("\n 0x%08X written to: 0x%08X\n", temp, MD5_MSG_BASE + (i * 4));
    cep_write(MD5_MSG_BASE + (i * BYTES_PER_WORD), temp);
  }
}

int md5_addPadding(uint64_t pMessageBits, char* buffer) {
  int extraBits = pMessageBits % MD5_MESSAGE_BITS;
  int paddingBits = extraBits > 448 ? (2 * MD5_MESSAGE_BITS) - extraBits :MD5_MESSAGE_BITS - extraBits; // Last 64 bits used to store message size (bytes)
    
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
    sprintf(temp, "%8.8x", (unsigned int)((uint32_t *)pProposedHash)[i]);
    temp += 8;
  }
    
  if(strncmp(longTemp, pExpectedHash, MD5_HASH_BITS / 4) == 0) {
    printf("MD5: PASSED: %s hash test\n", pTestString);
    return true;
  }
    
  printf("MD5: FAILED: %s hash test\n", pTestString);
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
    while(cep_read(RSA_ADDR_STATUS)!= 0x00000001);
}

/*Check if both inputs are equal*/
bool rsa_assertEquals(uint32_t expected, uint32_t actual){
    if (expected == actual) return true;// success
    else{                   printf("RSA: *** Expected: %08X, got %08X\r\n", (unsigned int) expected, (unsigned int) actual);
                            return false;// failure
    }
}

/*Verify outcome of testcases*/
bool rsa_assertSuccess(bool success){
    if (success){ printf("RSA: *** Test    -> passed\r\n");        
                  return true;
    }else{        printf("RSA: *** Test    -> FAILED\r\n");
                  return false;
    }
}

/*Preset Test 32-bit exponent with 2048-bit Modulus*/
void rsa_exp32bit_mod2048bit_test(){
    int i;
    bool success=true;

    printf("\r\n");
    printf("RSA: Test with e = 65537 and 2048 bit modulus -- Encrypting\r\n");

    cep_write(RSA_ADDR_EXPONENT_PTR_RST, 0x00000000);
    cep_write(RSA_ADDR_EXPONENT_DATA   , EXP);

    cep_write(RSA_ADDR_MODULUS_PTR_RST, 0x00000000);
    cep_write(RSA_ADDR_MODULUS_DATA   , 0x00000000);

    for(i=0; i<64; i=i+1) {
        cep_write(RSA_ADDR_MODULUS_DATA, MOD[i]);
    }

    cep_write(RSA_ADDR_MESSAGE_PTR_RST, 0x00000000);
    cep_write(RSA_ADDR_MESSAGE_DATA   , 0x00000000);

    for(i=0; i<64; i=i+1) {
        cep_write(RSA_ADDR_MESSAGE_DATA, MSG[i]);
    }

    cep_write(RSA_ADDR_EXPONENT_LENGTH, 0x00000001);
    cep_write(RSA_ADDR_MODULUS_LENGTH , 0x00000041);

    // Start processing and wait for ready.
    cep_write(RSA_ADDR_CTRL, 0x00000001);
    rsa_wait_ready();

    cep_write(RSA_ADDR_RESULT_PTR_RST, 0x00000000);
    RES[0]=cep_read(RSA_ADDR_RESULT_DATA);
    success=success&rsa_assertEquals(0x00000000, (unsigned int) RES[0]);

    for(i=0; i<64; i=i+1) {
        RES[i]=cep_read(RSA_ADDR_RESULT_DATA);
        success=success&rsa_assertEquals(RES[i], OUT[i]);
    }

    rsa_assertSuccess(success);
}

/*32-bit modulo exponentiation*/
void rsa_modexp_32bits(uint32_t Wmsg, uint32_t Wexp, uint32_t Wmod, uint32_t Wres){
    uint32_t Rres;
    bool success=true;
    
    printf("\r\n");
    printf("RSA: *** Running -> modexp_32bits()\r\n");
  
    printf("RSA: *** Writing -> MES: %08X EXP: %08X MOD: %08X\r\n", (unsigned int) Wmsg, (unsigned int) Wexp, (unsigned int) Wmod);

    cep_write(RSA_ADDR_EXPONENT_PTR_RST, 0x00000000);
    cep_write(RSA_ADDR_EXPONENT_DATA   , Wexp);

    cep_write(RSA_ADDR_MODULUS_PTR_RST , 0x00000000);
    cep_write(RSA_ADDR_MODULUS_DATA    , Wmod);

    cep_write(RSA_ADDR_MESSAGE_PTR_RST , 0x00000000);
    cep_write(RSA_ADDR_MESSAGE_DATA    , Wmsg);

    cep_write(RSA_ADDR_EXPONENT_LENGTH , 0x00000001);
    cep_write(RSA_ADDR_MODULUS_LENGTH  , 0x00000001);

    // Start processing and wait for ready.
    cep_write(RSA_ADDR_CTRL            , 0x00000001);
    rsa_wait_ready();

    cep_write(RSA_ADDR_RESULT_PTR_RST  , 0x00000000);
    Rres=cep_read(RSA_ADDR_RESULT_DATA);
     
    success=success&rsa_assertEquals(Wres, Rres);
    rsa_assertSuccess(success);
}

/*RSA Encrypt*/
void rsa_modexp_encrypt(){
    int i;
    bool success=true;

    printf("\r\n");
    printf("RSA: Encrypting -- exp = 65537 and 2048 bit mod\r\n");

    cep_write(RSA_ADDR_EXPONENT_PTR_RST, 0x00000000);
    cep_write(RSA_ADDR_EXPONENT_DATA   , pubEXP[0]);

    cep_write(RSA_ADDR_MODULUS_PTR_RST, 0x00000000);
    cep_write(RSA_ADDR_MODULUS_DATA   , 0x00000000);

    for(i=0; i<64; i=i+1) {
        cep_write(RSA_ADDR_MODULUS_DATA, pubMOD[i]);
    }

    cep_write(RSA_ADDR_MESSAGE_PTR_RST, 0x00000000);
    cep_write(RSA_ADDR_MESSAGE_DATA   , 0x00000000);

    for(i=0; i<64; i=i+1) {
        cep_write(RSA_ADDR_MESSAGE_DATA, PMSG[i]);
    }

    cep_write(RSA_ADDR_EXPONENT_LENGTH, 0x00000001);
    cep_write(RSA_ADDR_MODULUS_LENGTH , 0x00000041);

    // Start processing and wait for ready.
    cep_write(RSA_ADDR_CTRL, 0x00000001);
    rsa_wait_ready();

    cep_write(RSA_ADDR_RESULT_PTR_RST, 0x00000000);
    CMSG[0]=cep_read(RSA_ADDR_RESULT_DATA);
    success=success&rsa_assertEquals(0x00000000, CMSG[0]);

    for(i=0; i<64; i=i+1) {
        CMSG[i]=cep_read(RSA_ADDR_RESULT_DATA);
    }

    rsa_assertSuccess(success);
}

/*RSA Decrypt*/
void rsa_modexp_decrypt(){
    int i;
    bool success=true;

    printf("\r\n");
    printf("RSA: *** Running -> modexp_decrypt()\r\n");

    printf("RSA: Decrypting -- 2048 bit exp and 2048 bit mod\r\n");

    cep_write(RSA_ADDR_EXPONENT_PTR_RST, 0x00000000);
    cep_write(RSA_ADDR_EXPONENT_DATA   , 0x00000000);
    for(i=0; i<64; i=i+1) {
        cep_write(RSA_ADDR_EXPONENT_DATA, priPRIEXP[i]);
    }

    cep_write(RSA_ADDR_MODULUS_PTR_RST, 0x00000000);
    cep_write(RSA_ADDR_MODULUS_DATA   , 0x00000000);

    for(i=0; i<64; i=i+1) {
        cep_write(RSA_ADDR_MODULUS_DATA, priMOD[i]);
    }

    cep_write(RSA_ADDR_MESSAGE_PTR_RST, 0x00000000);
    cep_write(RSA_ADDR_MESSAGE_DATA   , 0x00000000);

    for(i=0; i<64; i=i+1) {
        cep_write(RSA_ADDR_MESSAGE_DATA, CMSG[i]);
    }

    cep_write(RSA_ADDR_EXPONENT_LENGTH, 0x00000041);
    cep_write(RSA_ADDR_MODULUS_LENGTH , 0x00000041);

    // Start processing and wait for ready.
    cep_write(RSA_ADDR_CTRL, 0x00000001);
    rsa_wait_ready();

    cep_write(RSA_ADDR_RESULT_PTR_RST, 0x00000000);
    GMSG[0]=cep_read(RSA_ADDR_RESULT_DATA);
    success=success&rsa_assertEquals(0x00000000, GMSG[0]);

    for(i=0; i<64; i=i+1) {
        GMSG[i]=cep_read(RSA_ADDR_RESULT_DATA);
        success=success&rsa_assertEquals(PMSG[i], GMSG[i]);
    }
    rsa_assertSuccess(success);
}
//************************************************************************
// END: RSA Specific utility functions
//************************************************************************


//************************************************************************
// BEGIN: SHA256 Specific utility functions
//************************************************************************
void sha256_waitForReady(void) {
    while (cep_read(SHA256_READY) == 0) {
        ;
    }
}

void sha256_waitForValidOutput(void) {
    while (cep_read(SHA256_HASH_DONE) == 0) {
        ;
    }
}

void sha256_reportAppended(void) {
    printf("SHA256: Padded input:\n");
    for(int i = SHA256_MESSAGE_WORDS - 1; i >= 0; --i) {
        printf("SHA256: 0x%08X\n", cep_read(SHA256_MSG_BASE + (i * BYTES_PER_WORD)));
    }
}

void sha256_updateHash(char *pHash) {
    uint32_t * temp = (uint32_t *)pHash;
    
    for(unsigned int i = 0; i < SHA256_HASH_WORDS; ++i) {
        *temp++ = cep_read(SHA256_HASH_BASE + (i * BYTES_PER_WORD));
    }
}

void sha256_strobeInit(void) {
    cep_write(SHA256_NEXT_INIT, 0x1);
    cep_write(SHA256_NEXT_INIT, 0x0);
}

void sha256_strobeNext(void) {
    cep_write(SHA256_NEXT_INIT, 0x2);
    cep_write(SHA256_NEXT_INIT, 0x0);
}

void sha256_loadPaddedMessage(const char* msg_ptr) {
    int temp = 0;
    for(int i = (SHA256_MESSAGE_BYTES - 1); i >= 0; --i) {
        temp = (temp << 8) | ((*msg_ptr++) & 0xFF);
        
        if(i % BYTES_PER_WORD == 0) {
            cep_write(SHA256_MSG_BASE + i, temp);
        }
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

bool sha256_compareHash(const char * pProposedHash, const char* pExpectedHash, const char * pTestString) {
    char longTemp[(SHA256_HASH_BITS / 4) + 1];
    
    char *temp = longTemp;
    for(int i = (SHA256_HASH_WORDS - 1); i >= 0; --i) {
        sprintf(temp, "%8.8x", ((uint32_t *)pProposedHash)[i]);
        temp += 8;
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

//************************************************************************
// BEGIN: SHA256 Specific utility functions
//************************************************************************



//************************************************************************
// BEGIN: Main function
//************************************************************************
int main(int argc, char **argv, char **env) {

    // General use variables
    int     i;
    int     j;
    bool    success = true;

    // Print CEP Greeting
    print_greeting();

    //************************************************************************
    // Run the AES Tests
    //************************************************************************
    printf("\r\n");
    printf("AES: *********************************************************\r\n");
    printf("AES: * AES test started ...                                  *\r\n");
    printf("AES: *********************************************************\r\n");
    printf("\r\n");    

    // AES-192 Core Test Vectors  
    uint32_t pt1[4]  = {0x3243f6a8, 0x885a308d, 0x313198a2, 0xe0370734};
    uint32_t key1[6] = {0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c, 0x2b7e1516, 0x28aed2a6};
    aes_setPlaintext(pt1);
    aes_setKey(key1);
    aes_start();
    aes_waitForValidOutput();
    aes_reportCiphertext();
    aes_verifyCiphertext("4fcb8db85784a2c1bb77db7ede3217ac", "test 1");

    uint32_t pt2[4]  = {0x00112233, 0x44556677, 0x8899aabb, 0xccddeeff};
    uint32_t key2[6] = {0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f, 0x00010203, 0x04050607};
    aes_setPlaintext(pt2);
    aes_setKey(key2);
    aes_start();
    aes_waitForValidOutput();
    aes_reportCiphertext();
    aes_verifyCiphertext("65d50128b115a7780981475a6bd64a0e", "test 2");
       
    uint32_t pt3[4]  = {0, 0, 0, 0};
    uint32_t key3[6] = {0, 0, 0, 0, 0, 0};
    aes_setPlaintext(pt3);
    aes_setKey(key3);
    aes_start();
    aes_waitForValidOutput();
    aes_reportCiphertext();
    aes_verifyCiphertext("aae06992acbf52a3e8f4a96ec9300bd7", "test 3");
    
    uint32_t pt4[4]  = {0, 0, 0, 0};
    uint32_t key4[6] = {0, 0, 0, 0, 0, 1};
    aes_setPlaintext(pt4);
    aes_setKey(key4);
    aes_start();
    aes_waitForValidOutput();
    aes_reportCiphertext();
    aes_verifyCiphertext("8bae4efb70d33a9792eea9be70889d72", "test 4");
    
    uint32_t pt5[4]  = {0, 0, 0, 1};
    uint32_t key5[6] = {0, 0, 0, 0, 0, 0};
    aes_setPlaintext(pt5);
    aes_setKey(key5);
    aes_start();
    aes_waitForValidOutput();
    aes_reportCiphertext();
    aes_verifyCiphertext("cd33b28ac773f74ba00ed1f312572435", "test 5");

    printf("\r\n");
    printf("AES: *********************************************************\r\n");
    printf("AES: * AES test done ...                                     *\r\n");
    printf("AES: *********************************************************\r\n");
    printf("\r\n");    

    //************************************************************************
    // Run the Triple-DES tests
    //************************************************************************
    printf("\r\n");
    printf("DES3: *********************************************************\r\n");
    printf("DES3: * Triple-DES test started ...                           *\r\n");
    printf("DES3: *********************************************************\r\n");
    printf("\r\n");    

    int decrypt=0, select=0;
    uint32_t ct[DES3_BLOCK_WORDS];

    const uint64_t x[10][10]={
    //       key1                key2                key3             Test data           Out data
    {0x0101010101010101, 0x0101010101010101, 0x0101010101010101, 0x95F8A5E5DD31D900, 0x8000000000000000},
    {0x0101010101010101, 0x0101010101010101, 0x0101010101010101, 0x9D64555A9A10B852, 0x0000001000000000},
    {0x3849674C2602319E, 0x3849674C2602319E, 0x3849674C2602319E, 0x51454B582DDF440A, 0x7178876E01F19B2A},
    {0x04B915BA43FEB5B6, 0x04B915BA43FEB5B6, 0x04B915BA43FEB5B6, 0x42FD443059577FA2, 0xAF37FB421F8C4095},
    {0x0123456789ABCDEF, 0x0123456789ABCDEF, 0x0123456789ABCDEF, 0x736F6D6564617461, 0x3D124FE2198BA318},
    {0x0123456789ABCDEF, 0x5555555555555555, 0x0123456789ABCDEF, 0x736F6D6564617461, 0xFBABA1FF9D05E9B1},
    {0x0123456789ABCDEF, 0x5555555555555555, 0xFEDCBA9876543210, 0x736F6D6564617461, 0x18d748e563620572},
    {0x0352020767208217, 0x8602876659082198, 0x64056ABDFEA93457, 0x7371756967676C65, 0xc07d2a0fa566fa30},
    {0x0101010101010101, 0x8001010101010101, 0x0101010101010102, 0x0000000000000000, 0xe6e6dd5b7e722974},
    {0x1046103489988020, 0x9107D01589190101, 0x19079210981A0101, 0x0000000000000000, 0xe1ef62c332fe825b}};

    for(decrypt=0;decrypt<2;decrypt=decrypt+1){
        des3_writeToDecrypt(decrypt);
        if(decrypt) printf("DES3: Running Encrypt test ...\r\n\r\n");
        else        printf("DES3: Running Decrypt test ...\r\n\r\n");

        for(select=0;select<10;select=select+1){
            uint32_t key[6]={(uint32_t)(x[select][0]>>32), (uint32_t)x[select][0], (uint32_t)(x[select][1]>>32), (uint32_t)x[select][1], (uint32_t)(x[select][2]>>32), (uint32_t)x[select][2]};
            des3_setKey(key);
            uint32_t pt[2]={(uint32_t)(x[select][3+decrypt]>>32), (uint32_t)x[select][3+decrypt]};
            des3_setPlaintext(pt);
        
            des3_start();
            des3_waitForValidOutput();
            des3_saveCiphertext(ct);

            success=success&des3_assertEquals(select, x[select][4-decrypt]>>32, x[select][4-decrypt], ct[1], ct[0]);
        }
    }

    printf("\r\n");
    des3_assertSuccess(success);
    printf("\r\n");
    printf("\r\n");
    printf("DES3: *********************************************************\r\n");
    printf("DES3: * Triple-DES test started ...                           *\r\n");
    printf("DES3: *********************************************************\r\n");
    printf("\r\n");    

    //************************************************************************
    // Run the DFT tests
    //************************************************************************
    uint16_t dft_dout[4]={0x0000, 0x0000, 0x0000, 0x0000};
    
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
            case 0: dft_setX(j, j*4, j*4+1, j*4+2, j*4+3); break;// Input: 2 complex words per cycle
            case 1: dft_setX(j, 128+j*4, 128+j*4+1, 128+j*4+2, 128+j*4+3); break;// Input 4 words per cycle
            }
        }
       
        dft_start();

        //Check Outputs
        dft_waitForValidOutput();

        printf("DFT: --- begin output %02d---\r\n", i+1);
        for(j=0;j<32;j++){
            dft_getY(j, &dft_dout[0], &dft_dout[1], &dft_dout[2], &dft_dout[3]);
            switch(i){
            case 0: success=success&dft_assertEquals(j, dft_exp_1[j], dft_dout); break;
            case 1: success=success&dft_assertEquals(j, dft_exp_2[j], dft_dout); break;
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

    //************************************************************************
    // Run the IDFT tests
    //************************************************************************
    uint16_t idft_dout[4]={0x0000, 0x0000, 0x0000, 0x0000};
    
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
            idft_getY(j, &idft_dout[0], &idft_dout[1], &idft_dout[2], &idft_dout[3]);
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


    //************************************************************************
    // Run the FIR tests
    //************************************************************************
    uint32_t fir_dout=0x00000000;
    success=true;

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

    //************************************************************************
    // Run the IIR tests
    //************************************************************************
    uint32_t iir_dout=0x00000000;
    success=true;
    
    printf("\r\n");
    printf("IIR: *********************************************************\r\n");
    printf("IIR: * IIR test started ...                                  *\r\n");
    printf("IIR: *********************************************************\r\n");
    printf("\r\n");

    //Set Inputs and check Outputs
    for (i=0; i < 32; i++)
        iir_setInData(i, i*4);
    
    iir_start();
    iir_waitForValidOutput();
       
    for (i=0; i < 32; i++){
        iir_dout=iir_getOutData(i);
        success=success&iir_assertEquals(i, iir_exp_1[i], iir_dout);
    }

    printf("\r\n");
    iir_assertSuccess(success);

    printf("\r\n");
    printf("IIR: *********************************************************\r\n");
    printf("IIR: * IIR test done ...                                     *\r\n");
    printf("IIR: *********************************************************\r\n");
    printf("\r\n");

    //************************************************************************
    // Run the GPS tests
    //************************************************************************
    printf("\r\n");
    printf("GPS: *********************************************************\r\n");
    printf("GPS: * GPS test started ...                                  *\r\n");
    printf("GPS: *********************************************************\r\n");
    printf("\r\n");

    gps_generateNextCode();
    gps_waitForValidOutput();
    gps_verifyCode("00001F43", "C/A code", GPS_CA_BYTES, GPS_CA_BASE);
    gps_verifyCode("9DDFDD9CE127D8D95394BF2838E7EF54", "P code", GPS_P_BYTES, GPS_P_BASE);
    gps_verifyCode("FD86C6D34155A750E284B05AC643BC27", "L code", GPS_L_BYTES, GPS_L_BASE);
    
    printf("\r\n");
    printf("GPS: *********************************************************\r\n");
    printf("GPS: * GPS test done ...                                     *\r\n");
    printf("GPS: *********************************************************\r\n");
    printf("\r\n");
    
    //************************************************************************
    // Run the MD5 tests
    //************************************************************************
    printf("\r\n");
    printf("MD5: *********************************************************\r\n");
    printf("MD5: * MD5 test started ...                                  *\r\n");
    printf("MD5: *********************************************************\r\n");
    printf("\r\n");

    unsigned char md5_hash[MD5_HASH_BYTES];

    printf("MD5: Resetting the MD5 module...\n");
    md5_resetAndReady();
    printf("MD5: Reset complete\n");
  
    md5_hashString("a", md5_hash);
    md5_compareHash(md5_hash, "0cc175b9c0f1b6a831c399e269772661", "single character");
    
    md5_hashString("abc", md5_hash);
    md5_compareHash(md5_hash, "900150983cd24fb0d6963f7d28e17f72", "three character");
    
    // Test a message that requires an extra chunk to be added to hold padding
    md5_hashString("Lincoln LaboratoLincoln LaboratoLincoln LaboratoLincoln Laborato", md5_hash);
    md5_compareHash(md5_hash, "93f3763d2c20d33e5031e20480eaca58", "512-bit sized message");

    printf("\r\n");
    printf("MD5: *********************************************************\r\n");
    printf("MD5: * MD5 test done ...                                     *\r\n");
    printf("MD5: *********************************************************\r\n");
    printf("\r\n");

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
    
    // Test common case 1
    printf("\r\n");
    sha256_hashString("", sha256_hash);
    sha256_compareHash(sha256_hash, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855", "empty string");
    
    // Test common case 2
    printf("\r\n");
    sha256_hashString("a", sha256_hash);
    sha256_compareHash(sha256_hash, "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb", "single character");
    
    // Test common case 3
    printf("\r\n");
    sha256_hashString("abc", sha256_hash);
    sha256_compareHash(sha256_hash, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "three character");
    
    // Test a message that requires an extra chunk to be added to hold padding
    printf("\r\n");
    sha256_hashString("Lincoln LaboratoLincoln LaboratoLincoln LaboratoLincoln Laborato", sha256_hash);
    sha256_compareHash(sha256_hash, "f88c49e2b696d45a699eb10effafb3c9522df6f7fa68c2509d105e849be605ba", "512-bit message");
 
    printf("\r\n");
    printf("SHA256: *********************************************************\r\n");
    printf("SHA256: * SHA256 test done ...                                  *\r\n");
    printf("SHA256: *********************************************************\r\n");
    printf("\r\n");

     //Check Outputs
    return 0;

}   // end of main
//************************************************************************
// END: Main function
//************************************************************************

