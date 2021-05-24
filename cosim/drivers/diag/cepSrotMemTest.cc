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
#include "cepSrotMemTest.h"
#include "cepMemTest.h"

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
int cepSrotMemTest_runTest(int cpuId, int seed, int verbose) {
  int errCnt = 0;

  int full = 1;
  u_int32_t mem_base;
  int adrWidth ;
  int dataWidth = 64;
  //
  // disable operational bits
  DUT_WRITE32_64(srot_base_addr + SROT_CTRLSTS_ADDR , 0);
  //
  //
  //
  for (int i=0;i<2;i++) {
    if (i == 0) {
      adrWidth = 11; // 2048 locations
      mem_base = srot_base_addr + SROT_KEYRAM_ADDR;
    } else {
      adrWidth = 6; // 32 locations
      mem_base = srot_base_addr + SROT_KEYINDEXRAM_ADDR;
    }
    if (!errCnt) { errCnt = cepMemTest_runTest(cpuId,mem_base,adrWidth,dataWidth ,seed, verbose, full); }
  }
  return errCnt;
}
