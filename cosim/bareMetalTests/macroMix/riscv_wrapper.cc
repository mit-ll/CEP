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
#include <stdlib.h>
#include <stdio.h>
#include "CEP.h"
#include "cep_adrMap.h"
#include "cep_apis.h"
#include "cepregression.h"
#include "cepMacroMix.h"

//
#include "md5_playback.h"
#include "sha256_playback.h"
#include "iir_playback.h"
#include "fir_playback.h"

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
  set_cur_status(CEP_RUNNING_STATUS);  
  //
  // ===================================
  // Call your program here
  // ===================================  
  //
  //int testId = 0x88;
  int coreId = read_csr(mhartid);
  uint64_t upper, lower;
  //initConfig(); // all 4 cores do the same????
  //
  if (coreId == 0) {
    init_md5();
    upper = md5_adrBase + md5_adrSize;
    lower = md5_adrBase;
    errCnt += cep_playback(md5_playback, upper, lower, md5_totalCommands, md5_size, 0);    
  }
  else if (coreId == 1) {
    init_sha256();
    upper = sha256_adrBase + sha256_adrSize;
    lower = sha256_adrBase;
    errCnt += cep_playback(sha256_playback, upper, lower, sha256_totalCommands, sha256_size, 0);    
  }
  else if (coreId == 2) {
    init_iir();
    upper = iir_adrBase + iir_adrSize;
    lower = iir_adrBase;
    errCnt += cep_playback(iir_playback, upper, lower, iir_totalCommands, iir_size, 0);    
  }
  else if (coreId == 3) {
    init_fir();
    upper = fir_adrBase + fir_adrSize;
    lower = fir_adrBase;
    errCnt += cep_playback(fir_playback, upper, lower, fir_totalCommands, fir_size, 0);    
  }  
  //
  // ===================================
  // Notify checker about program status
  // ===================================  
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
