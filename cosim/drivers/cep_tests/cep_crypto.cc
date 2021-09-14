//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef  BARE_MODE    
#include <string.h>
#endif

#include "v2c_cmds.h"
#include "cep_apis.h"
#include "cep_crypto.h"
#include "simdiag_global.h"
#include "portable_io.h"
#include "CEP.h"
#include "cepregression.h"
#include "random48.h"

void cep_crypto::init(int coreIndex) {
#ifndef  BARE_MODE        
  mFd     = 0;
#endif
  mCoreIndex = coreIndex;
  mCapture = 0;
  mSrotFlag = 0;
  mC2C_Capture = 0;
  mErrCnt  = 0;
  mCount   = 0;
  mSingle  = 0;
  mWordCnt = 0;
  mAdrBase = 0;
  mAdrSize  = 0x10000; // default
  SetExpErr(0);
}

void cep_crypto::freeMe(void) {
#ifndef BARE_MODE  
  if (mCapture) {
    // terminate
    fprintf(mFd,"};\n\n");
    fprintf(mFd,"#define %s_adrBase 0x%010lx\n", mTestName, mAdrBase);
    fprintf(mFd,"#define %s_adrSize 0x%0lx\n", mTestName, mAdrSize);
    fprintf(mFd,"#define %s_size %d\n", mTestName,mWordCnt);
    fprintf(mFd,"#define %s_cmdCnt4Single %d\n", mTestName, mSingle);    
    fprintf(mFd,"#define %s_totalCommands %d\n#endif\n", mTestName, mCount);

    //
    fclose(mFd);
  }
  // turn off the capturing
  if (mC2C_Capture) {
    writeDvt(mC2C_Capture, mC2C_Capture, 0);
    if (mC2C_Capture == DVTF_DFT_CAPTURE_EN_BIT) { // also stop IDFT
      writeDvt(DVTF_IDFT_CAPTURE_EN_BIT, DVTF_IDFT_CAPTURE_EN_BIT, 0);
    }
  }
  
#endif
} 
//
//
//
void cep_crypto::SetSeed (int seed) {
  mSeed=seed;
  Random48_srand48(seed);
}

//
void cep_crypto::RandomGen(uint8_t *buf, int size) {
  // random
  uint32_t rand = Random48_rand();  
  for (int j=0;j<size;j++) {
    if ((j % 4)==0) rand = Random48_rand(); // pick up new number
    buf[j] = rand & 0xFF;
    rand = rand >> 8;
  }
}
//
int cep_crypto::CheckCipherText(void) {
  for (int i=0;i<GetBlockSize();i++) {
    if (mHwCp[i] != mSwCp[i]) mErrCnt++;
  }
  return mErrCnt;
}
int cep_crypto::CheckPlainText(void) {
  for (int i=0;i<GetBlockSize();i++) {
    if (mHwPt[i] != mSwPt[i]) mErrCnt++;
  }
  return mErrCnt;
}

//
// Print
//
void cep_crypto::PrintMe(const char *name, uint8_t *buf, int size) {
  //if (GetVerbose()==0) return;
  //
  char str[256];
  //printf("Printing packet\n");
  // print every 32 bytes??
  int bC = 0;
  int first=1;
  str[0] = 0; // empty
  for (int i=0;i<size;i++) {
    if (bC == 0)        { 
      if (first) {
	sprintf(str,"%s: %02x",name,*(buf+i) & 0xff); 
	first = 0;
      } else {
	sprintf(str,"             %02x",*(buf+i) & 0xff); 
      }
    }  // start of line
    else if (bC & 0x1)  { sprintf(str,"%s%02x",str,*(buf+i) & 0xff); } // add 1 byte
    else                { sprintf(str,"%s_%02x",str,*(buf+i) & 0xff); } // add 1 byte
    // print every 32 bytes in a line
    if ((bC == 31) || (i == (size-1))) { // 16 bytes or end
      LOGI("%s\n",str);
      bC = 0;
    } else { bC++; }
  }
}

