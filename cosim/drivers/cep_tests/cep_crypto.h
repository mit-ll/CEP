//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
#ifndef cep_crypto_H
#define cep_crypto_H
// ---------------------------
#include "stdint.h"
#include "stdio.h"

#define MAX_MSG_SIZE        2048
#define MAX_KEY_SIZE        2048

//
// CEP's Crypto
//
class cep_crypto { 
  //
  // public stuffs
  //
public: //
  // constructors must be default to avoid unwind_Resume problem in bare mode
  cep_crypto() {};
  ~cep_crypto() {};  
  
  //
  void   SetVerbose      (int val) { mVerbose=val; }  
  int    GetVerbose (void) { return mVerbose; }    
  int    GetVerbose (int maskLevel) { return mVerbose & maskLevel; }    
  
  //
  void   SetSeed (int seed);
  int    GetSeed ( ) { return mSeed; }  

  void   SetExpErr (int val) { mExpErr = val; }
  int    GetExpErr ( ) { return mExpErr; }  
  // Access
  // in bytes 16,24,32 of 128/192/256bits
  void   SetKeySize (int kSize) { mKeySize=kSize; }  
  int    GetKeySize (void) { return mKeySize; }
  //
  void   SetBlockSize (int bSize) { mBlockSize=bSize; }  
  int    GetBlockSize (void) { return mBlockSize; }

  void   SetMsgSize (int bSize) { mMsgSize=bSize; }  
  int    GetMsgSize (void) { return mMsgSize; }    
  //
  int CheckCipherText(void);     // for decryt
  int CheckPlainText(void);  // for encryt
  //
  void RandomGen(uint8_t *buf, int size);
  void PrintMe(const char *name, uint8_t *buf, int size);
  void PrintMe(const char *name, double *buf1, double *buf2, int size);  
  //
  // To support capturing for playback in BAREMetal mode and Unit-testbench
  //
  void SetSrotFlag(int mode) { mSrotFlag = mode; } // no write to file
  void SetCaptureMode(int mode, const char *path, const char *testName);  
  void cep_writeNcapture(int device, uint32_t pAddress, uint64_t pData);
  uint64_t cep_readNcapture(int device, uint32_t pAddress);
  int cep_readNspin(int device, uint32_t pAddress,uint64_t pData, uint64_t mask, int timeOut);
  int cep_readNspin(int device, uint32_t pAddress,uint64_t pData, int timeOut);
  void MarkSingle(int loop);
  //
  void init(void);
  void freeMe(void);
  //
  
  //
  // Protected / Private cep_crypto items
  //
 protected:

  int mSeed;
  int mVerbose;
  int mKeySize;
  int mBlockSize;
  int mMsgSize;
  //
  uint8_t mKEY[MAX_KEY_SIZE];   // upto 256-bits AES key
  uint8_t mIV[MAX_KEY_SIZE];   // DOnt care
  uint8_t mHwPt[MAX_MSG_SIZE];   // from HW
  uint8_t mSwPt[MAX_MSG_SIZE];   // expected  
  uint8_t mHwCp[MAX_MSG_SIZE];   // from HW
  uint8_t mSwCp[MAX_MSG_SIZE];   // expected Cipher
  //
  // To support capture and playback (bareMetal & unit testbench)
  //
  int mCapture;
  int mSrotFlag;
  int mC2C_Capture;
  char mTestName[128];
  FILE *mFd;
  int mCount;
  int mSingle;
  int mWordCnt;
  int mErrCnt;
  int mExpErr; // expect error
  //
  uint64_t mAdrBase;
  uint64_t mAdrSize;

}; // class cep_crypto
#endif

