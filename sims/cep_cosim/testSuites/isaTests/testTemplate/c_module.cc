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

void *c_module(void *arg) {

  //--------------------------------------------------------------------------------------
  // Test Set up
  //--------------------------------------------------------------------------------------
  pthread_parm_t *tParm = (pthread_parm_t *)arg;
  int errCnt = 0;
  int slotId = tParm->slotId;
  int cpuId = tParm->cpuId;
  int verbose = tParm->verbose;
  Int32U seed = tParm->seed;    // Used as a timeout parameter
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
  // For the ISA tests, some tests only run on Core 0, with the other cores sitting
  // in an infinite loop.  Under these conditions, we must not only disable the
  // "stuck checker" for cores 1 - 4, we need to "assume" success
#ifdef SINGLE_CORE_ONLY
  if (cpuId != 0) {
    DUT_WRITE_DVT(DVTF_SINGLE_CORE_ONLY, DVTF_SINGLE_CORE_ONLY, 1);
  } // if (cpuId != 0)
#endif 

  // Wait until the program is loaded
  errCnt += is_program_loaded(seed);

  // A timeout has occured, terminate the thread
  if (errCnt) goto cleanup;

  // Release the tile reset
  release_tile_reset(cpuId);

  // Check the status of the bare metal program
  errCnt += check_PassFail_status(cpuId, seed);

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
  ptr->SetError(errCnt);
  ptr->SetThreadDone();
  pthread_exit(NULL);
  return ((void *)NULL);
}
  //--------------------------------------------------------------------------------------
