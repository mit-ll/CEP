//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cep_gps.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    gps test for CEP
// Notes:          
//************************************************************************

#if defined(BARE_MODE)
#else
#include <openssl/evp.h>

#include "simPio.h"

#endif

#include <string.h>
#include "cep_gps.h"
#include "simdiag_global.h"
#include "portable_io.h"
#include "CEP.h"
#include "cepregression.h"
#include "random48.h"
//
//
cep_gps::cep_gps(int seed, int verbose) : cep_aes(seed,verbose) {
  init();  
  SetSvNum(0);
  for (int i=0;i<11;i++) {
    g1[i] = 1;
    g2[i] = 1;
  }
  for (int i=0;i<192/8;i++) {
    mKEY[i] = 0xAA;
  }
}
//

void cep_gps::SetSvNum (int svNum) {
  if (svNum > MAX_SAT) {
    mSvNum= MIN_SAT;
  } else {
    mSvNum= svNum;
  }
  // Load SvNum
  cep_writeNcapture(GPS_BASE_K, GPS_SV_NUM, GetSvNum());
  //
}


void cep_gps::Start(void) {
  //
  cep_writeNcapture(GPS_BASE_K, GPS_GEN_NEXT, 0x1);
  cep_writeNcapture(GPS_BASE_K, GPS_GEN_NEXT, 0x0);
}

void cep_gps::BusReset(void) {
  // need to extend at least for slow clock can catch      
  for (int i=0;i<5;i++) {
    cep_writeNcapture(GPS_BASE_K, GPS_RESET, 0x01);
  }
  cep_writeNcapture(GPS_BASE_K, GPS_RESET, 0x00);
  //
}

int cep_gps::ReadNCheck_CA_Code(int mask)
{
  int expCACode= GetCA_code(GetSvNum());
  int actCACode = cep_readNcapture(GPS_BASE_K, GPS_CA_BASE);
  mErrCnt += ((expCACode ^ actCACode) & mask);
  //
  if (mErrCnt) {
    LOGE("ERROR: CA_Code mismatch svNum=%d exp=0x%04x act=0x%04x\n",GetSvNum(),expCACode,actCACode);
  } else if (GetVerbose()) {
    LOGI("CA_Code is OK svNum=%d exp=0x%04x act=0x%04x\n",GetSvNum(),expCACode,actCACode);
  }
  return mErrCnt;

}

int cep_gps::waitTilDone(int maxTO) {
#if 1
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }    
  return cep_readNspin(GPS_BASE_K, GPS_GEN_DONE, 2, 2, maxTO);  
#else
  while (maxTO > 0) {
    if (cep_readNcapture(GPS_BASE_K, GPS_GEN_DONE)) break;
    maxTO--;
  };
  return (maxTO <= 0) ? 1 : 0;
#endif
}

// 16 bytes = 2 words
// read P-code and use it as plainText for AES
void cep_gps::Read_PCode(void) {
  uint64_t word;
#ifdef BIG_ENDIAN
  for(int i = 0; i < mBlockSize/8; i++) { //  8-bytes/word
    word = cep_readNcapture(GPS_BASE_K, GPS_P_BASE + i*8);
    for (int j=0;j<8;j++) {
      mHwPt[i*8 +j]= (word >> (8*(7-j)) ) & 0xff;
    }
  }  
#else
  for(int i = 0; i < mBlockSize/8; i++) { //  8-bytes/word
    word = cep_readNcapture(GPS_BASE_K, GPS_P_BASE + (1-i)*8);
    // As of 04/12/20: each 32 bits are swapped within 64-bit register (see gps.scala)
    for (int j=0;j<8;j++) {
      switch (j) {
      case 0 : mHwPt[i*8 +j]= (word >> (8*(3)) ) & 0xff; break;
      case 1 : mHwPt[i*8 +j]= (word >> (8*(2)) ) & 0xff; break;
      case 2 : mHwPt[i*8 +j]= (word >> (8*(1)) ) & 0xff; break;
      case 3 : mHwPt[i*8 +j]= (word >> (8*(0)) ) & 0xff; break;
      case 4 : mHwPt[i*8 +j]= (word >> (8*(7)) ) & 0xff; break;
      case 5 : mHwPt[i*8 +j]= (word >> (8*(6)) ) & 0xff; break;
      case 6 : mHwPt[i*8 +j]= (word >> (8*(5)) ) & 0xff; break;
      case 7 : mHwPt[i*8 +j]= (word >> (8*(4)) ) & 0xff; break;		
      }
    }
  }
#endif
}

