//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
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

#include "portable_io.h"
#include "cepRegTest.h"


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
  //if (!errCnt) { errCnt = cepRegTest_runTest(coreId,64, revCheck, coreId*(0x100), 0); }

  //
  // Written by core0 during boot
  // so read them here to verify core0 did go thru the boot sequrce via scratchpad.
  //
  uint64_t rdDat;
  uint64_t wrDat = scratchpad_know_pattern;
  //
  for (int i=0;i<8;i++) {
    DUT_READ32_64(scratchpad_base_addr + (i*0x100),rdDat);    
    if (rdDat !=  wrDat) {
       errCnt++; 
       break;
    }
    wrDat++;
  }
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
