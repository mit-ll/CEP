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

#include "cepRegTest.h"
#include "cepLockfreeAtomic.h"
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
  int cpuId = read_csr(mhartid);
  set_printf(0);  
  //
  set_cur_status(CEP_RUNNING_STATUS);  
  //
  errCnt += cepLockfreeAtomic_runTest(cpuId, 0, 0);
  //
  // Done
  //
  set_status(errCnt,errCnt);
  //
  // Stuck here forever...
  //
  exit(errCnt);
}

#ifdef __cplusplus
}
#endif
  
#endif
