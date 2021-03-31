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

#ifndef cepSpiTest_H
#define cepSpiTest_H

#include "regBaseTest.h"

//
// Constructor/Destructor
//
#define cepSpiTest_CREATE(me,target,accessSize,seed,verbose)	\
  {REGBASETEST_CREATE(me,target,accessSize,seed,verbose);		\
    me->WriteReg_p = cepSpiTest_WriteEntry;				\
    me->ReadReg_p  = cepSpiTest_ReadEntry;}


#define cepSpiTest_DELETE(me) REGBASETEST_DELETE(me)


//
// Overload functions
//
int cepSpiTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat);
uint64_t  cepSpiTest_ReadEntry(regBaseTest_t *me, uint32_t adr);
//
// The test itself
//
int cepSpiTest_runTest(int cpuId, int seed, int verbose);
int cepSpiTest_runRegTest(int cpuId, int accessSize,int seed, int verbose);

#endif
