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

#include "cepMemTest.h"
#include "CEP.h"
//#include "cepregression.h"


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
  int full = 0;
  int verbose = 0;
  int seed = 0;
  //
  set_printf(0);
  uint32_t mem_base;
  int adrWidth ;
  int dataWidth = 64;
  //
  //
  set_cur_status(CEP_RUNNING_STATUS);
  //
  // disable operational bits
  //DUT_WRITE32_64(srot_base_addr + SROT_CTRLSTS_ADDR , 0);
  //
  //
  //
  for (int i=0;i<2;i++) {
    if (i == 0) {
      adrWidth = 8; // 256 locations
      mem_base = srot_base_addr + SROT_KEYRAM_ADDR;
    } else {
      adrWidth = 6; // 32 locations
      mem_base = srot_base_addr + SROT_KEYINDEXRAM_ADDR;
    }
    if (!errCnt) { errCnt = cepMemTest_runTest(coreId,mem_base,adrWidth,dataWidth ,seed, verbose, full); }
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
