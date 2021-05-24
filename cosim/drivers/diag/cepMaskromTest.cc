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
#include "cepMaskromTest.h"

#include "CEP.h"
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
int cepMaskromTest_runTest(int cpuId, int seed, int verbose) {
  int errCnt = 0;
  //
  uint32_t offs;
  uint64_t dat64, edat64;
  uint32_t dat32, edat32;
  uint16_t dat16, edat16;
  uint8_t dat8, edat8;

#ifdef SIM_ENV_ONLY
  uint32_t known_pat1 = bootrom_known_pat1;
  uint32_t known_pat0 = bootrom_known_pat0;
  //
  // Read ONLY
  //
  offs = bootrom_base_addr + ((1 << 15)-8); // bottom of 32k bytes
#else
  // from bootbare.hex
  uint32_t known_pat1 = 0x00800913; // location 0
  uint32_t known_pat0 = 0x30491073; // location 4
  //
  // Read ONLY
  //
  offs = bootrom_base_addr; //  + ((1<<13)-8);
#endif

  // 64-bit
  DUT_READ32_64(offs, dat64);

  edat64 = ((uint64_t)known_pat0 << 32) | known_pat1;
  if (dat64 != edat64) {
    LOGE("ERROR: mismatch offs=0x%08x act=0x%016lx exp=0x%016lx\n",offs,dat64,edat64);
    errCnt++;
  } else if (verbose) {
    LOGI("OK: read64 offs=0x%08x act=0x%016lx exp=0x%016lx\n",offs,dat64,edat64);    
  }
  // 32-bit
  offs += 4;
  DUT_READ32_32(offs, dat32);
  edat32 = (uint32_t)known_pat0;
  if (dat32 != edat32) {
    LOGE("ERROR: mismatch offs=0x%08x act=0x%08x exp=0x%08x\n",offs,dat32,edat32);
    errCnt++;
  } else if (verbose) {
    LOGI("OK: read32 mismatch offs=0x%08x act=0x%08x exp=0x%08x\n",offs,dat32,edat32);    
  }
  // 16-bit
  offs += 2;
  DUT_READ32_16(offs, dat16);
  edat16 = (uint16_t)(known_pat0>>16);
  if (dat16 != edat16) {
    LOGE("ERROR: mismatch offs=0x%08x act=0x%04x exp=0x%04x\n",offs,dat16,edat16);
    errCnt++;
  } else if (verbose) {
    LOGI("OK: read16 offs=0x%08x act=0x%04x exp=0x%04x\n",offs,dat16,edat16);    
  }
  // 8-bit
  offs += 1;
  DUT_READ32_8(offs, dat8);
  edat8 = (uint8_t)(known_pat0>>24);
  if (dat8 != edat8) {
    LOGE("ERROR: mismatch offs=0x%08x act=0x%02x exp=0x%02x\n",offs,dat8,edat8);
    errCnt++;
  } else if (verbose) {
    LOGI("OK: read8 offs=0x%08x act=0x%02x exp=0x%02x\n",offs,dat8,edat8);    
  }

  return errCnt;
}
