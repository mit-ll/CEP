//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
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
#include "cep_adrMap.h"
#include "cep_apis.h"
#include "simdiag_global.h"

#include "cepSrotMemTest.h"

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
  int offset = GET_OFFSET(slotId,cpuId);
  GlobalShMemory.getSlotCpuId(offset,&slotId,&cpuId);

  shIpc *ptr = GlobalShMemory.getIpcPtr(offset);
  ptr->SetAliveStatus();
  sleep(1);

  simPio pio;
  pio.MaybeAThread();
  pio.EnableShIpc(1);
  pio.SetVerbose(verbose);
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // Test starts here
  //--------------------------------------------------------------------------------------
  pio.RunClk(500);

  errCnt += cepSrotMemTest_runTest(cpuId, seed, verbose);

  pio.RunClk(100);  
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // Exit
  //--------------------------------------------------------------------------------------
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
  //--------------------------------------------------------------------------------------
