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
#include "cepGpioTest.h"

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
cepGpioTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat)
{
  DUT_WRITE32_32(adr,dat);
  return 0;
}

uint64_t  cepGpioTest_ReadEntry(regBaseTest_t *me, uint32_t adr) {
  uint32_t d32;
  DUT_READ32_32(adr,d32);
  return (uint64_t)d32;
}

//
// =============================
// The test itself
// =============================
//

int cepGpioTest_runRegTest(int cpuId, int accessSize,int seed, int verbose) {

  int errCnt = 0;
  //
  regBaseTest_t *regp; //
  //
  cepGpioTest_CREATE(regp,cpuId, accessSize, seed + (cpuId*100),verbose);
    
  //
  // start adding register to test
  //
  // only 4 bits
  //
  //  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_pin		, 0xF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_input_en	, 0xF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_output_en	, 0xF);
  //  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_port_output	, 0xF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_pue		, 0xF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_ds		, 0xF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_rise_ie	, 0xF);
  //  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_rise_ip	, 0xF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_fall_ie	, 0xF);
  //(*regp->AddAReg_p)(regp, gpio_base_addr + gpio_fall_ip	, 0xF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_high_ie	, 0xF);
  //(*regp->AddAReg_p)(regp, gpio_base_addr + gpio_high_ip	, 0xF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_low_ie		, 0xF);
  //(*regp->AddAReg_p)(regp, gpio_base_addr + gpio_low_ip		, 0xF);
  //  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_iof_en		, 0xF);
  //(*regp->AddAReg_p)(regp, gpio_base_addr + gpio_iof_sel	, 0xF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_out_xor	, 0xF);
  //
  (*regp->AddAHole_p)(regp, gpio_base_addr + 0xFFC	, 0xFFFFFFFF);
  // now do it
  errCnt = (*regp->doRegTest_p)(regp);
  //
  // Destructors
  //
  cepGpioTest_DELETE(regp);
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

int cepGpioTest_runTest(int cpuId, int seed, int verbose) {
  int errCnt = 0;
  if (!errCnt) { errCnt += cepGpioTest_runRegTest(cpuId,32, seed, verbose); }
  return errCnt;
}
