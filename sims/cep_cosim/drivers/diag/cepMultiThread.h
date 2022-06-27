//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef cepMultiThread_H
#define cepMultiThread_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
  int cepMultiThread_findATest2Run(int thrId, uint64_t testLockPtr, int doneMask, int maxTest, int verbose);
  int cepMultiThread_releaseTestLock(int thrId, int testId, uint64_t testLockPtr);
  int cepMultiThread_setup(int cpuId, uint64_t testLockPtr, int coreMask, int maxTest, int cpuActiveMask, int verbose);
  int cepMultiThread_runThr(int thrId, uint64_t testLockPtr, int coreMask, int maxTest, int maxLoop, int seed, int verbose);

#ifdef __cplusplus
    }
#endif

#endif
