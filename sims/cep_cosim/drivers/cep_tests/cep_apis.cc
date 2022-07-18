//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cep_apis.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//--------------------------------------------------------------------------------------

#include "v2c_cmds.h"
#include "portable_io.h"

#ifdef SIM_ENV_ONLY
  #include "simPio.h"
#else
  #include <stdint.h>
#endif

#include "CEP.h"
#include "cep_adrMap.h"
#include "cep_apis.h"

#ifdef BARE_MODE
#include "encoding.h"
#endif

// Private bare-metal printf pointer
int __prIdx[MAX_CORES] = {0, 0, 0, 0};

int is_program_loaded(int maxTimeOut) {
  int errCnt = 0;
  int loaded = 0;
#ifdef SIM_ENV_ONLY
  int to = 0;  
  while (!loaded && (to < maxTimeOut)) {
    loaded = DUT_READ_DVT(DVTF_GET_PROGRAM_LOADED, DVTF_GET_PROGRAM_LOADED);
    DUT_RUNCLK(1000);
    to++;
  }
  if (!loaded) {
    LOGE("ERROR: Program loading timeout\n");
    errCnt++;
  } else {
    LOGI("OK: Program loading is completed\n");    
  }
#endif
  return errCnt;
}

void set_backdoor_select(int backdoor) {
#ifdef SIM_ENV_ONLY
  DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, backdoor);
  DUT_WRITE_DVT(DVTF_SET_BACKDOOR_SELECT, DVTF_SET_BACKDOOR_SELECT, 1);
#endif 
}

void enable_bootrom_uart (void)
{
#ifdef SIM_ENV_ONLY
  DUT_WRITE_DVT(DVTF_BOOTROM_ENABLE_UART, DVTF_BOOTROM_ENABLE_UART, 1);
#endif
}

void enable_bootrom_sdboot (void)
{
#ifdef SIM_ENV_ONLY
  DUT_WRITE_DVT(DVTF_BOOTROM_ENABLE_SDBOOT, DVTF_BOOTROM_ENABLE_SDBOOT, 1);
#endif
}

void set_uart_loopback(int loopback)
{
#ifdef SIM_ENV_ONLY
  DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, loopback);
  DUT_WRITE_DVT(DVTF_CONTROL_UART_LOOPBACK, DVTF_CONTROL_UART_LOOPBACK, 1);
#endif
}

void set_spi_loopback(int loopback)
{
#ifdef SIM_ENV_ONLY
  DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, loopback);
  DUT_WRITE_DVT(DVTF_CONTROL_SPI_LOOPBACK, DVTF_CONTROL_SPI_LOOPBACK, 1);
#endif
}

void release_tile_reset(int cpuId)
{
#ifdef SIM_ENV_ONLY
  DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, cpuId);
  DUT_WRITE_DVT(DVTF_RELEASE_TILE_RESET, DVTF_RELEASE_TILE_RESET, 1);
#endif
}

void release_core_reset(int cpuId)
{
#ifdef SIM_ENV_ONLY
  DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, cpuId);
  DUT_WRITE_DVT(DVTF_RELEASE_CORE_RESET, DVTF_RELEASE_CORE_RESET, 1);
#endif
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

