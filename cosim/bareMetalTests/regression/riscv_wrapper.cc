//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
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
  int cpuId = read_csr(mhartid);
  set_cur_status(CEP_RUNNING_STATUS);  
  //if (!errCnt) { errCnt = cepregression_test(mask);  }
  int mask = 0; 
  switch (cpuId) {
  case 0:
    mask = ((1 << AES_BASE_K) |
	    (1 << MD5_BASE_K) |
	    (1 << SHA256_BASE_K));
    break;
  case 1:
    mask = ((1 << DFT_BASE_K) |
            (1 << IDFT_BASE_K));
    break;
  case 2:
    mask = ((1 << DES3_BASE_K ) |
	    (1 << GPS_BASE_K) |
	    (1 << CEP_VERSION_REG_K));
    break;
    
  case 3:
    mask = ((1 << FIR_BASE_K) |
	    (1 << IIR_BASE_K));
    break;
  }
  errCnt += run_ceptest(mask);
  
  //
  // ===================================
  // Notify checker about program status
  // ===================================  
  //  
  set_status(errCnt,mask);
  //
  // Stuck here forever...
  //
  exit(errCnt);  
}

#ifdef __cplusplus
}
#endif

#endif
