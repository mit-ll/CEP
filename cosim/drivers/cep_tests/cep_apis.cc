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
#include "portable_io.h"

#ifdef SIM_ENV_ONLY
#include "simPio.h"
#else
// SW or BARE in sim
#include <stdint.h>
#endif
//
#include "cep_adrMap.h"
#include "cep_apis.h"

#ifdef BARE_MODE
#include "encoding.h"
#endif



//
//
//
int calibrate_ddr3(int maxTimeOut) {
  //
  int calibDone = 0;
#ifdef SIM_ENV_ONLY
  int to=0;  
  while (!calibDone && (to < maxTimeOut)) {
    calibDone = DUT_READ_DVT(DVTF_READ_CALIBRATION_DONE, DVTF_READ_CALIBRATION_DONE);
    DUT_RUNCLK(1000);
    to++;
  }
  if (!calibDone) {
    LOGI("ERROR: Calibration is not done\n");
  } else {
    LOGI("OK: DDR3 calibration is completed\n");    
  }
#endif
  return calibDone;
}

int is_program_loaded(int maxTimeOut) {
  //
  int loaded = 0;
#ifdef SIM_ENV_ONLY
  int to=0;  
  while (!loaded && (to < maxTimeOut)) {
    loaded = DUT_READ_DVT(DVTF_PROGRAM_LOADED, DVTF_PROGRAM_LOADED);
    DUT_RUNCLK(1000);
    to++;
  }
  if (!loaded) {
    LOGI("ERROR: Program_loading is not done\n");
  } else {
    LOGI("OK: Program_loading is completed\n");    
  }
#endif
  return loaded;
}

void dump_wave(int cycle2start, int cycle2capture, int enable)
{
#ifdef SIM_ENV_ONLY
  //
  // When to start
  //
  DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, cycle2start); // at cycle 10
  DUT_WRITE_DVT(DVTF_WAVE_START_CYCLE, DVTF_WAVE_START_CYCLE, 1);
  
  // how many cycles: comment out if until end
  if (cycle2capture != -1) {
    DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, cycle2capture); // 
    DUT_WRITE_DVT(DVTF_WAVE_CYCLE2DUMP, DVTF_WAVE_CYCLE2DUMP, 1);
  }
  //
  // un-comment to turn on the wave
  if (enable) {
    DUT_WRITE_DVT(DVTF_WAVE_ON , DVTF_WAVE_ON, enable);
  }
#endif
}

int clear_printf_mem(int coreId) {
  int errCnt = 0;
#ifdef SIM_ENV_ONLY
  LOGI("%s: coreid=%d\n",__FUNCTION__,coreId);
  for (int i=0;i<cep_printf_max_lines;i++) {
    for (int j=0;j<16;j++) {
      DUT_WRITE32_64(cep_printf_mem + (coreId*cep_printf_core_size) + (i*cep_printf_str_max) + (j*8), 0);
    }
  }
#endif
  return errCnt;
}

//
// Load the image into main memory
//
int load_mainMemory(char *imageF, uint32_t ddr3_base, int srcOffset, int destOffset, int backdoor_on, int verify) {
  int errCnt = 0;
#ifdef SIM_ENV_ONLY  
  FILE *fd=NULL;
  uint64_t d64,rd64;
  int s,d,i=0, bCnt=0;  
  fd=fopen(imageF,"rb");
  //
  LOGI("==== %s: Loading file %s to main memory base=0x%08x ====\n",__FUNCTION__,imageF,ddr3_base);
  //
  if (fd == NULL) {
    printf("Can't open file %s\n",imageF);
    return 1;
  }
  else {
    DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, 0);
    DUT_WRITE_DVT(DVTF_SET_IPC_DELAY, DVTF_SET_IPC_DELAY, 1);
      
    //
    // fist 16bit = upstream, second 16 bits = downstream, etc..
    // And this test is for downstream so use only every second 16bits
    //
    // Enable
    if (backdoor_on) {
      DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, 1);
      DUT_WRITE_DVT(DVTF_ENABLE_MEM_BACKDOOR, DVTF_ENABLE_MEM_BACKDOOR, 1);
    }
    s=0;
    d=0;
    while (!feof(fd)) {
      fread(&d64,sizeof(uint64_t),1,fd);
      if ((s*8) >= srcOffset) {
	DUT_WRITE32_64(ddr3_base + destOffset + d*8,d64);
	d++;
      }
      s++;
    }
    bCnt=(i+1)*8;
    //
    // if not end at the cache line (64bytes), continue end so if in backdoor mode
    //
    if (backdoor_on && ((s & 0x7) != 0)) {
      d64 = 0xDEADDEADDEADDEADLL;      
      LOGI("==== %s: flusing to cache line s=%d ====\n",__FUNCTION__,s);
      while ((s & 0x7) != 0) {
	DUT_WRITE32_64(ddr3_base + d*8,d64);
	d++;
	s++;
      }
    }
    LOGI("==== %s: DONE backdoor loading file %s to main memory size=%d bytes ====\n",__FUNCTION__,imageF,bCnt);
    // Disable
    if (backdoor_on) {    
      DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, 0);
      DUT_WRITE_DVT(DVTF_ENABLE_MEM_BACKDOOR, DVTF_ENABLE_MEM_BACKDOOR, 1);
    }
    //
    if (verify) {
      rewind(fd);
      s = 0;
      d = 0;
      while (!feof(fd)) {
	fread(&d64,sizeof(uint64_t),1,fd);
	//
	// use front door to verify??
	//
	if ((s*8) >= srcOffset) {
	  DUT_READ32_64(ddr3_base + d*8,rd64);
	  d++;
	  if (d64 != rd64) {
	    LOGE("%s: Miscomapre @0x%08x act=0x%016llx exp=0x%016llx\n",__FUNCTION__,ddr3_base + i*8,rd64,d64);
	    errCnt++;
	    break;
	  }
	}
	s++;
      }
      if (!errCnt) {
	LOGI("%s: File %s preload and read-back OK size=%d bytes\n",__FUNCTION__,imageF,bCnt);
      } else {
	LOGE("%s: File %s has error size=%d bytes\n",__FUNCTION__,imageF,bCnt);	
      }
    }    
    fclose(fd);
  }
  //
  for (i=0;i<4;i++) {
    errCnt += clear_printf_mem(i);
  }
  // turn back on
  DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, 1);
  DUT_WRITE_DVT(DVTF_SET_IPC_DELAY, DVTF_SET_IPC_DELAY, 1);
  
  //
  DUT_WRITE_DVT(DVTF_PROGRAM_LOADED,DVTF_PROGRAM_LOADED, 1);

