//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef cepCsrTest_H
#define cepCsrTest_H

#include "regBaseTest.h"

//
// Constructor/Destructor
//
#define cepCsrTest_CREATE(me,target,accessSize,seed,verbose)	\
  {REGBASETEST_CREATE(me,target,accessSize,seed,verbose);		\
    me->WriteReg_p = cepCsrTest_WriteEntry;				\
    me->ReadReg_p  = cepCsrTest_ReadEntry;}


#define cepCsrTest_DELETE(me) REGBASETEST_DELETE(me)


//
// Overload functions
//
int cepCsrTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat);
uint64_t  cepCsrTest_ReadEntry(regBaseTest_t *me, uint32_t adr);
//
// The test itself
//
int cepCsrTest_runTest(int cpuId, int accessSize,int revCheck, int seed, int verbose);

#endif
