//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: MIT
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

#include "cepSrotTest.h"
#include "cepregression.h"


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
  //
  set_printf(0);

  //
  //
  set_cur_status(CEP_RUNNING_STATUS);
  initConfig();
  if (!errCnt) { errCnt = cepSrotTest_runTest(coreId,0,0); }
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
