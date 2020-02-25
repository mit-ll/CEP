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
#include "cepRegTest.h"

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
cepRegTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat)
{
  DUT_WRITE32_64(adr,dat);
  return 0;
}

uint64_t  cepRegTest_ReadEntry(regBaseTest_t *me, uint32_t adr) {
  uint64_t d;
  DUT_READ32_64(adr,d);
  return d;
}

//
// =============================
// The test itself
// =============================
//
int cepRegTest_runTest(int cpuId, int accessSize,int revCheck,int seed, int verbose) {

  int errCnt = 0;
  //
  regBaseTest_t *regp; //
  //
  cepRegTest_CREATE(regp,cpuId, accessSize, seed + (cpuId*100),verbose);
    
  //
  // start adding register to test
  //
  if (revCheck) {
    (*regp->AddROReg_p)(regp, CEP_VERSION_REG, CEP_EXPECTED_VERSION,CEP_VERSION_MASK);
  }
  switch (cpuId) {
  case 0: {
    (*regp->AddAReg_p)(regp, aes_base_addr + aes_pt0_addr,(uint64_t)(-1));
    (*regp->AddAReg_p)(regp, aes_base_addr + aes_pt1_addr,(uint64_t)(-1));        
    break;
  }
  case 1: {
    (*regp->AddAReg_p)(regp, aes_base_addr + aes_key0_addr,(uint64_t)(-1));
    break;
  }    
  case 2: {
    (*regp->AddAReg_p)(regp, aes_base_addr + aes_key1_addr,(uint64_t)(-1));
    break;
  }
  case 3: {
    (*regp->AddAReg_p)(regp, aes_base_addr + aes_key2_addr,(uint64_t)(-1));
    break;
  }    
  }
  // now do it
  errCnt = (*regp->doRegTest_p)(regp);
  //
  // Destructors
  //
  cepRegTest_DELETE(regp);
  return errCnt;
}
