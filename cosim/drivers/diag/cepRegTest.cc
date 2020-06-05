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
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_scratch0_reg,(uint64_t)(-1));
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_scratch4_reg,(uint64_t)(-1));
#ifdef SIM_ENV_ONLY    
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_core0_status,(uint64_t)(-1));            
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_core1_status,(uint64_t)(-1));            
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_core2_status,(uint64_t)(-1));            
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_core3_status,(uint64_t)(-1));            
#endif
    
    break;
  }
  case 1: {
    (*regp->AddAReg_p)(regp, aes_base_addr + aes_key0_addr,(uint64_t)(-1));
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_scratch1_reg,(uint64_t)(-1));                
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_scratch5_reg,(uint64_t)(-1));            
    break;
  }    
  case 2: {
    (*regp->AddAReg_p)(regp, aes_base_addr + aes_key1_addr,(uint64_t)(-1));
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_scratch2_reg,(uint64_t)(-1));                
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_scratch6_reg,(uint64_t)(-1));            
    break;
  }
  case 3: {
    (*regp->AddAReg_p)(regp, aes_base_addr + aes_key2_addr,(uint64_t)(-1));
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_scratch3_reg,(uint64_t)(-1));                
    (*regp->AddAReg_p)(regp, reg_base_addr + cep_scratch7_reg,(uint64_t)(-1));            
    break;
  }    
  }
  // now do it
  errCnt = (*regp->doRegTest_p)(regp);
  //
  // Destructors
  //
  cepRegTest_DELETE(regp);
  //
  //
  //
#ifdef SIM_ENV_ONLY
  for (int i=0;i<4;i++) {
    DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, i);
    DUT_WRITE_DVT(DVTF_GET_CORE_STATUS, DVTF_GET_CORE_STATUS, 1);
    uint64_t d64 = DUT_READ_DVT(DVTF_PAT_HI, DVTF_PAT_LO);
    LOGI("CoreId=%d = 0x%016lx\n",i,d64);
  }
#endif
  return errCnt;
}