void cep_gps::Read_LCode(void) {
  uint64_t word;
#ifdef BIG_ENDIAN
  for(int i = 0; i < mBlockSize/8; i++) { //  8-bytes/word
    word = cep_readNcapture(GPS_BASE_K, GPS_L_BASE + i*8);
    for (int j=0;j<8;j++) {
      mHwCp[i*8 +j]= (word >> (8*(7-j)) ) & 0xff;
    }
  }  
#else
  for(int i = 0; i < mBlockSize/8; i++) { //  8-bytes/word
    word = cep_readNcapture(GPS_BASE_K, GPS_L_BASE + (1-i)*8);
    // As of 04/12/20: each 32 bits are swapped within 64-bit register (see gps.scala)
    for (int j=0;j<8;j++) {
      switch (j) {
      case 0 : mHwCp[i*8 +j]= (word >> (8*(3)) ) & 0xff; break;
      case 1 : mHwCp[i*8 +j]= (word >> (8*(2)) ) & 0xff; break;
      case 2 : mHwCp[i*8 +j]= (word >> (8*(1)) ) & 0xff; break;
      case 3 : mHwCp[i*8 +j]= (word >> (8*(0)) ) & 0xff; break;
      case 4 : mHwCp[i*8 +j]= (word >> (8*(7)) ) & 0xff; break;
      case 5 : mHwCp[i*8 +j]= (word >> (8*(6)) ) & 0xff; break;
      case 6 : mHwCp[i*8 +j]= (word >> (8*(5)) ) & 0xff; break;
      case 7 : mHwCp[i*8 +j]= (word >> (8*(4)) ) & 0xff; break;		
      }
    }
  }
#endif
}

