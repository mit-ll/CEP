#define MAX_TIMEOUT 2000
//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
#include <unistd.h>
#include "v2c_cmds.h"
#include "access.h"
#include "c_dispatch.h"
#include "c_module.h"
#include "cep_apis.h"
#include "cep_adrMap.h"
#include "simPio.h"
/*
 * main 
 */
int main(int argc, char *argv[])
{

  /* ===================================== */
  /*   SETUP SECTION FOR SIMULATION        */
  /* ===================================== */
  unsigned long seed;
  sscanf(argv[1],"0x%x",&seed);  
  printf("Seed = 0x%x\n",seed);
  int errCnt = 0;
  int verbose = 0x1f;

  /* ===================================== */
  /*   spawn all the paralle threads       */
  /* ===================================== */
  int activeSlot=0; // only 1 board 
  //
  // ============================  
  // fork all the tests here
  // ============================  
  //
  shPthread thr;
  //
  // max number of cores not include the system thread
  //
  int maxHost = MAX_CORES; // number of cores/threads
  //
  // each bit is to turn on the given core (bit0 = core0, bit1=core1, etc..)
  //
  long unsigned int mask = 0xf;
#ifdef   SINGLE_CORE_ONLY
  mask  = SINGLE_CORE_ONLY;
#endif
  //
  // Set the active CPU mask before spawn the threads...
  //
  thr.SetActiveMask(mask);
  //
  // c_module is the threead to run
  //
  for (int i=0;i<maxHost;i++) {
    if ((long unsigned int)(1 << i) & mask) {
      thr.ForkAThread(activeSlot,i,verbose, MAX_TIMEOUT, c_module);
    }
  }  
  //
  // lastly: Added system thread always
  //
  thr.AddSysThread(SYSTEM_SLOT_ID,SYSTEM_CPU_ID);
      // turn on remote bitbang
  //
#ifdef SINGLE_THREAD_ONLY
  DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, mask);
  DUT_WRITE_DVT(DVTF_FORCE_SINGLE_THREAD, DVTF_FORCE_SINGLE_THREAD, 1);
#endif

  //
  // SOme test just go straigth to there if not <fail>
#ifdef PASS_WRITETOHOST
  DUT_WRITE_DVT(DVTF_PASS_WRITETOHOST, DVTF_PASS_WRITETOHOST, 1);
#endif

  
  // ============================
  // Turn on the wave here
  // ============================  
  //
  int cycle2start=0;
  int cycle2capture=-1; // til end
  int wave_enable=1;
  #ifndef NOWAVE
  dump_wave(cycle2start, cycle2capture, wave_enable);
  #endif
  //
  // Enable main memory logging
  //
  //DUT_WRITE_DVT(DVTF_ENABLE_MAIN_MEM_LOGGING, DVTF_ENABLE_MAIN_MEM_LOGGING, 1);
  //
  //
  #if 1
      //
      // ============================
      // Setup and Fork OpenOcd here as another thread...
      // ============================
      //
  int socketPortId = 0;
  int timeOut = 1000;
  LOGI("SocketPortId = 0x%08x to=%d\n",socketPortId,timeOut);        
  while (timeOut > 0) {
      DUT_WRITE_DVT(DVTF_GET_SOCKET_ID_BIT,DVTF_GET_SOCKET_ID_BIT,1);
      socketPortId = DUT_READ_DVT(DVTF_PAT_HI, DVTF_PAT_LO);
      LOGI("SocketPortId = 0x%08x to=%d\n",socketPortId,timeOut);      
      if (socketPortId >> 31)  {
          break; 
      }
      DUT_WRITE_DVT(DVTF_GET_SOCKET_ID_BIT,DVTF_GET_SOCKET_ID_BIT,0);
      DUT_RUNCLK(100);
      timeOut--;
  }
  
  char cmd[128];
  sprintf(cmd,"openocd -c \"set _JTAG_SOCKET_PORT %d\" -f dtm.cfg -d2 -l openOcd.log", socketPortId & 0xffff);
  DUT_WRITE_DVT(DVTF_ENABLE_REMOTE_BITBANG_BIT, DVTF_ENABLE_REMOTE_BITBANG_BIT, 1);
      // child
  if (fork() == 0) {
      printf("Executing system command %s\n",cmd);  
      system(cmd);
      return 0;
  }
#endif
  //
  // wait for calibration??
  //
  #if 1
  //int calibDone = calibrate_ddr3(50);
  int backdoor_on = 1;
  int verify = 0;
  int srcOffset = 0x1000;
  int destOffset = 0;
  errCnt += loadMemory((char *)"./riscv_wrapper.elf", ddr3_base_adr,srcOffset, destOffset, backdoor_on, verify);
  //errCnt += loadMemory(RISCV_WRAPPER, ddr3_base_adr,srcOffset, destOffset, backdoor_on, verify);
  #endif
  DUT_WRITE_DVT(DVTF_DISABLE_MAIN_MEM_LOGGING, DVTF_DISABLE_MAIN_MEM_LOGGING, 1);
  //
  // log the write only
  //DUT_WRITE_DVT(DVTF_ENABLE_MAIN_MEMWR_LOGGING, DVTF_ENABLE_MAIN_MEMWR_LOGGING, 1);
  //
  // ============================  
  // wait until all the threads are done
  // ============================  
  //
  int Done = 0;
  while (!Done) {
    Done = thr.AllThreadDone();
    sleep(2);
  }
  //
  toggleDmiReset();
      // just in case
  DUT_WRITE_DVT(DVTF_ENABLE_REMOTE_BITBANG_BIT, DVTF_ENABLE_REMOTE_BITBANG_BIT, 0);
      //
  //
  /* ===================================== */
  /*   END-OF-TEST CHECKING                */
  /* ===================================== */
  errCnt += thr.GetErrorCount();
  if (errCnt != 0) {
    LOGE("%s ======== TEST FAIL ========== %x\n",__FUNCTION__,errCnt);
  } else {
    LOGI("%s ======== TEST PASS ========== \n",__FUNCTION__);
  }
  //
  // shutdown HW side
  // dont call this DUT_SetInActiveStatus is used
  thr.Shutdown();
  return(errCnt);
}
