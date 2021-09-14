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
#include <string.h>

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepClintTest.h"

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
cepClintTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat)
{
  DUT_WRITE32_64(adr,dat);
  return 0;
}

uint64_t  cepClintTest_ReadEntry(regBaseTest_t *me, uint32_t adr) {
  uint64_t d64;
  DUT_READ32_64(adr,d64);
  return (uint64_t)d64;
}

//
// =============================
// The test itself
// =============================
//

int cepClintTest_runRegTest(int cpuId, int accessSize,int seed, int verbose) {

  int errCnt = 0;
  //
  regBaseTest_t *regp; //
  //
  cepClintTest_CREATE(regp,cpuId, accessSize, seed + (cpuId*100),verbose);
    
  //
  // start adding register to test
  //
  (*regp->AddAReg_p)(regp, clint_base_addr + clint_mtimecmp_offset + (cpuId*8), (uint64_t)-1);
#ifdef SIM_ENV_ONLY
  if (cpuId < 2) { // 0-1 will cover for 2 and 3 since this register is 32 bit only and only 1 bits
    (*regp->AddAReg_p)(regp, clint_base_addr + clint_msip_offset     + (cpuId*8), (uint64_t)0x0000000100000001LL);
  }
#endif
  //
  // add a hole in 4K
  //
  (*regp->AddAHole_p)(regp, clint_base_addr + 0xFFF8,(uint64_t)-1);
  //
  // now do it
  errCnt = (*regp->doRegTest_p)(regp);
  //
  // Destructors
  //
  cepClintTest_DELETE(regp);
  //
  return errCnt;
}

int cepClintTest_runTest(int cpuId, int seed, int verbose) {
  int errCnt = 0;
  //
  if (!errCnt) { errCnt += cepClintTest_runRegTest(cpuId,64, seed, verbose); }
  //
  return errCnt;
}
