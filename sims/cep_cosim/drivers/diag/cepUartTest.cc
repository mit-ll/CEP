//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
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
#include "cepUartTest.h"

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
cepUartTest_WriteEntry(regBaseTest_t *me, uint32_t adr, uint64_t dat)
{
  DUT_WRITE32_32(adr,dat);
  return 0;
}

uint64_t  cepUartTest_ReadEntry(regBaseTest_t *me, uint32_t adr) {
  uint32_t d32;
  DUT_READ32_32(adr,d32);
  return (uint64_t)d32;
}

//
// =============================
// The test itself
// =============================
//
int cepUartTest_runTxRxTest(int cpuId, int divisor, char *txStr, int txLen, int stopBits, int verbose) {
  int errCnt = 0;
  int ti=0, ri=0, tmp;
  char rxDat;

  // write divisor register
  if (verbose) {
    LOGI("%s: div=%d str=%s len=%d stop=%d\n",__FUNCTION__,divisor,txStr,txLen,stopBits);
  }

  DUT_WRITE32_32(uart_base_addr + uart_div, divisor);
  int act;
  DUT_READ32_32(uart_base_addr + uart_div, act);
  if (act != divisor) { 
    LOGE("%s: mismatch divisor act=0x%x exp=0x%x\n",__FUNCTION__,act,divisor);
    errCnt++;
    return errCnt;
  } else if (verbose) {
    LOGI("%s: OK divisor act=0x%x exp=0x%x\n",__FUNCTION__,act,divisor);
  }
  
  // enable tx/rx
  int dat = ((1 << 16) |  // watermark
            ( 1 <<  0) |  // txen
            ((stopBits & 0x1) << 1) ); // 0=1 stop bit, 1=2 stop bits
  //
  DUT_WRITE32_32(uart_base_addr + uart_txctrl, dat);
  DUT_WRITE32_32(uart_base_addr + uart_rxctrl, dat);
  
  // Enable interrupt pending
  dat = ((1 << 0) | // TX interrupt Enable
         (1 << 1)); // RX interrupt Enable
  DUT_WRITE32_32(uart_base_addr + uart_ie,dat);
  
  // verify the TX interrupt pending is there
  DUT_READ32_32(uart_base_addr + uart_ip, tmp);
  
  if (tmp != 1) { // TX pending and no RX pending
    LOGE("%s: No TX pending when fifo is empty (0x%x)??\n",__FUNCTION__,tmp);
    errCnt++;
  } else if (verbose) {
    LOGI("%s: OK TX pending is set when fifo is empty (0x%x)\n",__FUNCTION__,tmp);
  }
  
  // Tx / Rx loop
  while ((ri < txLen) && (!errCnt)) {
    
    // TX?
    if (ti < txLen) { // still more to TX?? (include NULL)
      DUT_READ32_32(uart_base_addr + uart_txfifo, tmp);

      if (((tmp >> 31) & 0x1) == 0) { // not full
        DUT_WRITE32_32(uart_base_addr + uart_txfifo, (uint32_t)txStr[ti]);
        ti++;
      }
    }
    
    // any RX?
    if (ri < txLen) {
      DUT_READ32_32(uart_base_addr + uart_rxfifo, tmp);

      if (((tmp >> 31) & 0x1) == 0) { // something in there

        rxDat = (char)(tmp & 0xff);
    
        // check
        if (rxDat != txStr[ri]) {
          LOGE("%s: mismatch char ri=%d act=0x%x exp=0x%x\n",__FUNCTION__,ri,rxDat,txStr[ri]);
          errCnt++;
          ri++;
          break;
        } else {
          if (verbose) {
            LOGI("%s: OK char ri=%d act=0x%x exp=0x%x\n",__FUNCTION__,ri,rxDat,txStr[ri]);
          }
          ri++;
        } // if (rxDat != txStr[ri])
      } // if (((tmp >> 31) & 0x1) == 0)
    } else {
      break;
    } // if (ri < txLen)
  } // while ((ri < txLen) && (!errCnt))

  // disable
  DUT_WRITE32_32(uart_base_addr + uart_txctrl,0);
  DUT_WRITE32_32(uart_base_addr + uart_rxctrl,0);
  DUT_WRITE32_32(uart_base_addr + uart_ie,0);
  return errCnt;
}

int cepUartTest_runRegTest(int cpuId, int accessSize,int seed, int verbose) {

  int errCnt = 0;
  //
  regBaseTest_t *regp; //
  //
  cepUartTest_CREATE(regp,cpuId, accessSize, seed + (cpuId*100),verbose);
    
  //
  // start adding register to test
  //
  (*regp->AddAReg_p)(regp, uart_base_addr + uart_txctrl, (0x7 << 16) | 0x3);
  (*regp->AddAReg_p)(regp, uart_base_addr + uart_rxctrl, (0x7 << 16) | 0x1);
  (*regp->AddAReg_p)(regp, uart_base_addr + uart_ie,     0x3);
  //  (*regp->AddAReg_p)(regp, uart_base_addr + uart_ie,     0xFFFFFFFF);
  (*regp->AddAReg_p)(regp, uart_base_addr + uart_div,     0xFFFF);

  // add a hole in 4K
  (*regp->AddAHole_p)(regp, uart_base_addr + 0xFFC,0xFFFFFFFF);

  // now do it
  errCnt = (*regp->doRegTest_p)(regp);
  //
  // Destructors
  //
  cepUartTest_DELETE(regp);
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

int cepUartTest_runTest(int cpuId, int seed, int verbose) {
  int errCnt = 0;
  char txStr[]  = "MIT";
  char txStr2[] = "LL";
  
  // divisor 16 is smallest!!!, try 15 .loop forever..
  if (!errCnt) { errCnt += cepUartTest_runTxRxTest(cpuId, 32, txStr,  strlen(txStr), 0, 1); }

  return errCnt;

  if (!errCnt) { errCnt += cepUartTest_runTxRxTest(cpuId, 63, txStr2, strlen(txStr2), 1, 1); }
  
  // this is just for coverage
  DUT_WRITE32_32(uart_base_addr + uart_div, 1 << 15);
  for (int i=0;i<8;i++) {
    DUT_WRITE32_32(uart_base_addr + uart_txfifo, 0);
  }
  int tmp;
  DUT_READ32_32(uart_base_addr + uart_rxfifo, tmp);
  //
  if (!errCnt) { errCnt += cepUartTest_runRegTest(cpuId, 32, seed, verbose); }
  //
  return errCnt;
}
