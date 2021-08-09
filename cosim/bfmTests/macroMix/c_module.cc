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

#include "cep_apis.h"
#include "simdiag_global.h"
#include "cepMacroMix.h"

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

  // Select which tests to run.  Core indicies are defined in CEP.h
  int mask = seed; // seed is used as cpuActiveMask from c_displatch
  int cryptoMask = 0xFFFFFFFF; // all cores
// int cryptoMask = 0x00000001; // AES only
//  int cryptoMask = 0x00000002; // MD5 only
//  int cryptoMask = 0x00000004; // SHA256 only
//  int cryptoMask = 0x00000008; // RSA only
//  int cryptoMask = 0x00000010; // DES3 only
//  int cryptoMask = 0x00000020; // DFT only
//  int cryptoMask = 0x00000040; // IDFT only
//  int cryptoMask = 0x00000080; // FIR only
//  int cryptoMask = 0x00000100; // IIR only
//  int cryptoMask = 0x00000200; // GPS only

  if (!errCnt) { errCnt = cepMacroMix_runTest(cpuId, mask, cryptoMask, seed, verbose); }
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

