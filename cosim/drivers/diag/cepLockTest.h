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

#ifndef cepLockTest_H
#define cepLockTest_H

#include "regBaseTest.h"

//
// Constructor/Destructor
//
#define cepLockTest_CREATE(me,target,accessSize,seed,verbose)	\
  {REGBASETEST_CREATE(me,target,accessSize,seed,verbose);		\
    me->WriteReg_p = cepLockTest_WriteEntry;				\
    me->ReadReg_p  = cepLockTest_ReadEntry;}


#define cepLockTest_DELETE(me) REGBASETEST_DELETE(me)


//
// Overload functions
//
int cepLockTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat);
uint64_t  cepLockTest_ReadEntry(regBaseTest_t *me, uint32_t adr);
//
// The test itself
//
int cepLockTest_runTest(int cpuId, int accessSize,int loop, int lockNum, int seed, int verbose);
int cepLockTest_runTest2(int cpuId, int accessSize,int loop, int seed, int verbose);

#endif
