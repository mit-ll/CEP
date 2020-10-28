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
#include "CEP.h"
#include "cep_adrMap.h"
#include "cep_apis.h"
#include "cepregression.h"
#include "cepMacroMix.h"

//
#include "des3_playback.h"
#include "gps_playback.h"
#include "aes_playback.h"
#include "dft_playback.h"

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
  uint64_t upper, lower;
  set_printf(printfOn);
  set_cur_status(CEP_RUNNING_STATUS);  
  //
  // ===================================
  // Call your program here
  // ===================================  
  //
  //int testId = 0x88;
  int coreId = read_csr(mhartid);
  //initConfig(); // all 4 cores do the same????
  //
  if (coreId == 0) {
    init_des3();
    upper = des3_adrBase + des3_adrSize;
    lower = des3_adrBase;
    errCnt += cep_playback(des3_playback, upper, lower, des3_totalCommands, des3_size, 0);    
  }
  else if (coreId == 1) {
    init_gps();
    upper = gps_adrBase + gps_adrSize;
    lower = gps_adrBase;
    errCnt += cep_playback(gps_playback, upper, lower, gps_totalCommands, gps_size, 0);    
  }
  else if (coreId == 2) {
    init_aes();
    upper = aes_adrBase + aes_adrSize;
    lower = aes_adrBase;
    errCnt += cep_playback(aes_playback, upper, lower, aes_totalCommands, aes_size, 0);    
  }
  else if (coreId == 3) {
    init_dft();
    init_idft();
    upper = dft_adrBase + dft_adrSize + dft_adrSize; // include IDFT
    lower = dft_adrBase;
    errCnt += cep_playback(dft_playback, upper, lower, dft_totalCommands, dft_size, 0);    
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