void cep_crypto::PrintMe(const char *name, double *buf1, double *buf2, int size) {
  //
  //if (GetVerbose()==0) return;  
  char str[128];
  //printf("Printing packet\n");
  // print every 32 bytes??
  int bC = 0;
  int first=1;
  str[0] = 0; // empty
  for (int i=0;i<size;i++) {
    if (bC == 0)        { 
      if (first) {
	sprintf(str,"%s: %f %f",name,*(buf1+i),*(buf2+i));
	first = 0;
      } else {
	sprintf(str,"             %f %f",*(buf1+i),*(buf2+i));
      }
    }  // start of line
    else if (bC & 0x1)  { sprintf(str,"%s%f %f ",str,*(buf1+i),*(buf2+i) ); } // add 1 byte
    else                { sprintf(str,"%s_%f_%f ",str,*(buf1+i),*(buf2+i)); } // add 1 byte
    // print every 32 bytes in a line
    if ((bC == 0) || (i == (size-1))) { // 16 bytes or end
      LOGI("%s\n",str);
      bC = 0;
    } else { bC++; }
  }
}
//
// To support bareMetal and unit level testbench
//

void cep_crypto::SetCaptureMode(int mode, const char *path, const char *testName) {
#ifndef BARE_MODE
  //
  char fName[512];
  //  
  //init();
  //
  // Sim Only
  //
  if (Get_C2C_Capture()) {
    if (strcmp(testName,"aes") == 0) {
      mC2C_Capture = DVTF_AES_CAPTURE_EN_BIT;
    }
    else if (strcmp(testName,"sha256") == 0) {
      mC2C_Capture = DVTF_SHA256_CAPTURE_EN_BIT;
    }    
    else if (strcmp(testName,"md5") == 0) {
      mC2C_Capture = DVTF_MD5_CAPTURE_EN_BIT;
    }    
    else if (strcmp(testName,"rsa") == 0) {
      mC2C_Capture = DVTF_RSA_CAPTURE_EN_BIT;
    }
    else if (strcmp(testName,"des3") == 0) {
      mC2C_Capture = DVTF_DES3_CAPTURE_EN_BIT;
    }        
    else if (strcmp(testName,"gps") == 0) {
      mC2C_Capture = DVTF_GPS_CAPTURE_EN_BIT;
    }    
    else if (strcmp(testName,"dft") == 0) {
      // both in one test
      mC2C_Capture = DVTF_DFT_CAPTURE_EN_BIT; 
      // also the IDFT since it is in same test
      writeDvt(DVTF_IDFT_CAPTURE_EN_BIT, DVTF_IDFT_CAPTURE_EN_BIT, 1);      
    }    
    else if (strcmp(testName,"idft") == 0) {
      mC2C_Capture = DVTF_IDFT_CAPTURE_EN_BIT;
    }    
    else if (strcmp(testName,"iir") == 0) {
      mC2C_Capture = DVTF_IIR_CAPTURE_EN_BIT;
    }    
    else if (strcmp(testName,"fir") == 0) {
      mC2C_Capture = DVTF_FIR_CAPTURE_EN_BIT;
    }    
    //
    writeDvt(mC2C_Capture, mC2C_Capture, 1);      
  }
  //  
  //
  mCapture = mode;
  //strcpy(mPath,path);
  strcpy(mTestName,testName);
  // open file if on
  if (mCapture) {
    // for input
    sprintf(fName,"%s/%s_playback.h",path,testName);
    mFd = fopen(fName,"w");
    if (mFd == NULL) {
      LOGI("%s: ERROR: Can't open file %s\n",__FUNCTION__,fName);
      mCapture = 0; // turn off
    } else {
      LOGI("%s: Opening file %s for sequence capturing\n",__FUNCTION__,fName);
      // print CopyRigth
      fprintf(mFd,"//************************************************************************\n");
      fprintf(mFd,"// Copyright 2021 Massachusetts Institute of Technology\n");
      fprintf(mFd,"//\n");
      fprintf(mFd,"// This file is auto-generated for test: %s. Do not modify!!!\n", testName);
      fprintf(mFd,"//\n");      
      fprintf(mFd,"// Generated on: %s %s\n",__DATE__, __TIME__);
      fprintf(mFd,"//************************************************************************\n");      
      //
      // print Header
      //
      fprintf(mFd,"#ifndef %s_playback_H\n",testName);
      fprintf(mFd,"#define %s_playback_H\n\n",testName);
      // command encoding
      fprintf(mFd,"#ifndef PLAYBACK_CMD_H\n");
      fprintf(mFd,"#define PLAYBACK_CMD_H\n");
      fprintf(mFd,"// Write to : <physicalAdr> <writeData>\n");
      fprintf(mFd,"#define WRITE__CMD  1\n");
      fprintf(mFd,"// Read and compare: <physicalAdr> <Data2Compare>\n");      
      fprintf(mFd,"#define RDnCMP_CMD  2\n");
      fprintf(mFd,"// Read and spin until match : <physicalAdr> <Data2Match> <mask> <timeout>\n");            
      fprintf(mFd,"#define RDSPIN_CMD  3\n\n");
      fprintf(mFd,"#define WRITE__CMD_SIZE  3\n");
      fprintf(mFd,"#define RDnCMP_CMD_SIZE  3\n");
      fprintf(mFd,"#define RDSPIN_CMD_SIZE  5\n");      
      fprintf(mFd,"#endif\n\n");      
      fprintf(mFd,"// %s command sequences to playback\n",testName);            
      fprintf(mFd,"uint64_t %s_playback[] = { \n",testName);            
    }
  }
#endif
  
}

