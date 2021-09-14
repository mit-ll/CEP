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
#include <string.h>

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepAccessTest.h"

#include "cep_apis.h"
#include "portable_io.h"

#ifdef SIM_ENV_ONLY
#include "simPio.h"
#endif

//
// =============================
// The test itself
// =============================
//
// modeSupportMask[0]  = 64 bits read
// modeSupportMask[1]  = 64 bits write
// modeSupportMask[8]  = 32 bits read
// modeSupportMask[9]  = 32 bits write
// modeSupportMask[16] = 16 bits read
// modeSupportMask[17] = 16 bits write
// modeSupportMask[24] =  8 bits read
// modeSupportMask[25] =  8 bits write
//
// ONly write if supported, read always supported
//

int cepAccessTest_runSingle(int cpuId, uint32_t adr, uint64_t pat, int modeSupportMask, int verbose) {
  int errCnt = 0;
  uint32_t wdat32, rdat32;
  uint16_t wdat16, rdat16;
  uint8_t wdat8, rdat8;
  uint64_t act=0;
  //
  // 64 read/write -bits
  //
  if (modeSupportMask & (1 << 0)) { // wr64
    DUT_WRITE32_64(adr, pat);
  }
  //
  if (modeSupportMask & (1 << 1)) { // rd64
    DUT_READ32_64(adr, act);
  }
  if (pat != act) {
    LOGE("ERROR: 64-bits offset0x%08x exp=0x%016lx act=0x%016lx\n",adr,pat,act);
    errCnt++;
  } else if (verbose) {
    LOGI("OK: 64-bits offset0x%08x exp=0x%016lx act=0x%016lx\n",adr,pat,act);
  }
  //
  // 32 read/write -bits
  //
  wdat32 = pat; // lower 32
  wdat32 = (uint32_t)~wdat32;
  DUT_WRITE32_32(adr, wdat32);
  if (modeSupportMask & (1 << 2)) { // wr32
    pat = (pat & ((uint64_t)0xFFFFFFFF00000000LL)) | wdat32;
  } else {
    wdat32 = pat; //  no change    
  }
  //
  // check 32
  DUT_READ32_32(adr, rdat32);
  if (wdat32 != rdat32) {
    LOGE("ERROR: 32-bits offset0x%08x exp=0x%08x act=0x%08x\n",adr,wdat32,rdat32);
    errCnt++;
  } else if (verbose) {
    LOGI("OK: 32-bits offset0x%08x exp=0x%08x act=0x%08x\n",adr,wdat32,rdat32);
  }
  // check with 64 bits
  DUT_READ32_64(adr, act);
  if (pat != act) {
    LOGE("ERROR: 64/32-bits offset0x%08x exp=0x%016lx act=0x%016lx\n",adr,pat,act);
    errCnt++;
  } else if (verbose) {
    LOGI("OK: 64/32-bits offset0x%08x exp=0x%016lx act=0x%016lx\n",adr,pat,act);
  }
  //
  // 16 read/write -bits
  //
  wdat16 = (uint16_t)(pat >> 32); // byte4-5
  wdat16 = ~wdat16;
  DUT_WRITE32_16(adr+4, wdat16);
  if (modeSupportMask & (1 << 4)) { // wr16
    // modify 64-bit pattern
    pat = (pat & ((uint64_t)0xFFFF0000FFFFFFFFLL)) | ((uint64_t)wdat16 << 32);
  } else {
    wdat16 = (uint16_t)(pat >> 32); // byte4-5 , no change
  }
  // check 16
  DUT_READ32_16(adr+4, rdat16);
  if (wdat16 != rdat16) {
    LOGE("ERROR: 16-bits offset0x%08x exp=0x%04x act=0x%04x\n",adr+4,wdat16,rdat16);
    errCnt++;
  } else if (verbose) {
    LOGI("OK: 16-bits offset0x%08x exp=0x%04x act=0x%04x\n",adr+4,wdat16,rdat16);
  }
  // check with 64 bits
  DUT_READ32_64(adr, act);
  if (pat != act) {
    LOGE("ERROR: 64/16-bits offset0x%08x exp=0x%016lx act=0x%016lx\n",adr,pat,act);
    errCnt++;
  } else if (verbose) {
    LOGI("OK: 64/16-bits offset0x%08x exp=0x%016lx act=0x%016lx\n",adr,pat,act);
  }
  //
  // 8 read/write -bits
  //
  wdat8 = (uint8_t)(pat >> 56); // byte7
  wdat8 = ~wdat8;
  DUT_WRITE32_8(adr+7, wdat8);
  if (modeSupportMask & (1 << 6)) { // wr8
    // modify 64-bit pattern
    pat = (pat & ((uint64_t)0x00FFFFFFFFFFFFFFLL)) | ((uint64_t)wdat8 << 56);
  } else {
    wdat8 = (uint8_t)(pat >> 56); // byte7
  }
  // check 8
  DUT_READ32_8(adr+7, rdat8);
  if (wdat8 != rdat8) {
    LOGE("ERROR: 8-bits offset0x%08x exp=0x%02x act=0x%02x\n",adr+7,wdat8,rdat8);
    errCnt++;
  } else if (verbose) {
    LOGI("OK: 8-bits offset0x%08x exp=0x%02x act=0x%02x\n",adr+7,wdat8,rdat8);
  }
  // check with 64 bits
  DUT_READ32_64(adr, act);
  if (pat != act) {
    LOGE("ERROR: 64/8-bits offset0x%08x exp=0x%016lx act=0x%016lx\n",adr,pat,act);
    errCnt++;
  } else if (verbose) {
    LOGI("OK: 64/8-bits offset0x%08x exp=0x%016lx act=0x%016lx\n",adr,pat,act);
  }
  return errCnt;
}


