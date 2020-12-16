//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef cepUartTest_H
#define cepUartTest_H

#include "regBaseTest.h"

//
// Constructor/Destructor
//
#define cepUartTest_CREATE(me,target,accessSize,seed,verbose)	\
  {REGBASETEST_CREATE(me,target,accessSize,seed,verbose);		\
    me->WriteReg_p = cepUartTest_WriteEntry;				\
    me->ReadReg_p  = cepUartTest_ReadEntry;}


#define cepUartTest_DELETE(me) REGBASETEST_DELETE(me)


//
// Overload functions
//
int cepUartTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat);
uint64_t  cepUartTest_ReadEntry(regBaseTest_t *me, uint32_t adr);
//
// The test itself
//
int cepUartTest_runTest(int cpuId, int seed, int verbose);

int cepUartTest_runRegTest(int cpuId, int accessSize,int seed, int verbose);
int cepUartTest_runTxRxTest(int cpuId, int divisor, char *txStr, int txLen, int stopBits, int verbose);

#endif
