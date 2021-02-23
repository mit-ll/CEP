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
  // hole
  (*regp->AddAHole_p)(regp, spi_base_addr + 0xFFC,        0xFFFFFFFF);
  //
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
//
// Do some SPI transaction
//
int cep_setup_spi(int csId, int mode, int divisor, int En) 
{
  int errCnt = 0;
  u_int32_t dat;
  /*
  DUT_WRITE32_32(spi_base_addr + spi_insnmode, 0); // FIFO mode
  DUT_WRITE32_32(spi_base_addr + spi_csid, 1 << csId);
  DUT_WRITE32_32(spi_base_addr + spi_csdef, 1 << csId); // active low for selected chip
  DUT_WRITE32_32(spi_base_addr + spi_csmode, 0); // auto
  DUT_WRITE32_32(spi_base_addr + spi_dcssck, 0x0101); // delay = default
  DUT_WRITE32_32(spi_base_addr + spi_dintercs, 0x0101); // delay = default
  DUT_WRITE32_32(spi_base_addr + spi_fmt , 0x00080008); // default
  DUT_WRITE32_32(spi_base_addr + spi_ie, En); // interrupt enable
  */
  //
  return errCnt;
}

//
// Run read/write test
//
// assuming SPI is loopback MOSI -> MISO
int cepSpiRdWrTest(int csId, int seed, int verbose) {
  int errCnt = 0;
  int dat, act;
  if (verbose) {
    LOGI("%s\n",__FUNCTION__);
  }
  //
  DUT_WRITE32_32(spi_base_addr + spi_txmark, 1);
  DUT_WRITE32_32(spi_base_addr + spi_rxmark, 1);
  DUT_WRITE32_32(spi_base_addr + spi_ie, 0x3); // interrupt enable
  DUT_WRITE32_32(spi_base_addr + spi_insnmode, 0); // enable
  DUT_WRITE32_32(spi_base_addr + spi_sckmode, 3) ; // OFF mode);
  DUT_WRITE32_32(spi_base_addr + spi_sckdiv,6);
  //
  // write to txFIFO and should get back what we write via rxfifo
  //
  for (int i=0;i<16;i++) {
    dat = 1 << (i & 0x7);
    if (i > 7) dat = ~dat & 0xFF;
    DUT_WRITE32_32(spi_base_addr + spi_txfifo, dat);
    //
    // Read rxFifo
    //
    int to = 300;
    int act = 0;
    int bCnt = 0;
    do {
      DUT_READ32_32(spi_base_addr + spi_rxfifo, act);
      if ((act & (1 << 31)) == 0) {
	if (((act ^ dat) & 0xff) != 0) {
	  LOGE("%s: miscompare act=0x%02x exp=0x%02x\n",__FUNCTION__,act,dat);
	  errCnt = (0xaa << 24) | (act << 16) | (dat << 8) | i;
	} else if (verbose) {
	  LOGI("%s: Got a byte = 0x%02x \n",__FUNCTION__,act);
	}
	break;
      }
      to--;
      if (to <= 0) {
	LOGE("%s: ERROR timeout\n",__FUNCTION__);
	errCnt++;
	break;
      }
    } while (to > 0);
    if (errCnt != 0) break;
  }
  return errCnt;
}

// the test
int cepSpiTest_runTest(int cpuId, int seed, int verbose) {
  int errCnt = 0;
  //
  if (!errCnt) { errCnt += cepSpiRdWrTest(0, seed, verbose); }
  if (!errCnt) { errCnt += cepSpiTest_runRegTest(cpuId,32, seed, verbose); }
  return errCnt;
}
