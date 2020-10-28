//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
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
#include "cepregression.h"
#include "simPio.h"

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
  int calibDone = is_program_loaded(50);

  //
  errCnt += check_bare_status(cpuId,1000);
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

