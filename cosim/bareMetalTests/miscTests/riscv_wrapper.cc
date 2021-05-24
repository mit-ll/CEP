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
#include <stdlib.h>
#include "cep_adrMap.h"
#include "cep_apis.h"
#include "portable_io.h"
#include "cepUartTest.h"
#include "cepSpiTest.h"
#include "cepGpioTest.h"
#include "cepMaskromTest.h"

#ifdef __cplusplus
extern "C" {
#endif

#define flush_page(addr) asm volatile ("sfence.vma %0" : : "r" (addr) : "memory")

//int main(void)
void thread_entry(int cid, int nc)
{
  int errCnt = 0;
  int stat = 0;
  int cpuId = read_csr(mhartid);
  set_printf(0);

  switch (cpuId) {
  case 3 :  errCnt += cepUartTest_runTest(cpuId,0x10, 0); break;
  case 1 :  errCnt += cepSpiTest_runTest(cpuId,0x20, 0); break;
  case 2 :  errCnt += cepGpioTest_runTest(cpuId,0x30,0); break;
  case 0 :  errCnt += cepMaskromTest_runTest(cpuId,0x40,0); break;
  }

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
