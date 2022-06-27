//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      c_dispatch.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//--------------------------------------------------------------------------------------
#include <unistd.h>
#include "v2c_cmds.h"
#include "access.h"
#include "c_dispatch.h"
#include "c_module.h"
#include "cep_adrMap.h"
#include "cep_apis.h"
#include "portable_io.h"
#include "simPio.h"

int main(int argc, char *argv[])
{

  unsigned long seed;
  sscanf(argv[1], "0x%x", &seed);  
  printf("Seed = 0x%x\n", seed);
  
  int errCnt                = 0;
  int verbose               = 0x1f;
  int activeSlot            = 0;          // only 1 board 
  int maxHost               = MAX_CORES;  // number of cores/threads

  // Some ISA Tests only run on a single core with the other cores sitting in an infinite loop.
  // Behavior is unpredicable when only "some" cores are released from reset / threads spawned.
  // Thus, all cores will be released from reset, but the mask parameter will be used to
  // determined if a core's Pass/Fail status should be considered (in c_module.cc)
  #ifdef SINGLE_CORE_ONLY
    long unsigned int mask  = 0xF;	// Core 0 only 	
  #else
    long unsigned int mask  = 0xF;	// All cores
  #endif

  int done                = 0;
  shPthread               thr;

  // Set the active mask for all threads  
  thr.SetActiveMask(mask);
  
  // Threads are spawned for ALL cores, regardless of if they are active for this test
  for (int i = 0; i < maxHost; i++) {
    thr.ForkAThread(activeSlot, i, verbose, MAX_TIMEOUT, c_module);
  }
  
  // spawn system thread
  thr.AddSysThread(SYSTEM_SLOT_ID, SYSTEM_CPU_ID);


  // Enable waveform capture, unless disabled
  int cycle2start   = 0;
  int cycle2capture = -1; // til end
  int wave_enable   = 1;
  #ifndef NOWAVE
    dump_wave(cycle2start, cycle2capture, wave_enable);
  #endif

  //--------------------------------------------------------------------------------------
  // Load the bare executable into scratchpad memory (from the system thread)
  //--------------------------------------------------------------------------------------
  int verify        = 0;
  int fileOffset    = 0;
  int maxByteCnt    = cep_max_program_size;
  errCnt += loadMemory(RISCV_WRAPPER, fileOffset, maxByteCnt);
  
  if (errCnt) goto cleanup;
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // Have the system thea wait until all threads are complete
  //--------------------------------------------------------------------------------------
  while (!done) {

    // Check to see if all the threads are done
    done = thr.AllThreadDone();

  }
  //--------------------------------------------------------------------------------------



  //--------------------------------------------------------------------------------------
  // End of test checking and thread termination
  //--------------------------------------------------------------------------------------
  cleanup: errCnt += thr.GetErrorCount();
  if (errCnt != 0) {
    LOGE("======== TEST FAIL ========== %x\n",errCnt);
  } else {
    LOGI("%s ======== TEST PASS ========== \n",__FUNCTION__);
  }

  thr.Shutdown();

  return(errCnt);
  //--------------------------------------------------------------------------------------
}
