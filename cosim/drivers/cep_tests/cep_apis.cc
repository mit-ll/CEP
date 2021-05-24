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
// 
//
int read_binFile(char *imageF, uint64_t *buf, int wordCnt) {
#ifdef SIM_ENV_ONLY  
  FILE *fd=NULL;
  int i = 0;
  fd=fopen(imageF,"rb");
  while (!feof(fd) && (i < wordCnt)) {
    fread(&(buf[i]),sizeof(uint64_t),1,fd);
    LOGI("fread i=%d 0x%016lx\n",i,buf[i]);
    i++;
  }
  fclose(fd);
#endif
  return 0;
}

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

// Check PassFail
int check_PassFail_status(int coreId,int maxTimeOut) {
  int errCnt = 0;
#ifdef SIM_ENV_ONLY  
  int passMask = 0;
  int failMask = 0;
  int inReset;
  uint64_t d64;
  int wait4reset = 1000;
  //
  while (wait4reset > 0) {
    DUT_WRITE_DVT(DVTF_GET_CORE_RESET_STATUS, DVTF_GET_CORE_RESET_STATUS, 1);
    inReset = DUT_READ_DVT(DVTF_PAT_LO, DVTF_PAT_LO);
    if (inReset == 0) break;
    // may be the test already pass
    DUT_WRITE_DVT(DVTF_GET_PASS_FAIL_STATUS, DVTF_GET_PASS_FAIL_STATUS, 1);
    d64 = DUT_READ_DVT(DVTF_PAT_HI, DVTF_PAT_LO);
    passMask = (d64 & 0x1); 
    failMask = ((d64>>1) & 0x1); 
    LOGI("Current Pass=0x%x Fail=0x%x maxTimeOut=%d\n",passMask,failMask,maxTimeOut);
    if (passMask | failMask) break;    
    //
    wait4reset--;
    if (wait4reset <= 0) {
      LOGE("Timeout while waiting to be active\n");
      errCnt++;
      break;
    }
    DUT_RUNCLK(1000);    
  }
  while ((errCnt == 0) && (maxTimeOut > 0)) {
    // only if I am out of reset
    DUT_WRITE_DVT(DVTF_GET_PASS_FAIL_STATUS, DVTF_GET_PASS_FAIL_STATUS, 1);
    d64 = DUT_READ_DVT(DVTF_PAT_HI, DVTF_PAT_LO);
    passMask = (d64 & 0x1); 
    failMask = ((d64>>1) & 0x1); 
    LOGI("Current Pass=0x%x Fail=0x%x maxTimeOut=%d\n",passMask,failMask,maxTimeOut);
    if (passMask | failMask) break;
    maxTimeOut--;    
    if (maxTimeOut <= 0) {
      LOGE("Time out while waiting for Pass/Fail\n");
      errCnt++;
    }
    DUT_RUNCLK(1000);        
  }
  errCnt += failMask != 0;
  if (errCnt) {
    DUT_WRITE_DVT(DVTF_SET_PASS_FAIL_STATUS, DVTF_SET_PASS_FAIL_STATUS, 1);
  }
#endif
  return errCnt;
}

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
      //
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
#if 1
    DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, coreId);
    DUT_WRITE_DVT(DVTF_GET_CORE_STATUS, DVTF_GET_CORE_STATUS, 1);
    d64 = DUT_READ_DVT(DVTF_PAT_HI, DVTF_PAT_LO);
#else
    DUT_READ32_64(offS, d64);
#endif
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
void set_cur_status(int status) {
#ifdef BARE_MODE  
  int coreId;
  uint64_t d64, offS;
  //
  // which core???
  //
  coreId = read_csr(mhartid);
  d64 = ((u_int64_t)coreId << 32) | (u_int64_t)status;
  // Use core status 05/18/20
  offS = reg_base_addr + cep_core0_status + (coreId * 8);
  *(volatile uint64_t *)(offS) = d64;
#endif
}


// for single core 0 ONLY
void set_pass(void) {
#ifdef BARE_MODE    
  uint64_t d64, offS;
  d64 = CEP_GOOD_STATUS;
  offS = reg_base_addr + cep_core0_status;
  *(volatile uint64_t *)(offS) = d64;
#endif
}

