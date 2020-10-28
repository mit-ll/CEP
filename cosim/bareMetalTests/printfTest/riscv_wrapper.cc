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
#include "cep_adrMap.h"
#include "cep_apis.h"

#ifdef __cplusplus
extern "C" {
#endif
  
//int main(void)
void thread_entry(int cid, int nc)
{

  unsigned long scratch=0x70000008;
  
  unsigned long adr=0x700f0000;
  unsigned long long rd64;
  int errCnt=0;
  //
  set_printf(1);
  set_cur_status(CEP_RUNNING_STATUS);  
  //
  rd64 = *(unsigned long long*)adr;
  if ((rd64  ^ CEP_EXPECTED_VERSION) & CEP_VERSION_MASK) {
    errCnt++;
    *(unsigned long long*)scratch = 0xBADBAD00BEEFBEEFULL;
  } else {
    errCnt = 0;
    *(unsigned long long*)scratch = 0x600D600DBABEBABEULL;    
  }
  printf("********\n");
  printf("== TEST %s\n",errCnt ? "FAILED" : "PASSED");
  printf("========\n"); 
  //
  //
  set_status(errCnt,0x88);
  //
  // Stuck here forever...
  //
  exit(errCnt);
}

#ifdef __cplusplus
}
#endif
  
#endif
