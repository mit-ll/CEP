//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      riscv_wrapper.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//--------------------------------------------------------------------------------------

// For bareMetal mode ONLY
#ifdef BARE_MODE
  #include "cep_apis.h"
  #include "portable_io.h"
  #include "CEP.h"
  #include "cepMemTest.h"

  #ifdef __cplusplus
  extern "C" {
  #endif
  
  void thread_entry(int cid, int nc) {
    
    int errCnt    = 0;
    int testId[4] = {0x00, 0x11, 0x22, 0x33};
    int coreId    = read_csr(mhartid);
  
    uint32_t mem_base;
    int adrWidth;
    int dataWidth = 64;
    int seed = 0;
    int full = 0;
    int verbose = 0;

    // Set the current core's status to running
    set_cur_status(CEP_RUNNING_STATUS);
  
    // Run the specified test 
    for (int i=0;i<2;i++) {
      if (i == 0) {
        adrWidth = 8-2; // 256 locations/4
        mem_base = SROT_BASE_ADDR + SROT_KEYRAM_ADDR + (coreId << 6) ;
      } else {
        adrWidth = 6-2; // 64 locations/4
        mem_base = SROT_BASE_ADDR + SROT_KEYINDEXRAM_ADDR + (coreId << 4);
      }
      if (!errCnt) { errCnt = cepMemTest_runTest(coreId,mem_base,adrWidth,dataWidth ,seed, verbose, full); }
    }

    // Set the core status
    set_status(errCnt, testId[coreId]);

    // Exit with the error count
    exit(errCnt);
  }

  #ifdef __cplusplus
  }
  #endif
  
#endif // #ifdef BARE_MODE