void set_fail(void) {
#ifdef BARE_MODE    
  uint64_t d64, offS;
  d64 = CEP_BAD_STATUS;
  offS = reg_base_addr + cep_core0_status;
  *(volatile uint64_t *)(offS) = d64;
#endif
}


int set_status(int errCnt, int testId) {
#ifdef BARE_MODE  
  int i=0, coreId;
  uint64_t d64, offS, myOffs;
  //
  // which core???
  //
  coreId = read_csr(mhartid);
  if (errCnt) {
    d64 = ((u_int64_t)testId << 32) | CEP_BAD_STATUS;
  } else {
    d64 = ((u_int64_t)testId << 32) | CEP_GOOD_STATUS;
  }
  // Use core status 05/18/20
  offS = reg_base_addr + cep_core0_status + (coreId * 8);
  *(volatile uint64_t *)(offS) = d64;
  //
  // Below are not predictable due to cache flush
  //
  myOffs = cep_scratch_mem + (coreId*cep_cache_size);
  //
  // Set the status
  //
  *(volatile uint64_t *)(myOffs) = d64;
  *(volatile uint64_t *)(myOffs+8) = coreId;
  // write to sratch register that belong to me
  //
  //
  // force a cache flush?? 
  //
  //asm volatile ("fence"); // flush????
  //
  i=0;
  while (i < 18) {
    offS = cep_scratch_mem | ((1<<i)*64);
    d64 = *(volatile uint64_t *)(offS | ((1<<i)*64));
    if (d64 == CEP_BAD_STATUS) { i += 2; } else { i++; }
  }
  #endif

	    
  return errCnt;
}

//
// For playback command file
//
void cep_raw_write(uint64_t pAddress, uint64_t pData) {
#ifdef BARE_MODE
  *(volatile uint64_t*)(pAddress) = pData;  
#elif SIM_ENV_ONLY
  DUT_WRITE32_64(pAddress, pData);
#elif LINUX_MODE
  return lnx_cep_write(pAddress, pData);  
#else
  ; // do nothing!!! memcpy(pAddress, &pData, sizeof(pData));
#endif  
}

uint64_t cep_raw_read(uint64_t pAddress) {
#ifdef BARE_MODE
  return *(volatile uint64_t*)(pAddress);
#elif SIM_ENV_ONLY
  uint64_t d64;
  DUT_READ32_64(pAddress, d64);
  return d64;
#elif LINUX_MODE
  return lnx_cep_read(pAddress);
#else  
  return -1; // NA!!!
#endif
    
}

static int inRange(uint64_t adr, uint64_t upperAdr,uint64_t lowerAdr) {
  return ((adr >= lowerAdr) && (adr < upperAdr)) ? 1 : 0;
}
		   
