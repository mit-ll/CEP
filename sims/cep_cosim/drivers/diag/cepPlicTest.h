//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef cepPlicTest_H
#define cepPlicTest_H

#include "regBaseTest.h"

//
// Constructor/Destructor
//
#define cepPlicTest_CREATE(me,target,accessSize,seed,verbose)	\
  {REGBASETEST_CREATE(me,target,accessSize,seed,verbose);		\
    me->WriteReg_p = cepPlicTest_WriteEntry;				\
    me->ReadReg_p  = cepPlicTest_ReadEntry;}


#define cepPlicTest_DELETE(me) REGBASETEST_DELETE(me)


//
// Overload functions
//
int cepPlicTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat);
uint64_t  cepPlicTest_ReadEntry(regBaseTest_t *me, uint32_t adr);
//
// The test itself
//
int cepPlicTest_runTest(int cpuId, int seed, int verbose);

int cepPlicTest_runRegTest(int cpuId, int accessSize,int seed, int verbose);

//int cepPlicTest_intrTest(int coreId, int verbose);
int cepPlicTest_prioIntrTest(int coreId, int verbose);

#endif