void cep_crypto::cep_writeNcapture(uint32_t pAddress, uint64_t pData)
{
  cep_writeNcapture(mCoreIndex, pAddress, pData);
}

void cep_crypto::cep_writeNcapture(int coreIndex, uint32_t pAddress, uint64_t pData)
{
  cep_write(coreIndex, pAddress, pData);
  //
  // save to file in captured mode (BFM only)
  //
#ifndef BARE_MODE  
  if (mCapture) {
    if (mCount++ == 0) {
      fprintf(mFd,"\t  WRITE__CMD, 0x%08x, 0x%016lx // %d\n", (uint32_t)get_physical_adr(coreIndex,pAddress), pData, mCount);
    } else {
      fprintf(mFd,"\t, WRITE__CMD, 0x%08x, 0x%016lx // %d\n", (uint32_t)get_physical_adr(coreIndex,pAddress), pData, mCount);      
    }
    // save this away
    if (mAdrBase == 0) {
      mAdrBase = get_physical_adr(coreIndex,pAddress) & ~(mAdrSize-1);
    }    
    mWordCnt += 3;
  }
#endif
}

uint64_t cep_crypto::cep_readNcapture(uint32_t pAddress) {
  cep_readNcapture(mCoreIndex, pAddress);
}

uint64_t cep_crypto::cep_readNcapture(int coreIndex, uint32_t pAddress) {
  uint64_t pData = cep_read(coreIndex, pAddress);
  //
  // save to file in captured mode (BFM only)
  //
#ifndef  BARE_MODE  
  if (mCapture) {
    if (mCount++ == 0) {
      fprintf(mFd,"\t  RDnCMP_CMD, 0x%08x, 0x%016lx // %d\n", (uint32_t)get_physical_adr(coreIndex,pAddress), pData, mCount);
    } else {
      fprintf(mFd,"\t, RDnCMP_CMD, 0x%08x, 0x%016lx // %d\n", (uint32_t)get_physical_adr(coreIndex,pAddress), pData, mCount);      
    }
    mWordCnt += 3;    
  }
#endif
  //
  return pData;
}
//
int cep_crypto::cep_readNspin(uint32_t pAddress, uint64_t pData,int timeOut) {
  return cep_readNspin(pAddress,pData,(uint64_t)(-1), timeOut) ;
}
int cep_crypto::cep_readNspin(uint32_t pAddress, uint64_t pData,uint64_t mask, int timeOut) {
  return cep_readNspin(mCoreIndex,pAddress,pData,mask, timeOut) ;
}
int cep_crypto::cep_readNspin(int coreIndex, uint32_t pAddress,uint64_t pData,uint64_t mask, int timeOut) {
  uint64_t rdDat;
  //
#ifndef  BARE_MODE  
  if (mCapture) {
    if (mCount++ == 0) {
      fprintf(mFd,"\t  RDSPIN_CMD, 0x%08x, 0x%016lx, 0x%lx, 0x%x // %d\n",
	      (uint32_t)get_physical_adr(coreIndex,pAddress), pData,mask,timeOut, mCount);
    } else {
      fprintf(mFd,"\t, RDSPIN_CMD, 0x%08x, 0x%016lx, 0x%lx, 0x%x // %d\n",
	      (uint32_t)get_physical_adr(coreIndex,pAddress), pData,mask,timeOut, mCount);      
    }
    mWordCnt += 5;    
  }
#endif
  //
  while (timeOut > 0) {
    rdDat = cep_read(coreIndex, pAddress);
    if (((rdDat ^ pData) & mask) == 0) {
      break;
    }
    USEC_SLEEP(100);
    timeOut--;
  };
  if (GetVerbose(2)) {  LOGI("%s: expData=0x%016lx rdDat=0x%016lx\n",__FUNCTION__,pData,rdDat); }
  return (timeOut <= 0) ? 1 : 0;
}
void cep_crypto::MarkSingle(int loop) {
  if ((loop >= mCapture) && (mSingle==0)) {
    mSingle = mCount;
  }
}

