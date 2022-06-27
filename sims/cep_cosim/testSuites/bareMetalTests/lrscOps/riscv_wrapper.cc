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

    uint64_t playUnit;
    uint64_t *ptr;
    uint64_t mySig;
    uint64_t partSig;
    uint64_t incVal;
    uint64_t fixSig[4] = {
      0x12345678ABCD0000ULL,
      0x1122334455660000ULL,
      0x778899aabbcc0000ULL,
      0xddeeff0011220000ULL
    };
    int loop = 16;
    
    set_printf(0);
  
    // Set the current core's status to running
    set_cur_status(CEP_RUNNING_STATUS);

    // ALl core touch this same register
    ptr = &playUnit; // (uint64_t *)(reg_base_addr + cep_scratch0_reg);
    incVal = 4;
    switch (coreId) {
      case 0:
        mySig   = fixSig[0] + 0x0;
        partSig = fixSig[1] + 0x1;  // next coreId
        DUT_WRITE32_64(ptr, mySig); // start for core 0
        break;
      case 1:
        mySig   = fixSig[1] + 0x1;
        partSig = fixSig[2] + 0x2; // next coreId
        break;
      case 2:
        mySig   = fixSig[2] + 0x2;
        partSig = fixSig[3] + 0x3; // next coreId
        break;
      case 3:
        mySig   = fixSig[3] + 0x3;
        partSig = fixSig[0] + 0x4; // next coreId
        break;
    }
  
    errCnt = cep_exchAtomicTest(coreId, ptr, mySig, partSig, incVal, loop);

    // Set the core status
    set_status(errCnt, testId[coreId]);

    // Exit with the error count
    exit(errCnt);
  }

  #ifdef __cplusplus
  }
  #endif
  
#endif // #ifdef BARE_MODE
