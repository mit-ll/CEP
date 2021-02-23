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

#ifndef cepGpioTest_H
#define cepGpioTest_H

#include "regBaseTest.h"

//
// Constructor/Destructor
//
#define cepGpioTest_CREATE(me,target,accessSize,seed,verbose)	\
  {REGBASETEST_CREATE(me,target,accessSize,seed,verbose);		\
    me->WriteReg_p = cepGpioTest_WriteEntry;				\
    me->ReadReg_p  = cepGpioTest_ReadEntry;}


#define cepGpioTest_DELETE(me) REGBASETEST_DELETE(me)


//
// Overload functions
//
int cepGpioTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat);
uint64_t  cepGpioTest_ReadEntry(regBaseTest_t *me, uint32_t adr);
//
// The test itself
//
int cepGpioTest_runTest(int cpuId, int seed, int verbose);
int cepGpioTest_runRegTest(int cpuId, int accessSize,int seed, int verbose);

#endif
