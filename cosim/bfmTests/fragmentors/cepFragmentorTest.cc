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
#include <string.h>

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepFragmentorTest.h"
#include "random48.h"

#include "cep_apis.h"
#include "portable_io.h"

#ifdef BARE_MODE
#else
#include "simPio.h"
#endif


// Atomic operation 
int cepFragmentorTest_atomicTest(int cpuId, uint32_t baseAdr, int mask, int checkEn, int seed, int verbose) {
  //
  // DO the write
  //
  int errCnt = 0;
  uint64_t pat = 0x123456789abcdef0;
  // 
  // for thru all 4 XOR.OR,AND.SWAP
  // 
  // Load this register with known pattern
  DUT_WRITE32_64(baseAdr,pat);
  for (int p=0;p<4;p++) {
    DUT_ATOMIC_RMW64(baseAdr, p, mask, &pat);
    if (checkEn) { // Added me!!!
    }
  }
  return errCnt;
}

//
// Test scratch register up to 8 of them
//
//
int cepFragmentorTest_verifyMaskRom(int cpuId, uint32_t baseAdr, char *file, int wordCnt, int seed, int verbose) {
  uint64_t buf[wordCnt], rdBuf[wordCnt], mask = (uint64_t)-1;
  int errCnt = read_binFile(file, buf, wordCnt);
  //
  DUT_READ32_BURST (baseAdr, wordCnt, rdBuf); // reg_base_addr + cep_scratch0_reg, w, rdBuf);
  for (int i=0;i<wordCnt;i++) {
    if ((buf[i] ^ rdBuf[i]) & mask) {
      LOGE("ERROR: baseAdr=0x%08x w=%d i=%d exp=0x%016lx act=0x%016lx\n",baseAdr,wordCnt,i,buf[i],rdBuf[i]);
      errCnt++;
    } else if (verbose) {
      LOGI("OK: baseAdr=0x%08x w=%d i=%d exp=0x%016lx act=0x%016lx\n",baseAdr,wordCnt,i,buf[i],rdBuf[i]);
    }
  }
  return errCnt;
}

int cepFragmentorTest_baseTest(int cpuId, uint32_t baseAdr, int wrW, int rdW, uint64_t mask, int seed, int verbose) {
  int i,r,w,p;
  int errCnt = 0;
  uint64_t buf[32] = {0xaaaaaaaaaaaaaaaa,
		      0x5555555555555555,
                      0x123456789abcdef0,
		      0x1111111111111111,
		      0x2222222222222222,
		      0x3333333333333333,
		      0x4444444444444444,
		      0x6666666666666666,
		      0x7777777777777777,
		      0x8888888888888888,
		      0x9999999999999999,
		      0xbbbbbbbbbbbbbbbb,
		      0xcccccccccccccccc,
		      0xdddddddddddddddd,
		      0xeeeeeeeeeeeeeeee,
		      0xffffffffffffffff,
		      0,0,0,0,0,0,0,0,
		      0,0,0,0,0,0,0,0 };
  uint64_t rdBuf[32];
  // 
  // if mask = 0, do the hole atomic!!
  //
  if (mask == 0) {
    cepFragmentorTest_atomicTest(cpuId,baseAdr,0xff,0,seed,verbose);
  }
  //
  Random48_srand48(seed);
  for (p=1;p<=8;p++) { // 2,4,8,etc... power of 2
    w = 1 << p;
    if (w <= wrW) {
      //
      DUT_WRITE32_BURST(baseAdr, w, buf); // reg_base_addr + cep_scratch0_reg, w, buf);
    }
    if (w <= rdW) {
      DUT_READ32_BURST (baseAdr, w, rdBuf); // reg_base_addr + cep_scratch0_reg, w, rdBuf);
      //
      for (int i=0;i<w;i++) {
	if ((buf[i] ^ rdBuf[i]) & mask) {
	  LOGE("ERROR: baseAdr=0x%08x w=%d i=%d exp=0x%016lx act=0x%016lx\n",baseAdr,w,i,buf[i],rdBuf[i]);
	  errCnt++;
	} else if (verbose) {
	  LOGI("OK: baseAdr=0x%08x w=%d i=%d exp=0x%016lx act=0x%016lx\n",baseAdr,w,i,buf[i],rdBuf[i]);
	}
	if (errCnt) break;
	// randomize the next
	for (i=0;i<32;i++) {
	  buf[i] = Random48_mrand48();
	}
      }
    }
  }
  return errCnt;
}
