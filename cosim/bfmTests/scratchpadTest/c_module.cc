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
#include "cepDdr3MemTest.h"

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
  // no need!!!
  int calibDone = calibrate_ddr3(50);

  int full = 0;
  int adrWidth = 16 - 2; // 65k (16 bits) for all 4 cores => each one will test 1/4 of that
  u_int32_t mem_base = scratchpad_base_addr + ((cpuId&0x3) << adrWidth);  

#if 0
  int dataWidth = 0;

  for (int i=0;i<4;i++) {
    switch ((i + cpuId) & 0x3) {
    case 0 : dataWidth = 8; break;
    case 1 : dataWidth = 16; break;
    case 2 : dataWidth = 32; break;
    case 3 : dataWidth = 64; break;
   }
    if (!errCnt) { errCnt = cepMemTest_runTest(cpuId, mem_base, adrWidth, dataWidth ,seed + ((cpuId+i)*0x10), verbose, full); }
  }
#endif
  errCnt = cepDdr3MemTest_runTest(cpuId, mem_base, adrWidth, seed, full, verbose);


  //
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

