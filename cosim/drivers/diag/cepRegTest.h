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

#ifndef cepRegTest_H
#define cepRegTest_H

#include "regBaseTest.h"

//
// Constructor/Destructor
//
#define cepRegTest_CREATE(me,target,accessSize,seed,verbose)	\
  {REGBASETEST_CREATE(me,target,accessSize,seed,verbose);		\
    me->WriteReg_p = cepRegTest_WriteEntry;				\
    me->ReadReg_p  = cepRegTest_ReadEntry;}


#define cepRegTest_DELETE(me) REGBASETEST_DELETE(me)


//
// Overload functions
//
int cepRegTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat);
uint64_t  cepRegTest_ReadEntry(regBaseTest_t *me, uint32_t adr);
//
// The test itself
//
int cepRegTest_runTest(int cpuId, int accessSize,int revCheck, int seed, int verbose);

#endif
