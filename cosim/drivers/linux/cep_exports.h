//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef cep_exports__H_
#define cep_exports__H_

#include <mutex>
#include "cep_run.h"
#include "cepregression.h"
//
// ============================
// Function prototypes
// ============================

//    printf("C%d", sched_getcpu(), format, ##__VA_ARGS__);		
//#define eprintf(format, …) fprintf (stderr, format, ##__VA_ARGS__)

/*
 Function protos
*/
#ifdef __cplusplus
extern "C" {
#endif
  //
  // Common/Utils
  //
  int run_threadTest(void);    
  //
  // CEP
  //
  int cep_aWrite(void);
  int cep_aRead(void);
  int run_cepRegTest(void);
  int run_cepLockTest(void);
  int run_cepMultiLock(void);    
  int run_ddr3Test(void);
  int run_cepAllMacros(void);
  int run_cepMacroMix(void);
  // misc
  int run_cepGpioTest(void);
  int run_cepSrotMemTest(void);
  int run_cepSrotMaxKeyTest(void);
  int run_cepMacroBadKey(void);
  int run_smemTest(void);
  int run_cepLockfreeAtomic(void);
  int run_cepLrscOps(void);
  int run_cepClintTest(void);
  int run_cepPlicTest(void);
  int run_cepAccessTest(void);
  int run_cepAtomicTest(void);
  int run_cepSpiTest(void);
  int run_cepMaskromTest(void);
  //  int run_cepCsrTest(void);
  int run_cepSrotErrTest(void);
  //
  int run_cep_AES      (void);  
  int run_cep_DES3     (void);  
  int run_cep_DFT      (void);  
  int run_cep_FIR      (void);  
  int run_cep_IIR      (void);  
  int run_cep_GPS      (void);  
  int run_cep_MD5      (void);  
  int run_cep_RSA      (void);  
  int run_cep_SHA256   (void);  
  //
  // DDR3
  //
  int ddr3_aWrite(void);
  int ddr3_aRead(void);  

  //
  // cores
  //
  int run_dcacheCoherency(void);
  int run_icacheCoherency(void);
  int run_cacheFlush(void);
#ifdef __cplusplus
}
#endif

// ==================
#endif

