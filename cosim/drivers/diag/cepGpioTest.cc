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
  // as of 03/31/21 = 8 bits, true GPIO
  //  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_pin		, 0xFF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_input_en	, 0xFF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_output_en	, 0xFF);
  //  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_port_output	, 0xFF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_pue		, 0xFF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_ds		, 0xFF);
  //(*regp->AddAReg_p)(regp, gpio_base_addr + gpio_low_ip		, 0xFF);
  //(*regp->AddAReg_p)(regp, gpio_base_addr + gpio_iof_en		, 0xFF);
  //(*regp->AddAReg_p)(regp, gpio_base_addr + gpio_iof_sel	, 0xFF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_out_xor	, 0xFF);
  //
  (*regp->AddAHole_p)(regp, gpio_base_addr + 0xFFC	, 0xFFFFFFFF);
  //
#ifdef SIM_ENV_ONLY
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_rise_ie	, 0xFF);
  //  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_rise_ip	, 0xFF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_fall_ie	, 0xFF);
  //(*regp->AddAReg_p)(regp, gpio_base_addr + gpio_fall_ip	, 0xFF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_high_ie	, 0xFF);
  //(*regp->AddAReg_p)(regp, gpio_base_addr + gpio_high_ip	, 0xFF);
  (*regp->AddAReg_p)(regp, gpio_base_addr + gpio_low_ie		, 0xFF);
#endif

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

//
int cepGpioTest_intrTest(int cpuId, int seed, int verbose) {
    int errCnt = 0;
    uint32_t rdat32, wdat32, offs;
        // setup
    DUT_WRITE32_32(gpio_base_addr + gpio_iof_en         , 0x00); // no IOF
    DUT_WRITE32_32(gpio_base_addr + gpio_pue            , 0x00); // no internal pull up           
    DUT_WRITE32_32(gpio_base_addr + gpio_output_en	, 0xFF);
    DUT_WRITE32_32(gpio_base_addr + gpio_input_en	, 0xFF);
    DUT_WRITE32_32(gpio_base_addr + gpio_high_ie        , 0xFF);    
        // 
        // Each GPIO
        //
    for (int i=0;i<8;i++) {
            // walking 1
        wdat32 = 1 << i;
        DUT_WRITE32_32(gpio_base_addr + gpio_port_output, wdat32);
            // loop back
        offs = gpio_base_addr + gpio_pin;
            //
        DUT_READ32_32 (offs, rdat32);
        if (rdat32 != wdat32) {
            LOGE("ERROR: Mismatch offs=0x%08x i=%d exp=0x%08x act=0x%08x\n",offs,i,wdat32,rdat32);
            errCnt++;
            break;
        }
        else if (verbose) {
            LOGI("OK: offs=0x%08x i=%d exp=0x%08x act=0x%08x\n",offs,i,wdat32,rdat32);            
        }
            // check pending
        offs = gpio_base_addr + gpio_high_ip;
            //
        DUT_READ32_32 (offs, rdat32);
        if (rdat32 != wdat32) {
            LOGE("ERROR: Mismatch offs=0x%08x i=%d exp=0x%08x act=0x%08x\n",offs,i,wdat32,rdat32);
            errCnt++;
            break;
        }
        else if (verbose) {
            LOGI("OK: offs=0x%08x i=%d exp=0x%08x act=0x%08x\n",offs,i,wdat32,rdat32);            
        }
        
    }
    return errCnt;
}


int cepGpioTest_runTest(int cpuId, int seed, int verbose) {
  int errCnt = 0;
  if (!errCnt) { errCnt += cepGpioTest_runRegTest(cpuId,32, seed, verbose); }
  return errCnt;
}
