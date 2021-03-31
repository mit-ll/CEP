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
#include "cepMemTest.h"
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
  int calibDone = calibrate_ddr3(50);

  int backdoor_on = 1;
  int verify = 1;
  int srcOffset = 0x1000;
  int destOffset = 0;
  errCnt += load_mainMemory((char *)"./image.elf", ddr3_base_adr,srcOffset, destOffset, backdoor_on, verify); 

#if 0  
  u_int32_t adr1 = 0x80000000;
  u_int32_t adr2 = 0x8F000000;  
  u_int64_t wrDat = 0x0102030405060708LL, rdDat;
  
  DUT_WRITE32_64(adr1,wrDat);
  DUT_WRITE32_64(adr2,~wrDat);  
  DUT_READ32_64(adr1,rdDat);
  LOGI("rdDat=0x%06llx\n",rdDat);  
  DUT_READ32_64(adr2,rdDat);  
  LOGI("rdDat=0x%06llx\n",rdDat);
#endif
  
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

