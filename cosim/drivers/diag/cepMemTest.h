//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef cepMemTest_H
#define cepMemTest_H

#include "memBaseTest.h"

//
// Constructor/Destructor
//
#define cepMemTest_CREATE(me,cpuId,baseAdr,adrSize,datSize,step,seed)	\
  {MEMBASETEST_CREATE(me,baseAdr,adrSize,datSize,step,seed);		\
    me->mTarget = cpuId;						\
    me->WriteEntry_p = cepMemTest_WriteEntry;				\
    me->ReadEntry_p  = cepMemTest_ReadEntry;}


#define cepMemTest_DELETE(me) MEMBASETEST_DELETE(me)


//
// Overload functions
//
int cepMemTest_WriteEntry(memBaseTest_t *me, uint64_t adr);
int cepMemTest_ReadEntry(memBaseTest_t *me, uint64_t adr);
//
// The test itself
//
int cepMemTest_runTest(int cpuId,uint64_t mem_base, int adrWidth,int seed, int verbose, int full);

#endif
