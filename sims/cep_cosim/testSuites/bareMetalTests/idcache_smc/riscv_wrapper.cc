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
  #include "cep_riscv.h"
  
  #ifdef __cplusplus
  extern "C" {
  #endif
  
  void thread_entry(int cid, int nc) {
    
    int errCnt    = 0;
    int testId[4] = {0x00, 0x11, 0x22, 0x33};
    int coreId    = read_csr(mhartid);
    int revCheck  = 1;
    int verbose   = 0;
    uint64_t mem_base = scratchpad_base_addr + 0x000E0000;

    // Set the current core's status to running
    set_cur_status(CEP_RUNNING_STATUS);

    // Each core needs to have its own portion of scratchpad memory
    switch (coreId) {
      case 0: { mem_base = scratchpad_base_addr + 0x000F0000; break; }
      case 1: { mem_base = scratchpad_base_addr + 0x000F4000; break; }
      case 2: { mem_base = scratchpad_base_addr + 0x000F8000; break; }
      case 3: { mem_base = scratchpad_base_addr + 0x000FC000; break; }
    }
    int blockCnt = 32;
  
    errCnt += do_DIcache_SMC((uint64_t *)mem_base, blockCnt, 0x10 + coreId, 0);
  
    // Set the core status
    set_status(errCnt, testId[coreId]);

    // Exit with the error count
    exit(errCnt);
  }

  #ifdef __cplusplus
  }
  #endif
  
#endif // #ifdef BARE_MODE
