//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepMacroMix.h"

#include "cep_apis.h"
#include "portable_io.h"

#include "cepregression.h"

#ifdef BARE_MODE

#else
#include "simPio.h"
#endif

//
// =============================
// The test itself
// =============================
//
int cepMacroMix_runTest(int cpuId, int mask, int seed, int verbose) {
  //
  int errCnt = 0;
  int maxTests = 10;
  //
  //
  // randomly pick a test to run base on seed and cpuId
  // and make sure no more than one core run the same test
  //
  int test2run = (cpuId + (seed % maxTests)) % MAX_CORES;
  if (((1 <<  test2run) & mask) == 0) { return errCnt; }
  if (verbose) {
    LOGI("Runnning test=%d\n",test2run);
  }
  //
  //
  //
  switch (test2run) {
  case 0 : {  errCnt += cep_AES_test(); break; }
  case 1 : {  errCnt += cep_DES3_test(); break; }
  case 2 : {  errCnt += cep_DFT_test(); break; }
  case 3 : {  errCnt += cep_IDFT_test(); break; }
  case 4 : {  errCnt += cep_FIR_test(); break; }
  case 5 : {  errCnt += cep_IIR_test() ; break; }
  case 6 : {  errCnt += cep_GPS_test() ; break; }
  case 7 : {  errCnt += cep_MD5_test(); break; }
  case 8 : {  errCnt += cep_RSA_test(); break; }
  case 9 : {  errCnt += cep_SHA256_test(); break; }
  }
  // else do nothing
  return errCnt;
}