int cep_playback(uint64_t *cmdSeq, uint64_t upperAdr, uint64_t lowerAdr, int totalCmds, int totalSize, int verbose) {
  if (verbose) {
    LOGI("%s: playback command sequence totalCmds=%d totalSize=%d\n",__FUNCTION__,totalCmds,totalSize);
  }
  int errCnt = 0;
  int i=0, TO;
  uint64_t rdDat;
  for (int c=1;c<=totalCmds;c++) {
    // read the first item
    if (cmdSeq[i] == WRITE__CMD) {
      if (inRange(cmdSeq[i+1],upperAdr,lowerAdr)) {    
	cep_raw_write(cmdSeq[i+1],cmdSeq[i+2]);
      }
      i += WRITE__CMD_SIZE;
    }
    else if (cmdSeq[i] == RDnCMP_CMD) {
      if (inRange(cmdSeq[i+1],upperAdr,lowerAdr)) {    	
	rdDat = cep_raw_read(cmdSeq[i+1]);
	if (rdDat != cmdSeq[i+2]) {
	  LOGE("%s: ERROR Mismatch at cmd=%d adr=0x%016lx exp=0x%016lx act=0x%016lx\n",__FUNCTION__,
	       c, cmdSeq[i+1],cmdSeq[i+2],rdDat);
	  errCnt = c;
	  break;
	} else if (verbose) {
	  LOGI("%s: OK at cmd=%d adr=0x%016lx exp=0x%016lx act=0x%016lx\n",__FUNCTION__,
	       c, cmdSeq[i+1],cmdSeq[i+2],rdDat);	
	}
      }
      i += RDnCMP_CMD_SIZE;
    }
    else if (cmdSeq[i] == RDSPIN_CMD) {
      if (inRange(cmdSeq[i+1],upperAdr,lowerAdr)) {    	
	TO = cmdSeq[i+4];
	while (TO > 0) {
	  rdDat = cep_raw_read(cmdSeq[i+1]);
	  if (((rdDat ^ cmdSeq[i+2]) & cmdSeq[i+3]) == 0) {
	    break;
	  }
	  TO--;
	  if (TO <= 0) {
	    LOGE("%s: timeout at cmd=%d adr=0x%016lx exp=0x%016lx act=0x%016lx\n",__FUNCTION__,
		 c, cmdSeq[i+1],cmdSeq[i+2],rdDat);
	    errCnt = c;
	    break;
	  }
	}
      }
      i += RDSPIN_CMD_SIZE;
    }
  }
  return errCnt;
}

//
// Lock support
//
int cep_get_lock(int myId, int lockNum, int timeOut) {
  int gotIt = 0;
  uint64_t dat64, rdDat, exp;
  dat64 = (((uint64_t)1 << reqLock_bit ) |
	   ((uint64_t)(lockNum & reqLockNum_mask) << reqLockNum_bit) |
	   ((uint64_t)(myId & reqId_mask) << reqId_bit_lo));
  exp = 1 | (myId << 1); // expected
  do {
    // request the lock
    cep_raw_write(reg_base_addr + testNset_reg, dat64);
    rdDat = cep_raw_read(reg_base_addr + testNset_reg);
    if (((rdDat >> (8*lockNum)) & 0xFF) == exp) {
      gotIt = 1;
      break;
    }
    timeOut--;
    USEC_SLEEP(100);
  } while (timeOut > 0);
  if (timeOut <= 0) {
    LOGE("%s: ERROR: timeout\n",__FUNCTION__);
    return -1;
  }
  return gotIt;
}

void cep_release_lock(int myId, int lockNum) {
  uint64_t dat64;
  // just release
  dat64 = (((uint64_t)1 << releaseLock_bit ) |
	   ((uint64_t)(lockNum & reqLockNum_mask) << reqLockNum_bit) |
	   ((uint64_t)(myId & reqId_mask) << reqId_bit_lo));  
  cep_raw_write(reg_base_addr + testNset_reg, dat64);
}

int cep_get_lock_status(int myId, int lockNum, int *lockMaster) {
  uint64_t dat64;
  dat64 = cep_raw_read(reg_base_addr + testNset_reg);
  dat64 = (dat64 >> (8*lockNum)) & 0xFF;
  *lockMaster = dat64>>1;
  return (dat64 & 0x1) ? 1 : 0;
}
//
//
//
int __c2c_captureOn = 0;
void Set_C2C_Capture(int enable) { __c2c_captureOn = enable; }
int Get_C2C_Capture(void) { return __c2c_captureOn; }
void writeDvt(int msb, int lsb, int bits) {
#ifdef SIM_ENV_ONLY    
  DUT_WRITE_DVT(msb,lsb,bits);
#endif
}

//
// Toggle dMI reset
//
void toggleDmiReset(void) {
  //
#ifdef SIM_ENV_ONLY    
  DUT_WRITE_DVT(DVTF_TOGGLE_DMI_RESET_BIT, DVTF_TOGGLE_DMI_RESET_BIT, 1);
  int loop = 100;
  int dmiReset = 1;
  do {
    dmiReset = DUT_READ_DVT(DVTF_TOGGLE_DMI_RESET_BIT, DVTF_TOGGLE_DMI_RESET_BIT);
    loop--;
  } while (dmiReset && (loop > 0));
#endif

}

