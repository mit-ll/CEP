//--------------------------------------------------------------------------------------
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      c_module.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//--------------------------------------------------------------------------------------
#include "v2c_cmds.h"
#include "simPio.h"
#include "shMem.h"
#include "c_module.h"
#include <unistd.h>
#include "random48.h"
#include "CEP.h" 
#include "cep_apis.h"
#include "simdiag_global.h"
#include "cepMultiThread.h"


void *c_module(void *arg) {

  //--------------------------------------------------------------------------------------
  // Test Set up
  //--------------------------------------------------------------------------------------
  pthread_parm_t *tParm = (pthread_parm_t *)arg;
  int errCnt = 0;
  int slotId = tParm->slotId;
  int cpuId = tParm->cpuId;
  int verbose = tParm->verbose;
  Int32U seed = tParm->seed;
  int restart = tParm->restart;
  int offset = GET_OFFSET(slotId, cpuId);
  GlobalShMemory.getSlotCpuId(offset, &slotId, &cpuId);

  shIpc *ptr = GlobalShMemory.getIpcPtr(offset);
  ptr->SetAliveStatus();
  sleep(1);

  simPio pio;
  pio.MaybeAThread(); 
  pio.EnableShIpc(1);
  pio.SetVerbose(verbose);

  //--------------------------------------------------------------------------------------
  // Test starts here
  //--------------------------------------------------------------------------------------
 
  // Wait until Calibration is done...
  pio.RunClk(500);

  // Indicate which cores to enable 
  // (all cores minus GPS because not currently working in MultiThread)
  //int coreMask = 0xFFFFCFFB; 
  int coreMask = 0xFFFF0FFF;

  //int maxTest = CEP_LLKI_CORES;
  int maxTest = 16;
  int maxLoop = 2;
 
  uint64_t testLockBuf = CEPREGS_BASE_ADDR + cep_scratch0_reg;
  
  // Setup SRoT and initialize scratchpad space for test status
  errCnt += cepMultiThread_setup(cpuId, testLockBuf, coreMask, maxTest, GlobalShMemory.getActiveMask(), verbose);
  
  // Run cepMultiThread 
  errCnt += cepMultiThread_runThr(cpuId, testLockBuf, coreMask, maxTest, maxLoop, seed, verbose);
      
  pio.RunClk(100);  

  //--------------------------------------------------------------------------------------
  // Exit
  //--------------------------------------------------------------------------------------
 cleanup:
  if (errCnt != 0) {
    LOGI("======== TEST FAIL ========== %x\n",errCnt);
  } else {
    LOGI("======== TEST PASS ========== \n");    
  }
  ptr->SetError(errCnt);
  ptr->SetThreadDone();
  pthread_exit(NULL);
  return ((void *)NULL);
}