//
// Compute C/A code (use HW code for now)
//
int cep_gps::GetCA_code(int svNum)
{
  int CACode = 0;
  int chip=0;
  //
  for (int i=0;i<11;i++) {
    g1[i] = 1;
    g2[i] = 1;
  }
  //
  if (GetVerbose()) {
    LOGI("G1=0x%04x G2=0x%04x\n",
	 (((g1[10] & 0x1) << 9) |
	  ((g1[9] & 0x1) << 8) |	  
	  ((g1[8] & 0x1) << 7) |	  
	  ((g1[7] & 0x1) << 6) |	  
	  ((g1[6] & 0x1) << 5) |	  
	  ((g1[5] & 0x1) << 4) |	  
	  ((g1[4] & 0x1) << 3) |	  
	  ((g1[3] & 0x1) << 2) |	  
	  ((g1[2] & 0x1) << 1) |	  
	  ((g1[1] & 0x1) << 0)),
	 (((g2[10] & 0x1) << 9) |
	  ((g2[9] & 0x1) << 8) |	  
	  ((g2[8] & 0x1) << 7) |	  
	  ((g2[7] & 0x1) << 6) |	  
	  ((g2[6] & 0x1) << 5) |	  
	  ((g2[5] & 0x1) << 4) |	  
	  ((g2[4] & 0x1) << 3) |	  
	  ((g2[3] & 0x1) << 2) |	  
	  ((g2[2] & 0x1) << 1) |	  
	  ((g2[1] & 0x1) << 0)));
  }
  for (int i=0;i<13;i++) {
    chip = 0;
    switch (svNum) {
    case 1 :
      chip = g1[10] ^ g2[2] ^ g2[6]; break;
    case 2 :
      chip = g1[10] ^ g2[3] ^ g2[7]; break;
    case 3 :
      chip = g1[10] ^ g2[4] ^ g2[8]; break;
    case 4 :
      chip = g1[10] ^ g2[5] ^ g2[9]; break;
    case 5 :
      chip = g1[10] ^ g2[1] ^ g2[9]; break;
    case 6 :
      chip = g1[10] ^ g2[2] ^ g2[10]; break;
    case 7 :
      chip = g1[10] ^ g2[1] ^ g2[8]; break;
    case 8 :
      chip = g1[10] ^ g2[2] ^ g2[9]; break;
    case 9 :
      chip = g1[10] ^ g2[3] ^ g2[10]; break;
    case 10:
      chip = g1[10] ^ g2[2] ^ g2[3]; break;
    case 11:
      chip = g1[10] ^ g2[3] ^ g2[4]; break;
    case 12:
      chip = g1[10] ^ g2[5] ^ g2[6]; break;
    case 13:
      chip = g1[10] ^ g2[6] ^ g2[7]; break;
    case 14:
      chip = g1[10] ^ g2[7] ^ g2[8]; break;
    case 15:
      chip = g1[10] ^ g2[8] ^ g2[9]; break;
    case 16:
      chip = g1[10] ^ g2[9] ^ g2[10]; break;
    case 17:
      chip = g1[10] ^ g2[1] ^ g2[4]; break;
    case 18:
      chip = g1[10] ^ g2[2] ^ g2[5]; break;
    case 19:
      chip = g1[10] ^ g2[3] ^ g2[6]; break;
    case 20:
      chip = g1[10] ^ g2[4] ^ g2[7]; break;
    case 21:
      chip = g1[10] ^ g2[5] ^ g2[8]; break;
    case 22:
      chip = g1[10] ^ g2[6] ^ g2[9]; break;
    case 23:
      chip = g1[10] ^ g2[1] ^ g2[3]; break;
    case 24:
      chip = g1[10] ^ g2[4] ^ g2[6]; break;
    case 25:
      chip = g1[10] ^ g2[5] ^ g2[7]; break;
    case 26:
      chip = g1[10] ^ g2[6] ^ g2[8]; break;
    case 27:
      chip = g1[10] ^ g2[7] ^ g2[9]; break;
    case 28:
      chip = g1[10] ^ g2[8] ^ g2[10]; break;
    case 29:
      chip = g1[10] ^ g2[1] ^ g2[6]; break;
    case 30:
      chip = g1[10] ^ g2[2] ^ g2[7]; break;
    case 31:
      chip = g1[10] ^ g2[3] ^ g2[8]; break;
    case 32:
      chip = g1[10] ^ g2[4] ^ g2[9]; break;
    case 33:
      chip = g1[10] ^ g2[5] ^ g2[10]; break;
    case 34:
      chip = g1[10] ^ g2[4] ^ g2[10]; break;
    case 35:
      chip = g1[10] ^ g2[1] ^ g2[7]; break;
    case 36:
      chip = g1[10] ^ g2[2] ^ g2[8]; break;
    case 37:
      chip = g1[10] ^ g2[4] ^ g2[10]; break;
    }
    CACode = (CACode<<1) | (chip & 0x1);
    // next [1]
    g1[0] = g1[3] ^ g1[10];
    g2[0] = g2[2] ^ g2[3] ^ g2[6] ^ g2[8] ^ g2[9] ^ g2[10];    
    for (int j=10;j>0;j--) {
      //g1[10:1] <= {g1[9:1], g1[3] ^ g1[10]};
      //g2[10:1] <= {g2[9:1], g2[2] ^ g2[3] ^ g2[6] ^ g2[8] ^ g2[9] ^ g2[10]};
      g1[j] = g1[j-1]; // shift
      g2[j] = g2[j-1];
    }
  }
  //
  // prepare for next round
  //
  g1[0] = g1[3] ^ g1[10];
  g2[0] = g2[2] ^ g2[3] ^ g2[6] ^ g2[8] ^ g2[9] ^ g2[10];    
  for (int j=10;j>0;j--) {
    g1[j] = g1[j-1]; // shift
    g2[j] = g2[j-1];
  }
  //
  return CACode;
}

int cep_gps::RunGpsTest(int maxLoop) {

  int outLen=mBlockSize;
  //
  //
  //
  SetSvNum(0);   // so GPS can detect a change
  //
  for (int i=1;i<=maxLoop;i++) {
    if (GetVerbose()) {
      LOGI("%s: Loop %d\n",__FUNCTION__,i);
    }
    SetSvNum(i);    
    //BusReset();
    // Load SvNum and start
    Start();
    waitTilDone(500);
    mErrCnt += ReadNCheck_CA_Code(0x1FFF);
    //
    Read_PCode();
    Read_LCode();
    // get expecte L-code
    mErrCnt += openssl_aes192_ecb_encryption
      (mHwPt, // uint8_t *input,           // input text packet
       mSwCp, // uint8_t *output,           // output cipher packet
       1,   // int padding_enable,
       mBlockSize, // int length,
       &outLen, GetVerbose());
    //
    mErrCnt += CheckCipherText();    
    //
    //
    // Print
    //
    if (mErrCnt || GetVerbose()) {
      PrintMe("Key",       &(mKEY[0]),mKeySize);          
      PrintMe("P-code"    ,&(mHwPt[0]),mBlockSize);
      PrintMe("Exp L-Code",&(mSwCp[0]),mBlockSize);
      PrintMe("Act L-Code",&(mHwCp[0]),mBlockSize);        
    }
    MarkSingle(i-1);    
    if (mErrCnt) break;
  }
  return mErrCnt;
}

