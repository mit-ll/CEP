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
#include "cep_version.h"
#include "cep_adrMap.h"
#include "cepRegTest.h"
#include "CEP.h"

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
  // LittleEndian
  uint64_t expectedVersion = (((uint64_t)(CEP_MAJOR_VERSION & 0xFF) << 48) |
			      ((uint64_t)(CEP_MINOR_VERSION & 0xFF) << 56));
    
  if (revCheck) {
    (*regp->AddROReg_p)(regp, CEP_VERSION_REG, expectedVersion, CEP_VERSION_MASK);
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

    (*regp->AddAReg_p)(regp, srot_base_addr  + SROT_LLKIC2_SCRATCHPAD0_ADDR, (uint64_t)(-1));
    (*regp->AddAReg_p)(regp, srot_base_addr  + SROT_LLKIC2_SCRATCHPAD1_ADDR, (uint64_t)(-1));
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
  // add some holes to punch for coverage
  /*
  for (int i=0;i<CEP_TOTAL_CORES-1;i++) { // no SRot Yet
    (*regp->AddAHole_p)(regp, cep_core_info[i].base_address  + 0xFFF8,(uint64_t)(-1));
  }
  */
  // from 0x700x_xxxx - 0x7FFx_xxxx
  (*regp->AddAHole_p)(regp, interrupt_controller_base_addr + 0xFF8,(uint64_t)(-1));

  //(*regp->AddAHole_p)(regp, 0x7FFFFFF8,(uint64_t)(-1));
  //
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
