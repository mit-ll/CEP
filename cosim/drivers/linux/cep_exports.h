//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cep_exports.cc/.h
// Program:        Common Evaluation Platform (CEP)
// Description:    Exports to the Linux Diagnostics
// Notes:          
//
//************************************************************************

#ifndef cep_exports__H_
#define cep_exports__H_

#include <mutex>
#include "cep_run.h"
#include "cepregression.h"

/*
 Function protos
*/
#ifdef __cplusplus
extern "C" {
#endif


    int run_threadTest(void);    
    int cep_aWrite(void);
    int cep_aRead(void);
    int run_cepRegTest(void);
    int run_cepLockTest(void);
    int run_cepMultiLock(void);    
    int run_ddr3Test(void);
    int run_cepAllMacros(void);
    int run_cepMacroMix(void);
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
    int run_cepSrotErrTest(void);
    int run_cepMultiThread(void);

    //
    // DDR3
    //
    int ddr3_aWrite(void);
    int ddr3_aRead(void);  

    //
    // Cache
    //    
    int run_dcacheCoherency(void);
    int run_icacheCoherency(void);
    int run_cacheFlush(void);

    
#ifdef __cplusplus
}
#endif

// ==================
#endif