int cepAccessTest_runTest(int cpuId, int seed, int verbose) {
  int errCnt = 0;
  uint32_t adr;
  uint64_t pat;
  int modeSupportMask;
  //
  // Clint
  //
#ifdef SIM_ENV_ONLY
  adr = clint_base_addr + clint_mtimecmp_offset + (cpuId*8);
  pat = (uint64_t)0xCC33C3C333CC3C3CLL;
  modeSupportMask = 0xFF;
  if (!errCnt) { errCnt += cepAccessTest_runSingle(cpuId, adr, pat, modeSupportMask, verbose); }
#endif
  //
  // DDR3
  // 
  adr = 0x90000000 + (cpuId*8);
  pat = (uint64_t)0x1122334455667788LL;
  modeSupportMask = 0xFF;
  if (!errCnt) { errCnt += cepAccessTest_runSingle(cpuId, adr, pat, modeSupportMask, verbose); }
  //
  //
  // Reg Space
  // 
  adr = reg_base_addr + cep_scratch0_reg + (cpuId*8);
  pat = (uint64_t)0x123456789abcdef0;
  modeSupportMask = 3;
  if (!errCnt) { errCnt += cepAccessTest_runSingle(cpuId, adr, pat, modeSupportMask, verbose); }

  // SHA256
  adr = sha256_base_addr + 0x8 + (cpuId*8); // msg input word 0 - 7
  pat = (uint64_t)0x33445566778899aa;
  modeSupportMask = 3;
  if (!errCnt) { errCnt += cepAccessTest_runSingle(cpuId, adr, pat, modeSupportMask, verbose); }

  // MD5
  adr = md5_base_addr + 0x8 + (cpuId*8); // msg_padded.word0 - 7
  pat = (uint64_t)0xbbccddeeff55aa55;
  modeSupportMask = 3;
  if (!errCnt) { errCnt += cepAccessTest_runSingle(cpuId, adr, pat, modeSupportMask, verbose); }

  //
  // Other spaces & cpuId = 0 Only since only 1 register
  //
  if (cpuId == 0) {
    adr = aes_base_addr + aes_pt0_addr;
    pat = (uint64_t)0xaabbccddeeff1234;
    modeSupportMask = 3;
    if (!errCnt) { errCnt += cepAccessTest_runSingle(cpuId, adr, pat, modeSupportMask, verbose); }
    // DES3
    adr = des3_base_addr + 0x10; // DES3 Plaintext Word 1
    pat = (uint64_t)0x56789abcdef01122;
    modeSupportMask = 3;
    if (!errCnt) { errCnt += cepAccessTest_runSingle(cpuId, adr, pat, modeSupportMask, verbose); }
  }

  return errCnt;
}
