//************************************************************************
<<<<<<< HEAD:cosim/bareMetalTests/csrTest/riscv_wrapper.cc
// Copyright (C) 2020 Massachusetts Institute of Technology
=======
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: MIT
>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0:cosim/bareMetalTests/clintTest/riscv_wrapper.cc
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
//
// For bareMetal mode ONLY
//
#ifdef BARE_MODE
#include "cep_adrMap.h"
#include "cep_apis.h"

<<<<<<< HEAD:cosim/bareMetalTests/csrTest/riscv_wrapper.cc
#include "cepCsrTest.h"
=======
#include "cepClintTest.h"
>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0:cosim/bareMetalTests/clintTest/riscv_wrapper.cc


//#define printf(...) { return 0; }

#ifdef __cplusplus
extern "C" {
#endif
  
//int main(void)
void thread_entry(int cid, int nc)
{
  //
  int errCnt = 0;
  int testId[4] = {0x00,0x11,0x22,0x33};
  int coreId = read_csr(mhartid);
  int revCheck = 1;
  //
  set_printf(0);
  
  //
  //
  set_cur_status(CEP_RUNNING_STATUS);
<<<<<<< HEAD:cosim/bareMetalTests/csrTest/riscv_wrapper.cc
  if (!errCnt) { errCnt = cepCsrTest_runTest(coreId,64, revCheck, coreId*(0x100), 0); }
=======
  if (!errCnt) { errCnt = cepClintTest_runTest(coreId, coreId*(0x100), 0); }
>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0:cosim/bareMetalTests/clintTest/riscv_wrapper.cc
  //
  //
  // Done
  //
  set_status(errCnt,testId[coreId]);
  /*
  if (errCnt) {
    set_pass();
  } else {
    set_fail();
  }
  */
  //
  // Stuck here forever...
  //
  exit(errCnt);
}

#ifdef __cplusplus
}
#endif
  
#endif
