//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: MIT
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
#include "cep_aes.h"

//
//
//
static int findATest2Run(int myId, int doneMask, int maxTest) {
  uint64_t rdDat, exp;
  //
  // get the lock
  //
  int lockStat = cep_get_lock(myId, 0, 100);
  if (lockStat == -1) return -1; // fail
  //
  // FInd a test that other cores are not running
  //
  // clear my last test
  //
  cep_raw_write(reg_base_addr + cep_scratch0_reg  + (myId*8), (uint64_t)doneMask << 32);
  //
  for (int i=0;i<maxTest;i++) {
    //
    if (((1<<i) & doneMask) == 0) {
      // check if any one is running it??
      // check the other cores and see if anyone running the same test??
      // pattern = 0x1<xx> xx = 8'dTestId
      exp = (0xBABE0000 + i) | ((uint64_t)doneMask << 32);
      int found = 0;
      for (int c=0;c<4;c++) {
	rdDat = cep_raw_read(reg_base_addr + cep_scratch0_reg  + (c*8));
	if (((exp ^ rdDat) & 0xFFFFFFFF) == 0) {
	  found = 1;
	  break;
	}
      }
      // mark this test to run
      if (found == 0) { // claim it
	cep_raw_write(reg_base_addr + cep_scratch0_reg  + (myId*8), exp);
	cep_release_lock(myId, 0);	
	return i;
      }
    } // if
  } // for
  //
  // release the lock
  //
  cep_release_lock(myId, 0);
  // not found
  return -1;
}

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
  // Playback all the tests on all cores
  //
#include "aes_playback.h"
#include "des3_playback.h"
#include "md5_playback.h"
#include "sha256_playback.h"
#include "gps_playback.h"
#include "iir_playback.h"
#include "fir_playback.h"  
#include "dft_playback.h"
#include "rsa_playback.h"  

  //
  int maxTests = 9;
  uint64_t upper, lower;
  int timeOut = 1000;
  int test2Run;
  //
  int mask = (1 << maxTests)-1;
  int doneMask=0;
  while ((doneMask != mask) && (errCnt == 0)) {
    test2Run = findATest2Run(cpuId,doneMask,maxTests);
    if (test2Run == -1) {
      LOGI("can't find a test to run...TORemain=%d doneMask=0x%04x\n",timeOut--, doneMask);
      pio.RunClk(500);        
      //
      if (timeOut <= 0) {
	LOGE("ERROR: can't find a test to run...TORemain=%d doneMask=0x%04x\n",timeOut--, doneMask);	
	errCnt++;
	break;
      }
    } else {
      timeOut = 100; // reset TO
      LOGI("===> CPU=%d running test %d (doneMask=0x%04x)\n",cpuId,test2Run,doneMask);
      switch (test2Run) {
      case 0:
	upper = aes_adrBase + aes_adrSize;
	lower = aes_adrBase;
	errCnt += cep_playback(aes_playback, upper, lower, aes_cmdCnt4Single, aes_size, verbose);
	break;
      case 1:
	upper = des3_adrBase + des3_adrSize;
	lower = des3_adrBase;
	errCnt += cep_playback(des3_playback, upper, lower, des3_cmdCnt4Single, des3_size, verbose);
	break;
      case 2:
	upper = md5_adrBase + md5_adrSize;
	lower = md5_adrBase;
	errCnt += cep_playback(md5_playback, upper, lower, md5_cmdCnt4Single, md5_size, verbose);
	break;
      case 3:
	upper = sha256_adrBase + sha256_adrSize;
	lower = sha256_adrBase;
	errCnt += cep_playback(sha256_playback, upper, lower, sha256_cmdCnt4Single, sha256_size, verbose);
	break;
      case 4:
	upper = gps_adrBase + gps_adrSize;
	lower = gps_adrBase;
	errCnt += cep_playback(gps_playback, upper, lower, gps_cmdCnt4Single, gps_size, verbose);
	break;
      case 5:
	upper = iir_adrBase + iir_adrSize;
	lower = iir_adrBase;
	errCnt += cep_playback(iir_playback, upper, lower, iir_cmdCnt4Single, iir_size, verbose);
	break;
      case 6:
	upper = fir_adrBase + fir_adrSize;
	lower = fir_adrBase;
	errCnt += cep_playback(fir_playback, upper, lower, fir_cmdCnt4Single, fir_size, verbose);
	break;
      case 7:
	upper = dft_adrBase + dft_adrSize;
	lower = dft_adrBase;
	errCnt += cep_playback(dft_playback, upper, lower, dft_cmdCnt4Single, dft_size, verbose);
	break;
      case 8:
	// IDFT: share with dft
	upper = dft_adrBase + dft_adrSize + dft_adrSize;
	lower = dft_adrBase + dft_adrSize;
	errCnt += cep_playback(dft_playback, upper, lower, dft_cmdCnt4Single, dft_size, verbose);
	break;
#if 0
      case 9:
	upper = rsa_adrBase + rsa_adrSize;
	lower = rsa_adrBase;
	errCnt += cep_playback(rsa_playback, upper, lower, rsa_cmdCnt4Single, rsa_size, verbose);
	break;
#endif
      }	
      doneMask |= 1 << test2Run;
      if (errCnt) {
	break;
      }
    } // else
  }
  //
  // clear my lock if any
  //
  (void)findATest2Run(cpuId,doneMask,maxTests); 
  //
  pio.RunClk(500);  
  //
  // ======================================
  // Exit
  // ======================================
cleanup:
  if (errCnt != 0) {
    LOGI("======== TEST FAIL test2Run=%d ========== %x\n",test2Run,errCnt);
  } else {
    LOGI("======== TEST PASS ========== \n");    
  }
  //  shIpc *ptr = GlobalShMemory.getIpcPtr(offset);
  ptr->SetError(errCnt);
  ptr->SetThreadDone();
  pthread_exit(NULL);
  return ((void *)NULL);
}

