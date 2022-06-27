//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cep_aes.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    aes test for CEP
// Notes:          
//************************************************************************

#if defined(BARE_MODE)
#else
#include <cryptopp/cryptlib.h>
#include <cryptopp/aes.h>
#include <cryptopp/rijndael.h>
#include <cryptopp/modes.h>

#include "simPio.h"

#endif

#include <string.h>
#include "cep_aes.h"
#include "simdiag_global.h"
#include "portable_io.h"
#include "CEP.h"
#include "random48.h"
//
//

cep_aes::cep_aes(int coreIndex, int seed, int verbose) {
  init(coreIndex);
  //
  SetKeySize(192/8);
  SetBlockSize(128/8);
  SetSeed(seed);
  SetVerbose(verbose);
}
//

//
// via cryptopp
//
int cep_aes::cryptopp_aes192_ecb_encryption
(
 uint8_t *input,           // input text packet
 uint8_t *output,           // output cipher packet
 int verbose)
{
  //
#if defined(BARE_MODE)
#else

  using namespace CryptoPP;

  ECB_Mode< AES >::Encryption encryption;
  encryption.SetKey(&(mKEY[0]), GetKeySize());
  encryption.ProcessLastBlock(output, input, GetBlockSize());
 
#endif
  //
  return mErrCnt;
}

// Key[2,1:0] (64-bits) = mKEY[0....23]
void cep_aes::LoadKey(void) {
  uint64_t word;
#ifdef BIG_ENDIAN
  for(int i = 0; i < mKeySize/8; i++) { //  8-bytes/word
    word = 0;
    for (int j=0;j<8;j++) {
      word = (word << 8) | (uint64_t)mKEY[i*8 + j];      
    }
    cep_writeNcapture(AES_KEY_BASE + (i * BYTES_PER_WORD), word);
  }
#else
  for(int i = 0; i < mKeySize/8; i++) { //  8-bytes/word
    word = 0;
    for (int j=0;j<8;j++) {
      word = (word << 8) | (uint64_t)mKEY[i*8 + j];      
    }
    cep_writeNcapture(AES_KEY_BASE + (((AES_KEY_WORDS - 1) - i) * BYTES_PER_WORD), word);
  }
#endif
}

// State[1:0] (64-bits/unit) = mHwPt[0....23]
void cep_aes::LoadPlaintext(void) {
  uint64_t word;
#ifdef BIG_ENDIAN
  for(int i = 0; i < mBlockSize/8; i++) { //  8-bytes/word
    word = 0;
    for (int j=0;j<8;j++) {
      word = (word << 8) | (uint64_t)mHwPt[i*8 + j];      
    }
    cep_writeNcapture(AES_PT_BASE + (i* BYTES_PER_WORD), word);
  }  
#else
  for(int i = 0; i < mBlockSize/8; i++) { //  8-bytes/word
    word = 0;
    for (int j=0;j<8;j++) {
      word = (word << 8) | (uint64_t)mHwPt[i*8 + j];      
    }
    cep_writeNcapture(AES_PT_BASE + (((AES_BLOCK_WORDS - 1) - i) * BYTES_PER_WORD), word);
  }
#endif
  
}

void cep_aes::StartEncrypt(void) {
  cep_writeNcapture(AES_START, 0x1);
  cep_writeNcapture(AES_START, 0x0);
}

int cep_aes::waitTilDone(int maxTO) {
#if 1
  if (GetVerbose(2)) {  LOGI("%s\n",__FUNCTION__); }    
  return cep_readNspin(AES_DONE, 2, maxTO);
#else
  while (maxTO > 0) {
    if (cep_readNcapture(AES_DONE)) break;
    maxTO--;
  };
  return (maxTO <= 0) ? 1 : 0;
#endif
}

void cep_aes::ReadCiphertext(void) {
  uint64_t word;
#ifdef BIG_ENDIAN
  for(int i = 0; i < mBlockSize/8; i++) { //  8-bytes/word
    word = cep_readNcapture(AES_CT_BASE +(i * BYTES_PER_WORD));
    for (int j=0;j<8;j++) {
      mHwCp[i*8 +j]= (word >> (8*(7-j)) ) & 0xff;
    }
  }  
#else
  for(int i = 0; i < mBlockSize/8; i++) { //  8-bytes/word
    word = cep_readNcapture(AES_CT_BASE +(((AES_BLOCK_WORDS - 1) - i) * BYTES_PER_WORD));
    for (int j=0;j<8;j++) {
      mHwCp[i*8 +j]= (word >> (8*(7-j)) ) & 0xff;
    }
  }
#endif
}

int cep_aes::RunAes192Test(int maxLoop) {
  /*
  uint8_t key0[] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6, 0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c, 0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6};
  uint8_t pt0[]  = {0x32,0x43,0xf6,0xa8,0x88,0x5a,0x30,0x8d, 0x31,0x31,0x98,0xa2,0xe0,0x37,0x07,0x34};
  */
  int     outLen=mBlockSize;
  //

  // Run the main test loop
  for (int i=0;i<maxLoop;i++) {
    if (GetVerbose()) {
      LOGI("%s: Loop %d\n",__FUNCTION__,i);
    }
    //
    // cover every byte of key and plain text
    //
    if (i < 256) {
      for (int j=0;j<GetKeySize();j++) {
        mKEY[j] = (i&1) ? ((i+j) & 0xFF) : (~(i+j) & 0xFF);
      }
      for (int j=0;j<GetBlockSize();j++) {
        mHwPt[j] = (i&1) ? (~(i+GetKeySize()+j) & 0xFF) : ((i+GetKeySize()+j) & 0xFF);
      }
    } else {
      // just randomize
      // random
      RandomGen(mKEY, GetKeySize());
      RandomGen(mHwPt, GetBlockSize());      
    }
    
    /* quick test to compare a known source
       strncpy((char *)mKEY,(char *)key0,24);
       strncpy((char *)mHwPt,(char *)pt0,16);
    */
    
    // ----------------------------------------------------------------------------
    
    //
    // create expected cipherTet
    mErrCnt += cep_aes::cryptopp_aes192_ecb_encryption
      (mHwPt,           // uint8_t *input,           // input text packet
       mSwCp,           // uint8_t *output,           // output cipher packet
       GetVerbose());
    
      //
    // load key&plaintext to HW
    //
    LoadPlaintext();
    LoadKey();
    StartEncrypt();
    mErrCnt += waitTilDone(50);
    if (!mErrCnt) {
      ReadCiphertext();
      mErrCnt += CheckCipherText();
    }
    if (mErrCnt && !GetExpErr()) {
      LOGE("%s: Loop %d with %d error\n",__FUNCTION__,i,mErrCnt);      
    }
    //
    // Print
    //
    if ((mErrCnt && !GetExpErr()) || GetVerbose(2)) {
      PrintMe("Key",       &(mKEY[0]),mKeySize);          
      PrintMe("PlainText",&(mHwPt[0]),mBlockSize);
      PrintMe("ExpCipher",&(mSwCp[0]),mBlockSize);
      PrintMe("ActCipher",&(mHwCp[0]),mBlockSize);        
    }
    MarkSingle(i);
    if (mErrCnt) break;
  }
  return mErrCnt;
}

