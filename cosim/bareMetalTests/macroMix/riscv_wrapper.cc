//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
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
#include <stdlib.h>
#include <stdio.h>
#include "CEP.h"
#include "cep_adrMap.h"
#include "cep_apis.h"
#include "cepregression.h"
#include "cepMacroMix.h"

//
// =================================================
// Bare metal Mode
// =================================================
//
#ifdef __cplusplus
extern "C" {
#endif

//int main(void)
void thread_entry(int cid, int nc)
{
  int errCnt = 0;
  int printfOn = 0;
  set_printf(printfOn);  
  //
  // ===================================
  // Call your program here
  // ===================================  
  //
  int testId = 0x88;
  int coreId = read_csr(mhartid);
  //initConfig(); // all 4 cores do the same????
  //
  int mask = ~((1 << AES_BASE_K) |
	       (1 << RSA_ADDR_BASE_K));  
  if (!errCnt) { errCnt = cepMacroMix_runTest(coreId, mask, 0, 0);  }
  //
  // ===================================
  // Notify checker about program status
  // ===================================  
  //  
  set_status(errCnt,testId);
  //
  // Stuck here forever...
  //
  exit(errCnt);  
}

#ifdef __cplusplus
}
#endif

#endif