// Load a file into Main Memory or the SD Flash Model (must be called from the system thread)
// mem_base is no longer used and it is assumed to be the start of the selected memory
int loadMemory(char *imageF, int fileOffset, int maxByteCnt) {
  int errCnt = 0;

  #ifdef SIM_ENV_ONLY  
    FILE      *fd       = NULL;
    uint64_t  d64;
    uint64_t  rd64;
    int       f         = 0;
    int       d         = 0;
    int       bCnt      = 0;  
    int       fileSize  = 0;

    // Open binary file
    fd = fopen(imageF, "rb");

    if (fd == NULL) {
      LOGE("Can't open file %s\n",imageF);
      return 1;
    }

    // Determine if file size (minus fileOffset exceeds the maximum byte count)
    fseek(fd, 0, SEEK_END);
    fileSize = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    if (fileOffset >= fileSize) {
      LOGE("%s: fileOffset exceeds file size = %0dB/%0dB\n", __FUNCTION__, fileOffset, fileSize);
      fclose(fd);
      return 1;
    }

    if ((fileSize - fileOffset) > maxByteCnt) {
      LOGE("%s: File size (minus starting offset) exceeds maximum allowed = %0dB/%0dB\n", __FUNCTION__, fileSize - fileOffset, maxByteCnt);
      fclose(fd);
      return 1;
    }

    // Which memory are we loading?
    DUT_WRITE_DVT(DVTF_GET_BACKDOOR_SELECT, DVTF_GET_BACKDOOR_SELECT, 1);
    if (DUT_READ_DVT(DVTF_PAT_HI, DVTF_PAT_LO) == 1) {
      LOGI("%s: Loading file %s to SD Flash with maxByteCnt of %0dB and fileOffset = %0dB\n",__FUNCTION__, imageF, maxByteCnt, fileOffset);  
    } else {
      LOGI("%s: Loading file %s to SCRATCHPAD RAM with maxByteCnt of %0dB and fileOffset = %0dB\n",__FUNCTION__, imageF, maxByteCnt, fileOffset);  
    }


    // Read from the file and load into the memory (via backdoor if enabled)
    while (!feof(fd)) {

      // Read 8-bytes from the file      
      fread(&d64, sizeof(uint64_t), 1, fd);

      // If we have reached the file offset, then write the value to memory
      if ((f * 8) >= fileOffset) {
        DUT_WRITE32_64((d * 8), d64);
        d++;
      }

      // Increment the file "offset"
      f++;

      // Calculate the current byte count
      bCnt = (d + 1) * 8;

    } // end while
    
    // Ensure we fill out the cache line
    if ((d & 0x7) != 0) {
      d64 = 0xDEADDEADDEADDEADLL;      
      LOGI("%s: flushing cache line\n",__FUNCTION__);
      while ((d & 0x7) != 0) {
        DUT_WRITE32_64((d * 8), d64);
        d++;
      }
    }

    // Close the file descriptor
    fclose(fd);

    // Indicate that a program has been loaded (assuming there was no error)
    if (!errCnt) {
      LOGI("%s: Setting program loaded flag\n", __FUNCTION__);
      DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, 1);
      DUT_WRITE_DVT(DVTF_SET_PROGRAM_LOADED, DVTF_SET_PROGRAM_LOADED, 1);
    }

  #endif // #ifdef SIM_ENV_ONLY

  return errCnt;
} // loadMemory

// Check PassFail
int check_PassFail_status(int cpuId, int maxTimeOut) {
  int errCnt      = 0;
#ifdef SIM_ENV_ONLY  
  int passMask    = 0;
  int failMask    = 0;
  int inReset;
  uint64_t d64;
  int wait4reset  = 1000;
  //
  while (wait4reset > 0) {
    DUT_WRITE_DVT(DVTF_GET_CORE_RESET_STATUS, DVTF_GET_CORE_RESET_STATUS, 1);
    inReset = DUT_READ_DVT(DVTF_PAT_LO, DVTF_PAT_LO);
    if (inReset == 0) break;
    wait4reset--;
    if (wait4reset <= 0) {
      LOGE("Timeout while waiting for release of reset\n");
      errCnt++;
      break;
    }
    DUT_RUNCLK(1000);    
  }

  // Poll the Pass/Fail Status
  while ((errCnt == 0) && (maxTimeOut > 0)) {
    // only if I am out of reset
    DUT_WRITE_DVT(DVTF_GET_PASS_FAIL_STATUS, DVTF_GET_PASS_FAIL_STATUS, 1);
    d64 = DUT_READ_DVT(DVTF_PAT_HI, DVTF_PAT_LO);
    passMask = (d64 & 0x1); 
    failMask = ((d64>>1) & 0x1); 
    LOGI("Current Pass = 0x%x Fail = 0x%x maxTimeOut = %d\n",passMask,failMask,maxTimeOut);
    if (passMask | failMask) break;
    maxTimeOut--;    
    if (maxTimeOut <= 0) {
      LOGE("Time out while waiting for Pass/Fail\n");
      errCnt++;
    }
    DUT_RUNCLK(1000);        
  }
  errCnt += failMask != 0;
#endif
  return errCnt;
}

