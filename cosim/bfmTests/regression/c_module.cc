//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
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

#include "cep_apis.h"
#include "simdiag_global.h"
#include "CEP.h"
#include "cepregression.h"

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
  pio.RunClk(1000);
  //
  int mask = 0; 
  switch (cpuId) {
  case 0:
    mask = ((1 << CEP_VERSION_REG_K) |
	    (1 << AES_BASE_K) |
	    (1 << MD5_BASE_K) |
	    (1 << SHA256_BASE_K));
    break;
  case 1:
    mask = ((1 << DES3_BASE_K) |
            (1 << DFT_BASE_K ) |
            (1 << IDFT_BASE_K));
    break;
  case 2:
    mask = (1 << RSA_ADDR_BASE_K);
    break;
    
  case 3:
    mask = ((1 << FIR_BASE_K) |
	    (1 << IIR_BASE_K) |
	    (1 << GPS_BASE_K));
    break;
  }
  errCnt += run_ceptest(mask);
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

