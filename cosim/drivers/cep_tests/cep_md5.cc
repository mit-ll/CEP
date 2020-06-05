//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      cep_md5.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    md5 test for CEP
// Notes:          
//************************************************************************

#if defined(BARE_MODE)
#else
#include <openssl/md5.h>
#include "simPio.h"

#endif

#include <string.h>
#include "cep_md5.h"
#include "simdiag_global.h"
#include "portable_io.h"
#include "CEP.h"
#include "cepregression.h"
#include "random48.h"
//
//

cep_md5::cep_md5(int seed, int verbose) {
  init();  
  SetBlockSize(64); // 64bytes bytes
  SetSeed(seed);
  SetVerbose(verbose);
}
//
cep_md5::~cep_md5()  {
}

//
// via openssl
//
int cep_md5::prepare_md5_key_N_text
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
  MD5_CTX ctx;
  //
  MD5_Init(&ctx);
  MD5_Update(&ctx,input, length);
  MD5_Final(output,&ctx);
  *outlen = 64; // 64bytes
#endif
  //
  return mErrCnt;
}

// State[1:0] (64-bits/unit) = mHwPt[0....23]
void cep_md5::LoadInText(uint8_t *buf) {
  uint64_t word;
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }  
#ifdef BIG_ENDIAN
  // 
  for(int i = 0; i < GetBlockSize()/8; i++) { //  8-bytes/word
    word = 0;
    for (int j=0;j<8;j++) {
      word = (word << 8) | (uint64_t)buf[i*8 + j];      
    }
    cep_writeNcapture(MD5_BASE_K, MD5_MSG_BASE + (i * BYTES_PER_WORD), word);
  }
#else
  // NOT the same as SHA256, it is byte flipped
  for(int i = 0; i < GetBlockSize()/8; i++) { //  8-bytes/word
    word = 0;
    // byte flipped!!
    for (int j=0;j<8;j++) {
      word = (word << 8) | (uint64_t)buf[i*8 + (7-j)];      
    }
    cep_writeNcapture(MD5_BASE_K, MD5_MSG_BASE + (i * BYTES_PER_WORD), word);
  }
#endif
}

void cep_md5::Clear(void) {
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }  
  cep_writeNcapture(MD5_BASE_K, MD5_RST, 0x2); // bit[1]=init , bit[0]=rst
  cep_writeNcapture(MD5_BASE_K, MD5_RST, 0x0);
  waitTilReady(100);    
}

void cep_md5::Start(void) {
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }  
  cep_writeNcapture(MD5_BASE_K, MD5_MSG_IN_VALID, 0x1);
  cep_writeNcapture(MD5_BASE_K, MD5_MSG_IN_VALID, 0x0);
}

int cep_md5::waitTilReady(int maxTO) {
#if 1
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }    
  return cep_readNspin(MD5_BASE_K, MD5_READY, 1, maxTO);
#else  
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }  
  while (maxTO > 0) {
    if (cep_readNcapture(MD5_BASE_K, MD5_READY)) break;
    maxTO--;
  };
  return (maxTO <= 0) ? 1 : 0;
#endif
}

int cep_md5::waitTilDone(int maxTO) {
#if 1
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }    
  return cep_readNspin(MD5_BASE_K, MD5_MSG_OUT_VALID, 1, maxTO);
#else
  while (maxTO > 0) {
    if (cep_readNcapture(MD5_BASE_K, MD5_MSG_OUT_VALID)) break;
    maxTO--;
  };
  return (maxTO <= 0) ? 1 : 0;
#endif
}

void cep_md5::ReadOutText(void) {
  uint64_t word;
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }  
#ifdef BIG_ENDIAN
  for(int i = 0; i < 2; i++) { //  128bits
    word = cep_readNcapture(MD5_BASE_K, MD5_HASH_BASE + (i*8));
    for (int j=0;j<8;j++) {
      mHwCp[i*8 +j]= (word >> (8*(7-j)) ) & 0xff;
    }
  }  
#else  
  for(int i = 0; i < 2; i++) { //  128bits
    word = cep_readNcapture(MD5_BASE_K, MD5_HASH_BASE + (8*(1-i)));
    for (int j=0;j<8;j++) {
      mHwCp[i*8 +j]= (word >> (8*(7-j)) ) & 0xff;
    }
  }
#endif
}

int cep_md5::CheckHashText(uint8_t *buf) {
  for (int i=0;i<GetBlockSize()/4;i++) {
    if (mHwCp[i] != buf[i]) mErrCnt++;
  }
  return mErrCnt;
}
int cep_md5::RunMd5Test(int maxLoop) {
  int outLen=32;
  //
  for (int i=0;i<maxLoop;i++) {
    if (GetVerbose()) { LOGI("%s: Loop %d\n",__FUNCTION__,i); }
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
#if 0
    // test
    for (int z=0;z<26;z++) {
      mHwPt[z] = 0x61+z; // 'a'
    }
    SetMsgSize(26);
#endif
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
    // FIXME!!! the other endian
    for (int j=0;j<8;j++) {
      //      mHwPt[x++] = lenInBits >> ((7-j)*8);
      mHwPt[x++] = lenInBits >> ((j)*8);      
    }
    if (GetVerbose()) {
      LOGI("msgSize=%d padded=%d total=%d\n",GetMsgSize(),padded,totalLen);
    }
    //
    // create expected hash
    //
    mErrCnt += prepare_md5_key_N_text
      (mHwPt, // uint8_t *input,           // input text packet
       mSwCp, // uint8_t *output,           // output cipher packet
       0,   // int padding_enable,
       GetMsgSize(), // int length,
       &outLen, GetVerbose());	
    //
    int numBlocks = totalLen/64;
    Clear();
    for (int b=0;b<numBlocks;b++) {
      //
      // load key&plaintext to HW
      //
      mErrCnt += waitTilReady(100);
      LoadInText(&mHwPt[b*64]);
      Start();
      //mErrCnt += waitTilDone(100);
      mErrCnt += waitTilReady(100);      
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
      //PrintMe("Key",       &(mKEY[0]),mKeySize);          
      PrintMe("InText     ",&(mHwPt[0]),totalLen);
      PrintMe("ExpOutText",&(mSwCp[0]),mBlockSize/4);
      PrintMe("ActOutText",&(mHwCp[0]),mBlockSize/4);        
    }
    MarkSingle(i);    
    if (mErrCnt) break;        
  }
  return mErrCnt;
}

