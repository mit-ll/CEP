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

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepLockTest.h"

#include "cep_apis.h"
#include "portable_io.h"


#ifdef BARE_MODE

#else
#include "simPio.h"
#endif

extern void regBaseTest_Construct(regBaseTest_t *me, 
			   int target,
			   int accessSize,
			   int seed,
			   int verbose
			   );
//
// Overload functions
//
int 
cepLockTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat)
{
  DUT_WRITE32_64(adr,dat);
  return 0;
}

uint64_t  cepLockTest_ReadEntry(regBaseTest_t *me, uint32_t adr) {
  uint64_t d;
  DUT_READ32_64(adr,d);
  return d;
}

//
// =============================
// The test itself
// =============================
//
int cepLockTest_runTest(int cpuId, int accessSize,int loop,int lockNum, int seed, int verbose) {

  int errCnt = 0;
  if (verbose) {
    LOGI("%s: cpuId=%d lockNum=%d seed=%d\n",__FUNCTION__,cpuId,lockNum,seed);
  }
  //
  regBaseTest_t *regp; //
  //
  cepLockTest_CREATE(regp,cpuId, accessSize, seed + (cpuId*100),verbose);
  //
  // add 8 scratch pads
  //
  for (int i=0;i<8;i++) {
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_scratch0_reg + (i*8),(uint64_t)(-1));    
  }
  int maxTO = 50000;
  for (int i=0;i<loop;i++) {
    if (verbose) {
      LOGI("=== Loop=%d ===\n",i);
    }
    // get the lock
    int lockStat = cep_get_lock(cpuId, lockNum, maxTO);
    if (lockStat == 1) { // proceed
      errCnt += (*regp->uniquifyTest_p)(regp,1,0);
      errCnt += (*regp->uniquifyTest_p)(regp,0,1);      
    }
    else { errCnt++; }
    if (errCnt) break;
    // release the lock
    cep_release_lock(cpuId,lockNum);
    // change the seed
    (*regp->SetSeed_p)(regp, seed+0x10);
  }
  //
  // Destructors
  //
  cepLockTest_DELETE(regp);
  return errCnt;
}
//
// Use all 4 locks
//
int cepLockTest_runTest2(int cpuId, int accessSize,int loop,int seed, int verbose) {

  int errCnt = 0;
  if (verbose) {
    LOGI("%s: cpuId=%d seed=%d\n",__FUNCTION__,cpuId,seed);
  }
  //
  regBaseTest_t *regp; //
  //
  cepLockTest_CREATE(regp,cpuId, accessSize, seed + (cpuId*100),verbose);
  //
  int maxTO = 50000;
  int i;
  for (i=0;i<loop;i++) {
    if (verbose) {
      LOGI("=== Loop=%d ===\n",i);
    }
    for (int lockNum=0;lockNum<4;lockNum++) {
      //
      // add 2 scratch pads
      //
      (*regp->ClearAll_p)(regp);
      for (int j=0;j<2;j++) {
	(*regp->AddAReg_p)(regp, reg_base_addr + cep_scratch0_reg + (lockNum * 0x10) + (j*8),(uint64_t)(-1));    
      }
      // get the lock      
      int lockStat = cep_get_lock(cpuId, lockNum, maxTO);
      if (lockStat == 1) { // proceed
	errCnt += (*regp->uniquifyTest_p)(regp,1,0);
	errCnt += (*regp->uniquifyTest_p)(regp,0,1);      
      }
      else { errCnt++; }
      //
      if (errCnt) break;
      // release the lock
      cep_release_lock(cpuId,lockNum);
      // change the seed
      (*regp->SetSeed_p)(regp, seed+(lockNum*0x10)+(i*8));
    }
  }
  //
  // Destructors
  //
  cepLockTest_DELETE(regp);
  return errCnt;
}
