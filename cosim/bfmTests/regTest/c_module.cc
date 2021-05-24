//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
#include "v2c_cmds.h"
#include "simPio.h"
//
#include "shMem.h"
#include "c_module.h"
#include <unistd.h>
#include "random48.h"

#include "cep_adrMap.h"
#include "cep_apis.h"
#include "simdiag_global.h"
#include "cepRegTest.h"

//
void *c_module(void *arg) {


  // ======================================
  // Set up
  // ======================================
  pthread_parm_t *tParm = (pthread_parm_t *)arg;
  int errCnt = 0;
  int slotId = tParm->slotId;
  int cpuId = tParm->cpuId;
  int verbose = tParm->verbose;
  Int32U seed = tParm->seed;
  int restart = tParm->restart;
  int offset = GET_OFFSET(slotId,cpuId);
  GlobalShMemory.getSlotCpuId(offset,&slotId,&cpuId);
  //printf("offset=%x seed=%x verbose=%x GlobalShMemory=%x\n",offset,seed, verbose,(unsigned long) &GlobalShMemory);
  // notify I am Alive!!!
  shIpc *ptr = GlobalShMemory.getIpcPtr(offset);
  ptr->SetAliveStatus();
  sleep(1);

  // ======================================
  // Test is Here
  // ======================================
  simPio pio;
  pio.MaybeAThread(); // chec
  pio.EnableShIpc(1);
  pio.SetVerbose(verbose);

  //
  // ======================================
  // Test starts here
  // ======================================
  // MUST
  // wait until Calibration is done..
  //int calibDone = calibrate_ddr3(50);
  pio.RunClk(500);

#if 0
  int i;
  uint64_t buf[8] = {0x8888888888888888,
		     0x1111111111111111,
		     0x2222222222222222,
		     0x3333333333333333,
		     0x4444444444444444,
		     0x5555555555555555,
		     0x6666666666666666,
		     0x7777777777777777};
  uint64_t rdBuf[8];
  // DUT_WRITE32_64(reg_base_addr + cep_scratch0_reg, buf[0]);
  DUT_WRITE32_BURST(reg_base_addr + cep_scratch0_reg, 8, buf);
  DUT_READ32_BURST(reg_base_addr + cep_scratch0_reg, 8, rdBuf);
  for (int i=0;i<8;i++) {
    if (buf[i] != rdBuf[i]) {
      LOGE("ERROR: i=%d exp=0x%016lx act=0x%016lx\n",i,buf[i],rdBuf[i]);
      errCnt++;
    } else {
      LOGI("OK: i=%d exp=0x%016lx act=0x%016lx\n",i,buf[i],rdBuf[i]);
    }
  }
#endif
  
  //
#if 1
  int revCheck = 1;
  if (!errCnt) { errCnt = cepRegTest_runTest(cpuId,64, revCheck,seed, verbose); }
#endif
  //
  //
  pio.RunClk(100);
  
  //
  // ======================================
  // Exit
  // ======================================
cleanup:
  if (errCnt != 0) {
    LOGI("======== TEST FAIL ========== %x\n",errCnt);
  } else {
    LOGI("======== TEST PASS ========== \n");    
  }
  //  shIpc *ptr = GlobalShMemory.getIpcPtr(offset);
  ptr->SetError(errCnt);
  ptr->SetThreadDone();
  pthread_exit(NULL);
  return ((void *)NULL);
}

