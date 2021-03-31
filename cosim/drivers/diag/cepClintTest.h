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

#ifndef cepClintTest_H
#define cepClintTest_H

#include "regBaseTest.h"

//
// Constructor/Destructor
//
#define cepClintTest_CREATE(me,target,accessSize,seed,verbose)	\
  {REGBASETEST_CREATE(me,target,accessSize,seed,verbose);		\
    me->WriteReg_p = cepClintTest_WriteEntry;				\
    me->ReadReg_p  = cepClintTest_ReadEntry;}


#define cepClintTest_DELETE(me) REGBASETEST_DELETE(me)


//
// Overload functions
//
int cepClintTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat);
uint64_t  cepClintTest_ReadEntry(regBaseTest_t *me, uint32_t adr);
//
// The test itself
//
int cepClintTest_runTest(int cpuId, int seed, int verbose);

int cepClintTest_runRegTest(int cpuId, int accessSize,int seed, int verbose);

#endif
