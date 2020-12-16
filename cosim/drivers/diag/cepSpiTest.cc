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
#include <string.h>

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepSpiTest.h"

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
cepSpiTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat)
{
  DUT_WRITE32_32(adr,dat);
  return 0;
}

uint64_t  cepSpiTest_ReadEntry(regBaseTest_t *me, uint32_t adr) {
  uint32_t d32;
  DUT_READ32_32(adr,d32);
  return (uint64_t)d32;
}

//
// =============================
// The test itself
// =============================
//

int cepSpiTest_runRegTest(int cpuId, int accessSize,int seed, int verbose) {

  int errCnt = 0;
  //
  regBaseTest_t *regp; //
  //
  cepSpiTest_CREATE(regp,cpuId, accessSize, seed + (cpuId*100),verbose);
    
  //
  // start adding register to test
  //
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_sckdiv,    0x00000FFF);
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_sckmode,   0x00000003);
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_csid,      0x00000001);
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_csdef,     0x00000001);
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_csmode,    0x00000003);
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_dcssck,    0x00FF00FF);
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_dintercs,  0x00FF00FF);
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_extradel,  0x000000FF);
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_sampledel, 0x0000001F);
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_fmt,       0x000F000F);
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_txmark,    0x00000007);
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_rxmark,    0x00000007);
  (*regp->AddAReg_p)(regp, spi_base_addr + spi_ie,        0x00000003);
  //(*regp->AddAReg_p)(regp, spi_base_addr + spi_insnmode,  0x00000001);
  //(*regp->AddAReg_p)(regp, spi_base_addr + spi_insnfmt,   0xFFFF3FFF);
  // now do it
  errCnt = (*regp->doRegTest_p)(regp);
  //
  // Destructors
  //
  cepSpiTest_DELETE(regp);
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

int cepSpiTest_runTest(int cpuId, int seed, int verbose) {
  int errCnt = 0;
  if (!errCnt) { errCnt += cepSpiTest_runRegTest(cpuId,32, seed, verbose); }
  return errCnt;
}