// Monitor the "bare" status of the specified core
int check_bare_status(int cpuId, int maxTimeOut) {

  int errCnt = 0;

  // This function is only relevant in Bare Metal and Simulation Mode
  #ifdef SIM_ENV_ONLY
    uint64_t  d64;
    uint64_t  offS;
    uint32_t  d32;
    uint32_t  testId;
    int       i         = 0;
    int       done      = 0;
    int       uart_busy = 0;
  
    LOGI("%s: cpuId = %0d, maxTimeOut = %0d\n", __FUNCTION__, cpuId, maxTimeOut);  
  
    // Loop until a done or timeout is detected
    while (!done && (i < maxTimeOut)) {
    
      // Read the core status (from the core status register)
      DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, cpuId);
      DUT_WRITE_DVT(DVTF_GET_CORE_STATUS, DVTF_GET_CORE_STATUS, 1);
      d64 = DUT_READ_DVT(DVTF_PAT_HI, DVTF_PAT_LO);
      testId  = (d64 >> 32) & 0xFFFFFFFF;
      d32     = d64 & 0xFFFFFFFF;

      // Check status
      if (d32 == CEP_GOOD_STATUS) {
        LOGI("%s: GOOD Status: cpuId = %0d, testId = %0d, i = %0d\n",__FUNCTION__, cpuId, testId, i);
        done = 1;
      } else if (d32 == CEP_BAD_STATUS) {
        LOGI("%s: BAD Status: cpuId = %0d, testId = %0d, i = %0d\n",__FUNCTION__, cpuId, testId, i);
        done = 1;
        errCnt++;
      } // end if (d32 == CEP_GOOD_STATUS)
    
      // Increment the timeout counter
      i++;

      if (!done) {
        LOGI("%s: NOT DONE Status: cpuId = %0d, i = %0d\n",__FUNCTION__, cpuId, i);
        DUT_RUNCLK(1000);
      } // end if (!done)

  } // while (!done && (i < maxTimeOut))
  
  // Has a timeout occurred?
  if (i >= maxTimeOut) {
      LOGI("%s: TIMEOUT: cpuId = %0d, i = %0d\n",__FUNCTION__, cpuId, i);
      errCnt++;    
  }

  // Check to see if the UART is busy before putting the core in reset.  If it
  // is busy, wait a few clocks and check again
  do {
    DUT_WRITE_DVT(DVTF_UART_BUSY, DVTF_UART_BUSY, 1);
    uart_busy = DUT_READ_DVT(DVTF_PAT_LO, DVTF_PAT_LO);
    if (uart_busy) {
      DUT_RUNCLK(1000);
    }
  } while (uart_busy);

  // Put the core, not the tile into reset to stop the program counter
  // If the tile is reset, then it MIGHT lock up the tilelink bus... and thus
  // prevent other cores from completing their work
  DUT_RUNCLK(1000);
  LOGI("%s: Putting core in reset to stop the PC...\n",__FUNCTION__);
  DUT_WRITE_DVT(DVTF_PAT_HI, DVTF_PAT_LO, cpuId);
  DUT_WRITE_DVT(DVTF_FORCE_CORE_RESET, DVTF_FORCE_CORE_RESET, 1);

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
  
  // which core???
  coreId = read_csr(mhartid);
  d64 = ((u_int64_t)coreId << 32) | (u_int64_t)status;
  offS = CEPREGS_BASE_ADDR + cep_core0_status + (coreId * 8);
  *(volatile uint64_t *)(offS) = d64;
#endif
}


// for single core 0 ONLY
void set_pass(void) {
#ifdef BARE_MODE    
  uint64_t d64, offS;
  d64 = CEP_GOOD_STATUS;
  offS = CEPREGS_BASE_ADDR + cep_core0_status;
  *(volatile uint64_t *)(offS) = d64;
#endif
}

void set_fail(void) {
#ifdef BARE_MODE    
  uint64_t d64, offS;
  d64 = CEP_BAD_STATUS;
  offS = CEPREGS_BASE_ADDR + cep_core0_status;
  *(volatile uint64_t *)(offS) = d64;
#endif
}


// Store test status and testId in the current core's status register
int set_status(int errCnt, int testId) {
  #ifdef BARE_MODE  
    int       coreId;
    uint64_t  d64;
    uint64_t  offS;

    coreId = read_csr(mhartid);
    if (errCnt) {
      d64 = ((u_int64_t)testId << 32) | CEP_BAD_STATUS;
    } else {
      d64 = ((u_int64_t)testId << 32) | CEP_GOOD_STATUS;
    }

    // Write status to the current core's status register
    offS = CEPREGS_BASE_ADDR + cep_core0_status + (coreId * 8);
    *(volatile uint64_t *)(offS) = d64;
    
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
       
int cep_playback(uint64_t *cmdSeq, uint64_t upperAdr, uint64_t lowerAdr, int totalCmds, int verbose) {
  if (verbose) {
    LOGI("%s: playback command sequence totalCmds=%d\n",__FUNCTION__,totalCmds);
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

  if (verbose)
  	LOGI("%s: no errors detected\n");

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
    cep_raw_write(CEPREGS_BASE_ADDR + testNset_reg, dat64);
    rdDat = cep_raw_read(CEPREGS_BASE_ADDR + testNset_reg);
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
  cep_raw_write(CEPREGS_BASE_ADDR + testNset_reg, dat64);
}

int cep_get_lock_status(int myId, int lockNum, int *lockMaster) {
  uint64_t dat64;
  dat64 = cep_raw_read(CEPREGS_BASE_ADDR + testNset_reg);
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

