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
#include "cep_adrMap.h"
#include "cep_apis.h"
#include "CEP.h"
#include "cepregression.h"

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
  int mask = ~((1 << AES_BASE_K) |
	       (1 << RSA_ADDR_BASE_K));
  if (!errCnt) { errCnt = cepregression_test(mask);  }
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
