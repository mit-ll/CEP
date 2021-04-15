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
#include "cepDdr3MemTest.h"
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
//, 
int cepDdr3MemTest_runTest(int cpuId, uint32_t mem_base, int adrWidth, int seed, int full, int verbose) {
  int errCnt = 0;
  int dataWidth = 0;
  //
  for (int i=0;i<4;i++) {
    switch ((i + cpuId) & 0x3) {
    case 0 : dataWidth = 8; break;
    case 1 : dataWidth = 16; break;
    case 2 : dataWidth = 32; break;
    case 3 : dataWidth = 64; break;
    }
    if (!errCnt) { errCnt = cepMemTest_runTest(cpuId,mem_base,adrWidth,dataWidth ,seed + ((cpuId+i)*0x10), verbose, full); }
  }
  
  return errCnt;
}
