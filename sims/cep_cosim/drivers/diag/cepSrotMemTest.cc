//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cepSrotMemTest.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include "simdiag_global.h"
#include "CEP.h"
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

  // SROT Memory does not support sub-word access
  int dataWidth = 64;

  // This will be a single-threaded test (only CPU#0)

  if (cpuId == 0) {   
    for (int i=0;i<2;i++) {
      if (i == 0) {
        // Test the KeyRAM
        adrWidth = 11; // 2048 locations
        mem_base = SROT_BASE_ADDR + SROT_KEYRAM_ADDR;
      } else {
        // Test the KeyIndex RAM
        adrWidth = 6; // 32 locations
        mem_base = SROT_BASE_ADDR + SROT_KEYINDEXRAM_ADDR;
      } // end if (i == 0)
      if (!errCnt) { errCnt = cepMemTest_runTest(cpuId, mem_base, adrWidth, dataWidth ,seed, verbose, full); }
    } // end for (int i=0;i<2;i++)
  } // end if (cpuId == 0)
  return errCnt;
}
