//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cep_sha256.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    sha256 test for CEP
// Notes:          
//************************************************************************

#if defined(BARE_MODE)
#else
#include <openssl/sha.h>
#include "simPio.h"

#endif

#include <string.h>
#include "cep_sha256.h"
#include "simdiag_global.h"
#include "portable_io.h"
#include "CEP.h"
#include "cepregression.h"
#include "random48.h"
//
//

cep_sha256::cep_sha256(int seed, int verbose) {
  init();
  
  SetBlockSize(64); // 64bytes bytes
  SetSeed(seed);
  SetVerbose(verbose);
}
//
cep_sha256::~cep_sha256()  {
}

//
// via openssl
//
int cep_sha256::prepare_sha256_key_N_text
(
 uint8_t *input,           // input text
 uint8_t *output,           // output packet
 int padding_enable,
 int length,
 int *outlen,
 int verbose)
{
  //
#if defined(BARE_MODE)
#else
  SHA256_CTX ctx;
  //
  SHA256_Init(&ctx);
  SHA256_Update(&ctx,input, length);
  SHA256_Final(output,&ctx);
  *outlen = 64; // 64bytes
#endif
  //
  return mErrCnt;
}

// State[1:0] (64-bits/unit) = mHwPt[0....23]
void cep_sha256::LoadInText(uint8_t *buf) {
  uint64_t word;
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }  
#ifdef BIG_ENDIAN
  for(int i = 0; i < GetBlockSize()/8; i++) { //  8-bytes/word
    word = 0;
    for (int j=0;j<8;j++) {
      word = (word << 8) | (uint64_t)buf[i*8 + j];      
    }
    cep_writeNcapture(SHA256_BASE_K, SHA256_MSG_BASE + (i * BYTES_PER_WORD), word);
  }  
#else
  for(int i = 0; i < GetBlockSize()/8; i++) { //  8-bytes/word
    word = 0;
    for (int j=0;j<8;j++) {
      word = (word << 8) | (uint64_t)buf[i*8 + j];      
    }
    cep_writeNcapture(SHA256_BASE_K, SHA256_MSG_BASE + ((7-i) * BYTES_PER_WORD), word);
  }
#endif
}

void cep_sha256::Start(int firstTime) {
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }    
  if (firstTime) {
    cep_writeNcapture(SHA256_BASE_K, SHA256_NEXT_INIT, 0x0000000000000002);
  } else {
    cep_writeNcapture(SHA256_BASE_K, SHA256_NEXT_INIT, 0x0000000000000004);    
  }
  cep_writeNcapture(SHA256_BASE_K, SHA256_NEXT_INIT, 0x0000000000000000);    
}

int cep_sha256::waitTilReady(int maxTO) {
#if 1
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }    
  return cep_readNspin(SHA256_BASE_K, SHA256_READY, 1, maxTO);
#else
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }    
  while (maxTO > 0) {
    if (cep_readNcapture(SHA256_BASE_K, SHA256_READY)) break;
    maxTO--;
  };
  return (maxTO <= 0) ? 1 : 0;
#endif
}

int cep_sha256::waitTilDone(int maxTO) {
#if 1
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }    
  return cep_readNspin(SHA256_BASE_K, SHA256_HASH_DONE, 1, maxTO);
#else
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }    
  while (maxTO > 0) {
    if (cep_readNcapture(SHA256_BASE_K, SHA256_HASH_DONE)) break;
    maxTO--;
  };
  return (maxTO <= 0) ? 1 : 0;
#endif
}

void cep_sha256::ReadOutText(void) {
  uint64_t word;
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }
#ifdef BIG_ENDIAN
  for(int i = 0; i < 4; i++) { //  256-bits=32
    word = cep_readNcapture(SHA256_BASE_K, SHA256_HASH_BASE + (i*8));
    for (int j=0;j<8;j++) {
      mHwCp[i*8 +j]= (word >> (8*(7-j)) ) & 0xff;
    }
  }  
#else
  for(int i = 0; i < 4; i++) { //  256-bits=32
    word = cep_readNcapture(SHA256_BASE_K, SHA256_HASH_BASE + (8*(3-i)));
    for (int j=0;j<8;j++) {
      mHwCp[i*8 +j]= (word >> (8*(7-j)) ) & 0xff;
    }
  }
#endif
}

int cep_sha256::CheckHashText(uint8_t *buf) {
  for (int i=0;i<GetBlockSize()/2;i++) {
    if (mHwCp[i] != buf[i]) mErrCnt++;
  }
  return mErrCnt;
}
int cep_sha256::RunSha256Test(int maxLoop) {
  int outLen=32;
  //
  for (int i=0;i<maxLoop;i++) {
    if (GetVerbose()) {
      LOGI("%s: Loop %d\n",__FUNCTION__,i);
    }
    SetMsgSize((i*32)+i);
    //
    // cover every byte of key and plain text
    //
    if (i < 8) {
      for (int j=0;j<GetMsgSize();j++) {
	mHwPt[j] = (i&0x1) ? ((i+j) & 0xFF) : (~(i+j) & 0xFF) ;
      }
    } else {
      // just randomize
      RandomGen(mHwPt, GetMsgSize());
    }
    // Add pading of 0x80 folow by 0 and 8-bytes of len
    int x = GetMsgSize();
    int padded = (GetMsgSize()+8)%64;
    if (padded == 0) {  // ad whole block
      padded = 64;
    } else {
      padded = 64 - padded;
    }
    // now do the padding
    for (int j=0;j<padded;j++) {
      mHwPt[x++] = (j==0) ? 0x80 : 0;
    }
    // add len: 8 bytes
    int totalLen = GetMsgSize()+padded+8;
    uint64_t lenInBits = (uint64_t)GetMsgSize() << 3; // to bits
    for (int j=0;j<8;j++) {
      mHwPt[x++] = lenInBits >> ((7-j)*8);
    }
    if (GetVerbose()) {
      LOGI("msgSize=%d padded=%d total=%d\n",GetMsgSize(),padded,totalLen);
    }
    //
    // create expected hash
    //
    mErrCnt += prepare_sha256_key_N_text
      (mHwPt, // uint8_t *input,           // input text packet
       mSwCp, // uint8_t *output,           // output cipher packet
       0,   // int padding_enable,
       GetMsgSize(), // int length,
       &outLen, GetVerbose());	
    //
    int numBlocks = totalLen/64;
    for (int b=0;b<numBlocks;b++) {
      //
      // load key&plaintext to HW
      //
      mErrCnt += waitTilReady(100);
      LoadInText(&mHwPt[b*64]);
      Start(b==0);
      mErrCnt += waitTilDone(100);
      // read last block for hash
      if ((!mErrCnt) && (b==(numBlocks-1))) {
	ReadOutText();
	mErrCnt += CheckHashText(mSwCp);
      }
    }
    if (mErrCnt) {
      LOGE("%s: Loop %d len=%d with %d error\n",__FUNCTION__,i,GetMsgSize(),mErrCnt);      
    }
    //
    // Print
    //
    if (mErrCnt || GetVerbose()) {
      PrintMe("Key",       &(mKEY[0]),mKeySize);          
      PrintMe("InText     ",&(mHwPt[0]),totalLen);
      PrintMe("ExpOutText",&(mSwCp[0]),mBlockSize/2);
      PrintMe("ActOutText",&(mHwCp[0]),mBlockSize/2);        
    }
    MarkSingle(i);
    if (mErrCnt) break;        
  }
  return mErrCnt;
}

