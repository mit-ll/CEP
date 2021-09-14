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
#include "cepPlicTest.h"
#include "cepUartTest.h"
#include "cepSpiTest.h"
//
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
  // pick one to run
  if (cpuId == 0) {
      int maxReg = 10; // uart,spi,gpio[7:0], no 0
      for (int i=1;i<=maxReg;i++) {
          (*regp->AddAReg_p)(regp, plic_base_addr + (plic_source0_prio_offset) + (i*4), 0x7); // only 3 bits
      }
  }
  // each hart has 2 context
  for (int c=0;c<2;c++) {
      int ctx = (cpuId*2) + c;
      (*regp->AddAReg_p)(regp, plic_base_addr + plic_hart0_m_mode_ien_start  + (plic_mode_ien_size_offset*ctx), 0x7FE); // only 10 sources, no 0
      (*regp->AddAReg_p)(regp, plic_base_addr + plic_hart0_m_mode_prio_thresh  + (plic_prio_thresh_size_offset*ctx), 0x7); // only 3 bits
  }
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


//
// PLIC priority interrupt test
//
int cepPlicTest_prioIntrTest(int coreId, int verbose)
{
    int errCnt = 0;
    uint32_t offS, dat32;
    int startSrc = 1; // uart(1),spi(2),GPIO (3-10)
    int endSrc   = 10;
    char txStr[]  = "MITLL";
    //
    // setup
    //
    uint32_t enableMask = 0;
    for (int i=startSrc;i<= endSrc;i++) {
        enableMask |= (1 << i);
    }
    //
    // set the threshold to 0,2,4,6
    // each Hart will have 2 context? M-mode and S-mode
    //
    for (int ctx=0;ctx<2;ctx++) {
        offS = plic_base_addr + plic_hart0_m_mode_prio_thresh + (plic_prio_thresh_size_offset*((coreId*2)+ctx));
        dat32 = 0;
        DUT_WRITE32_32(offS, dat32);
    }
    // prio from 0 to 7
    // NOTE: if prio=0, there will be no interrupt but the internal pending bit is still set.. CHECKME!!!!
    int prio = 7;
    for (int i=startSrc;i<= endSrc;i++) {
        DUT_WRITE32_32(plic_base_addr + plic_source0_prio_offset +  i*4, prio); // start with 1 for core0
        prio--;
        if (prio < 1) prio = 1; // min
    }
    //
    // get the lock
    //
    int actSrc = 0;
    if (cep_get_lock(coreId, 1, 5000)) { // use lock 1
        for (int c=0;c<2;c++) {
            int ctx = (coreId*2) + c;
            //
            // set enable
            //
            offS = plic_base_addr + plic_hart0_m_mode_ien_start + (plic_mode_ien_size_offset*ctx);        
            DUT_WRITE32_32(offS, enableMask);
            //
            // pulse the interrupt
            //
            errCnt += cepUartTest_runTxRxTest(coreId,32-1, &txStr[coreId],  1, 0, verbose);
            errCnt += cepSpiRdWrTest(coreId,1, 0, verbose);
            //
            DUT_WRITE32_32(gpio_base_addr + gpio_low_ie, 0xff); // gpio[7:0]
            DUT_WRITE32_32(gpio_base_addr + gpio_low_ie, 0x0);        
            //
            // check claim
            //
            for (int expSrc=startSrc; expSrc <= endSrc;expSrc++) {
                // read my claim
                DUT_READ32_32(plic_base_addr + (plic_prio_thresh_size_offset*ctx) + plic_hart0_m_mode_claim_done,actSrc);
                if (actSrc != expSrc) {
                    LOGE("ERROR: ctx=%d mismatch expSrc=%d actSrc=%d\n",ctx,expSrc, actSrc);
                    errCnt++;
                    break;
                }
                else if (verbose) {
                    LOGI("OK: ctx=%d expSrc=%d actSrc=%d\n",ctx,expSrc, actSrc);                
                }
                // I am done with my claim
                DUT_WRITE32_32(plic_base_addr + (plic_prio_thresh_size_offset*ctx) + plic_hart0_m_mode_claim_done,actSrc);
            }
            // clear enable
            offS = plic_base_addr + plic_hart0_m_mode_ien_start + (plic_mode_ien_size_offset*ctx);
            DUT_WRITE32_32(offS, 0);
            if (errCnt) break;
        }
        cep_release_lock(coreId, 1);
    } else {
        errCnt++;
    }
    //
    return errCnt;
}