#endif
  return errCnt;
}

int __prIdx[MAX_CORES] = {0,0,0,0};

// spin on the scratch mem until it is good or bad
int check_bare_status(int coreId,int maxTimeOut) {
  int errCnt=0;  
  //
#ifdef SIM_ENV_ONLY
  uint64_t d64,offS;
  uint32_t d32,testId;
  int i=0,done=0;
  LOGI("%s: maxTimeOut=%d\n",__FUNCTION__,maxTimeOut);  
  offS = cep_scratch_mem + (coreId*cep_cache_size);
  while (!done && (i < maxTimeOut)) {
    //
    // check if there is a printf??
    //
    while (1) {
      uint32_t p_adr = cep_printf_mem + (coreId*cep_printf_core_size) + (cep_printf_str_max*__prIdx[coreId]);
      DUT_READ32_64(p_adr,d64);
      //LOGI("%s: PRINTF**** p_adr=0x%x d=0x%016llx i=%d\n",__FUNCTION__,p_adr,d64,__prIdx[coreId]);
      if (d64 != 0) {
	// get the string!!!
	DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, (p_adr & ~0x3) | (coreId & 0x3));
	DUT_WRITE_DVT(DVTF_PRINTF_CMD,DVTF_PRINTF_CMD,1);
	//
	__prIdx[coreId] = (__prIdx[coreId] + 1) % cep_printf_max_lines;
      } else {
	break;
      }
    }
    //
    DUT_READ32_64(offS, d64);
    testId = d64 >> 32;
    d32 = d64 & 0xFFFFFFFF;
    if (d32 == CEP_GOOD_STATUS) {
      LOGI("%s: GOOD offS=0x%016llx Status=0x%016llx detected..i=%d\n",__FUNCTION__,offS,d64,i);
      done = 1;
    }
    else if (d32 == CEP_BAD_STATUS) {
      LOGI("%s: BAD offS=0x%016llx Status=0x%016llx detected..i=%d\n",__FUNCTION__,offS,d64,i);
      done = 1;
      errCnt++;
    }
    i++;
    if (!done) {
      LOGI("%s: offS=0x%016llx d64=0x%016llx..i=%d pIdx=%d\n",__FUNCTION__,offS,d64,i,__prIdx[coreId]);
      DUT_RUNCLK(1000);
    }
  }
  if (i >= maxTimeOut) {
      LOGI("%s: max Timeout offS=0x%016llx Status=0x%016llx..\n",__FUNCTION__,offS,d64);
      errCnt++;    
  }
  //
  // put core in reset
  //
  DUT_RUNCLK(100);
  LOGI("%s: Putting core in reset to stop the PC...\n",__FUNCTION__);
  DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, coreId); // at cycle 10
  DUT_WRITE_DVT(DVTF_PUT_CORE_IN_RESET, DVTF_PUT_CORE_IN_RESET, 1);

#endif
  return errCnt;
}
    
//
// BARE Metal ONLY
//
int set_status(int errCnt, int testId) {
#ifdef BARE_MODE  
  int i=0, coreId;
  uint64_t d64, offS;
  //
  // which core???
  //
  coreId = read_csr(mhartid);
  offS = cep_scratch_mem + (coreId*cep_cache_size);
  //
  // Set the status
  //
  if (errCnt) {
    *(volatile uint64_t *)offS = ((u_int64_t)testId << 32) | CEP_BAD_STATUS;
  } else {
    *(volatile uint64_t *)offS = ((u_int64_t)testId << 32) | CEP_GOOD_STATUS;
  }
  *(volatile uint64_t *)(offS+8) = coreId;
    
  //
  //
  // force a cache flush?? 
  //
  //asm volatile ("fence"); // flush????
  //
  while (i < 16) {
    d64 = *(volatile uint64_t *)(offS | ((1<<i)*64));
    if (d64 == CEP_BAD_STATUS) { i += 2; } else { i++; }
  }
  #endif
  return errCnt;
}

//
//
//
