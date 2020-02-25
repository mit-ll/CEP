//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepMemTest.h"

#include "cep_apis.h"
#include "portable_io.h"


#ifdef BARE_MODE

#else
#include "simPio.h"
#endif

extern void memBaseTest_Construct         (memBaseTest_t *me, 
					   uint64_t baseAdr,
					   int adrSize,
					   int datSize,
					   int step,
					   int seed);

//
// Overload functions
//
int 
cepMemTest_WriteEntry(memBaseTest_t *me, uint64_t adr)
{
  uint64_t dat64 = (uint64_t)me->wrPat[0];
  if (GET_VERBOSE(me)) { 
    LOGI("%s cpu=%d adr=%016lx dat=%016lx\n",__FUNCTION__,me->mTarget,adr,dat64);
  }
  DDR3_WRITE(me->mBAR + (adr*8),dat64);
  return 0;
}

int
cepMemTest_ReadEntry(memBaseTest_t *me, uint64_t adr)
{
  uint64_t dat64;
  if (GET_VERBOSE(me)) { 
    LOGI("%s cpu=%d adr=%016lx\n",__FUNCTION__,me->mTarget,adr); 
  }
  DDR3_READ(me->mBAR + (adr*8),dat64);
  me->rdPat[0] = dat64 ;
  //
  return 0;
}
//
// =============================
// The test itself
// =============================
//
int cepMemTest_runTest(int cpuId,uint64_t mem_base, int adrWidth,int seed, int verbose, int full) {

  int errCnt = 0;
  //
  memBaseTest_t *memp; //
  //
  //
  // constructors
  // 1Gbytes??
  //
  int step = (full) ? 1 : 0x10000;
  //int adrWidth = 30-2; // 1 Gbytes (30bits) for all 4 cores => each one will test 1/4 of that
  //uint64_t mem_base = ddr3_base_adr + ((cpuId&0x3) << adrWidth);  
  cepMemTest_CREATE(memp,cpuId,mem_base, adrWidth-3, 64, step, seed + (cpuId*100));
  //
  // uniquify
  //
  // write
  // flip the order just for fun :-))  
  if (memp->mTarget & 0x1) {
    if (!errCnt) { errCnt += (*memp->walk_1_thruAdrTest_p)(memp,1,0); }
    if (!errCnt && full) { errCnt += (*memp->walk_1_thruDatTest_p)(memp,1,0); }
  } else {
    if (!errCnt && full) { errCnt += (*memp->walk_0_thruDatTest_p)(memp,1,0); }        
    if (!errCnt) { errCnt += (*memp->walk_1_thruAdrTest_p)(memp,1,0); }
  }
  //
  // readNcompare
  //
  if (memp->mTarget & 0x1) {
    if (!errCnt) { errCnt += (*memp->walk_1_thruAdrTest_p)(memp,0,1); }
    if (!errCnt && full) { errCnt += (*memp->walk_1_thruDatTest_p)(memp,0,1); }
  } else {
    if (!errCnt && full) { errCnt += (*memp->walk_0_thruDatTest_p)(memp,0,1); }        
    if (!errCnt) { errCnt += (*memp->walk_1_thruAdrTest_p)(memp,0,1); }
  }
  //
  // Destructors
  //
  cepMemTest_DELETE(memp);
  return errCnt;
}
