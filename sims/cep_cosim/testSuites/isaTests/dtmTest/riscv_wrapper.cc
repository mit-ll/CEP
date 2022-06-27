//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
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

#include "cepRegTest.h"
#include "portable_io.h"
#include "portable_io.h"

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

    // Do nothing but keep
int done = 0;
uint64_t dat64 = coreId;
uint64_t rdat64;
while (!done) {
  DUT_WRITE32_64(cep_scratch0_reg + (coreId * 8),dat64);
  dat64 += 1 + coreId;
    //
  DUT_READ32_64(cep_core0_status + (coreId * 8),rdat64);
if (rdat64 == CEP_GOOD_STATUS) done=1;
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
