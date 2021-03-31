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
#include <string.h>

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepPlicTest.h"

#include "cep_apis.h"
#include "portable_io.h"

#ifdef SIM_ENV_ONLY
#include "simPio.h"
#endif


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
cepPlicTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat)
{
  DUT_WRITE32_32(adr,dat);
  return 0;
}

uint64_t  cepPlicTest_ReadEntry(regBaseTest_t *me, uint32_t adr) {
  uint32_t d32;
  DUT_READ32_32(adr,d32);
  return (uint64_t)d32;
}

//
// =============================
// The test itself
// =============================
//

int cepPlicTest_runRegTest(int cpuId, int accessSize,int seed, int verbose) {

  int errCnt = 0;
  //
  regBaseTest_t *regp; //
  //
  cepPlicTest_CREATE(regp,cpuId, accessSize, seed + (cpuId*100),verbose);
    
  //
  // start adding register to test
  //
  // there are 127 of them , split into 4 regions one per core
#if 1
  // pick one to run
  if (cpuId == 0) {
    int maxReg = 7; // ONly ?? are implemented!!
    for (int i=1;i<maxReg;i++) {
      (*regp->AddAReg_p)(regp, plic_base_addr + (plic_source1_prio_offset-4) + (cpuId*maxReg*4) + (i*4), 0x7); // only 3 bits
    }
  }
#else
  int start = (cpuId == 0) ? 1 : 0;
  int maxReg = 8; // 128/4 = 32 each except 0
  uint32_t offs;
  for (int i=start;i<maxReg;i++) {
    offs = (plic_source1_prio_offset-4) + (cpuId*maxReg*4) + (i*4);
    LOGI("i=%d offs=0x%x\n",i,offs);
    (*regp->AddAReg_p)(regp, plic_base_addr + (plic_source1_prio_offset-4) + (cpuId*maxReg*4) + (i*4), 0x7); // only 3 bits
  }
#endif
  (*regp->AddAReg_p)(regp, plic_base_addr + plic_hart0_m_mode_ien_start  + (plic_mode_ien_size_offset*cpuId), 0x3E); // only 5 bits
  (*regp->AddAReg_p)(regp, plic_base_addr + plic_hart0_m_mode_prio_thresh  + (plic_prio_thresh_size_offset*cpuId), 0x7); // only 3 bits
  //
  // add a hole in 4K
  //
  (*regp->AddAHole_p)(regp, plic_base_addr + ((plic_base_size-1) & ~0x7),(uint64_t)-1);
  //
  // now do it
  errCnt = (*regp->doRegTest_p)(regp);
  //
  // Destructors
  //
  cepPlicTest_DELETE(regp);
  //
  return errCnt;
}

int cepPlicTest_runTest(int cpuId, int seed, int verbose) {
  int errCnt = 0;
  //
  if (!errCnt) { errCnt += cepPlicTest_runRegTest(cpuId,32, seed, verbose); }
  //
  return errCnt;
}
