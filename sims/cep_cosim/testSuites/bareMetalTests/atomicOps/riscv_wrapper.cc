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
    
    int l;
    uint64_t *ptr;
    uint64_t expVal;
    uint64_t atomicOps_playUnit[4];

    // Set the current core's status to running
    set_cur_status(CEP_RUNNING_STATUS);
 

    for (l = 0; l < 3; l++) {
      switch (l) {
        case 0 : 
          // cbus via CLINT
          ptr = (uint64_t *)(clint_base_addr + clint_mtimecmp_offset + (coreId*8));      
          expVal = ~(clint_base_addr + clint_mtimecmp_offset * (coreId*8));
        break;

        case 1 : 
          // mbus: cacheable memory
          ptr = &atomicOps_playUnit[coreId];
          expVal = (CEPREGS_BASE_ADDR + cep_scratch0_reg * (coreId*8));      
        break;
      
        case 2 :  
          // pbus
          ptr = (uint64_t *)(CEPREGS_BASE_ADDR + cep_scratch0_reg + (coreId*8));      
          expVal = ~(CEPREGS_BASE_ADDR + cep_scratch0_reg * (coreId*8));
        break;

      } // end switch(l)
      if (!errCnt) { errCnt = cep_runAtomicTest(ptr,expVal); }
    } // end for  

    // Set the core status
    set_status(errCnt, (l << 16) | errCnt);

    // Exit with the error count
    exit(errCnt);
  }

  #ifdef __cplusplus
  }
  #endif
  
#endif // #ifdef BARE_MODE

