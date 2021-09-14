//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cep_exports.cc/.h
// Program:        Common Evaluation Platform (CEP)
// Description:    Exports to the Linux Diagnostics
// Notes:          
//
//************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <mutex>
#include <sys/mman.h>
#include <malloc.h>
#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cep_io.h"
#include "cep_token.h"
#include "cep_cmds.h"
#include "cep_vars.h"
#include "cep_run.h"
#include "cep_acc.h"
#include "cep_diag.h"
#include "cep_exports.h"
#include "cep_riscv.h"
#include "CEP.h"
#include "cep_srot.h"
//
// ********************
// Simple multi-threaded tests to verify threads are locxked to each core and never moved
// ********************
//
static void thr_test_func(int id) {
  int i;
  int errCnt = thr_waitTilLock(id, 5);
  if (!errCnt) {
    for (i=0;i<50;i++) {
      {
	if (!GET_VAR_VALUE(noCoreLock) && (id != sched_getcpu())) {
	  THR_LOGE("Not locked!! Thread=%d i=%d\n",id,i);
	  cep_set_thr_errCnt(1);
	  return;
	} else if (VERBOSE1()) {
	  THR_LOGI("Thread=%d i=%d\n",id,i);
	}
      }
      usleep(20000);
    }
  }
  cep_set_thr_errCnt(errCnt); 
  return;
}

int run_threadTest(void) {
  int errCnt = 0;
  cep_set_thr_function(thr_test_func);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask)); 
  return errCnt;
}

//
// ********************
// CEP related
// ********************
//
int cep_aWrite(void)
{
  u_int32_t offset = (u_int32_t)get_token(1)->value;
  u_int64_t dat    = (u_int64_t)get_token(2)->value;
  int accType      = (int)cep_get_value_if_any(4,64);
  switch (accType) {
  case 64: 
    lnx_cep_write(offset,dat);
    LOGI("CEP_Write64: offset=0x%08x data=0x%016lx\n",offset, dat);
    break;
  case 32: 
    lnx_cep_write32(offset,(uint32_t)dat);
    LOGI("CEP_Write32: offset=0x%08x data=0x%08x\n",offset, (uint32_t)dat);
    break;
  case 16: 
    lnx_cep_write16(offset,(uint16_t)dat);
    LOGI("CEP_Write16: offset=0x%08x data=0x%04x\n",offset, (uint16_t)dat);
    break;
  case 8: 
    lnx_cep_write8(offset,(uint8_t)dat);
    LOGI("CEP_Write8: offset=0x%08x data=0x%02x\n",offset, (uint8_t)dat);
    break;
  }
  return 0;
}

int cep_aRead(void)
{
  u_int32_t offset = (u_int32_t)get_token(1)->value;
  u_int64_t dat64;
  u_int32_t dat32;
  u_int16_t dat16;
  u_int8_t dat8;
  int accType      = (int)cep_get_value_if_any(3,64);
  //
  switch (accType) {
  case 64: 
    dat64 = lnx_cep_read(offset);
    cep_set_data(0,(u_int64_t)dat64);
    LOGI("CEP_Read64: offset=0x%08x data=0x%016lx\n",offset, dat64);
    break;

  case 32: 
    dat32 = lnx_cep_read32(offset);
    cep_set_data(0,(u_int64_t)dat32);
    LOGI("CEP_Read32: offset=0x%08x data=0x%08x\n",offset, (uint32_t)dat32);
    break;
  case 16: 
    dat16 = lnx_cep_read16(offset);
    cep_set_data(0,(u_int64_t)dat16);
    LOGI("CEP_Read16: offset=0x%08x data=0x%04x\n",offset, (uint16_t)dat16);
    break;
  case 8: 
    dat8 = lnx_cep_read8(offset);
    cep_set_data(0,(u_int64_t)dat8);
    LOGI("CEP_Read8: offset=0x%08x data=0x%02x\n",offset, (uint8_t)dat8);
    break;
  }
  return 0;
}

//
// ********************
// cepRegTest (4 cores)
// ********************
//
#include "cepRegTest.h"
static void cepRegTest_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);  
  //
  if (!errCnt) {
    errCnt += cepRegTest_runTest(id, 64,GET_VAR_VALUE(revCheck), GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  }
  cep_set_thr_errCnt(errCnt);   
}
int run_cepRegTest(void) {
  int errCnt = 0;
  cep_set_thr_function(cepRegTest_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask)); 
  return errCnt;
}
//
// ********************
// lock
// ********************
//
#include "cepLockTest.h"
static void cepLockTest_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);  
  //
  if (!errCnt) {
    errCnt += cepLockTest_runTest(id, 64,2, GET_VAR_VALUE(seed) & 0x3, GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  }
  cep_set_thr_errCnt(errCnt);   
}
int run_cepLockTest(void) {
  int errCnt = 0;
  cep_set_thr_function(cepLockTest_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask)); 
  return errCnt;
}
//
// ********************
// 4-lock
// ********************
//
static void cepMultiLock_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);  
  //
  if (!errCnt) {
    errCnt += cepLockTest_runTest2(id, 64,2, GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  }
  cep_set_thr_errCnt(errCnt);   
}
int run_cepMultiLock(void) {
  int errCnt = 0;
  cep_set_thr_function(cepMultiLock_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask)); 
  return errCnt;
}
//
// ********************
// GPIO
// ********************
//
#include "cepGpioTest.h"
int run_cepGpioTest(void) {
  int errCnt = 0;
  // single thread
  int id = GET_VAR_VALUE(seed) & 0x3; // 
  errCnt += cepGpioTest_runTest(id, GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  return errCnt;
}
//
// ********************
// SPI
// ********************
//
#include "cepSpiTest.h"
int run_cepSpiTest(void) {
  int errCnt = 0;
  // single thread
  int id = GET_VAR_VALUE(seed) & 0x3; // 
  errCnt += cepSpiTest_runTest(id, GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  return errCnt;
}

//
// ********************
// MASKROM
// ********************
//
#include "cepMaskromTest.h"

static void cepMaskromTest_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  errCnt += cepMaskromTest_runTest(id, GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  cep_set_thr_errCnt(errCnt);     
}
 
int run_cepMaskromTest(void) {
  int errCnt = 0;
  //
  cep_set_thr_function(cepMaskromTest_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask));
  return errCnt;  
 }

//
// ********************
// CLINT
// ********************
//
#include "cepClintTest.h"

static void cepClintTest_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  errCnt += cepClintTest_runTest(id, GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  cep_set_thr_errCnt(errCnt);     
}
 
int run_cepClintTest(void) {
  int errCnt = 0;
  //
  cep_set_thr_function(cepClintTest_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask));
  return errCnt;  
 }

//
// ********************
// PLIC
// ********************
//
#include "cepPlicTest.h"
static void cepPlicTest_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  errCnt += cepPlicTest_runTest(id, GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  cep_set_thr_errCnt(errCnt);     
}
 
int run_cepPlicTest(void) {
  int errCnt = 0;
  //
  cep_set_thr_function(cepPlicTest_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask));
  return errCnt;  
 }

#if 0
// priority interrupt test
static void cepPlicPrioIntrTest_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  errCnt += cepPlicTest_prioIntrTest(id, GET_VAR_VALUE(verbose));
  cep_set_thr_errCnt(errCnt);     
}
 
int run_cepPlicPrioIntrTest(void) {
  int errCnt = 0;
  //
  cep_set_thr_function(cepPlicPrioIntrTest_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask));
  return errCnt;  
 }
#endif

//
// ********************
// cepSrotMemTest (single)
// ********************
//
#include "cepSrotMemTest.h"
int run_cepSrotMemTest(void) {
  int errCnt = 0;
  // single thread
  int id = GET_VAR_VALUE(seed) & 0x3; // 
  errCnt += cepSrotMemTest_runTest(id, GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  return errCnt;
}
//
// ********************
// cepSrotTest maxKeyTest (single)
// ********************
//
#include "cepSrotTest.h"
int run_cepSrotMaxKeyTest(void) {
  int errCnt = 0;
  // single thread
  int id = GET_VAR_VALUE(seed) & 0x3; // 
  errCnt += cepSrotTest_maxKeyTest(id, GET_VAR_VALUE(verbose));
  return errCnt;
}

//
// ********************
// DDR3 related
// ********************
//
int ddr3_aWrite(void)
{
  u_int32_t offset = (u_int32_t)get_token(1)->value;
  u_int64_t dat    = (u_int64_t)get_token(2)->value;
  lnx_mem_write(offset,dat);
  LOGI("DDR3_Write: offset=0x%08x data=0x%016lx\n",offset, dat);
  return 0;
}

int ddr3_aRead(void)
{
  u_int32_t offset = (u_int32_t)get_token(1)->value;
  u_int64_t dat    = lnx_mem_read(offset);
  LOGI("DDR3_Read : offset=0x%08x data=0x%016lx\n",offset, dat);  
  // save for later
  cep_set_data(0,(u_int64_t)dat);
  return 0;
}

//
// ********************
// D-cache Cohererency Test
// ********************
//
std::mutex test_mutex;

#define DC_WRITE_64(a,d) { \
    *(volatile uint64_t *)(a)=d;	\
}

#define DC_READ_64(a,d)  { \
    d=*(volatile uint64_t *)(a); \
  }

volatile int dcache_gCnt = 0;
static void dcacheCoherency_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  int i,to;
  int lastId = (MAX_CORES + id -1) % MAX_CORES; // last
  int nextId = (id + 1) % MAX_CORES; // next
  
  u_int64_t *my_ptr = getScratchPtr();  // make local copy
  u_int64_t my_base = (u_int64_t)my_ptr + (id*8);
  u_int64_t lt_base = (u_int64_t)my_ptr + (lastId*8);
  u_int64_t nt_base = (u_int64_t)my_ptr + (nextId*8);  
  //
  // upto 4 for now
  uint64_t pattern[MAX_CORES] = {
    0x0102030405060708LL ,   
    0x1112131415161718LL ,   
    0x2122232425262728LL  ,
    0x3132333435363738LL
  };
  u_int64_t volatile rd64;
  //
  if (VERBOSE1()) {
    LOGI("%s: my_base=0x%016lx last=0x%016lx next=0x%016lx\n",__FUNCTION__,my_base,lt_base,nt_base);
  }
  //
  //
  //
  for (i=0;i<MAX_CORES;i++) {
    //
    // write to my space
    //
    DC_WRITE_64(my_base, pattern[id]);
#if 1
    //
    // LAST
    //    
    to = 100;
    while (to > 0) {
      // check the pattern to make sure it get here
      DC_READ_64(lt_base,rd64);
      //if (rd64 != pattern[lastId]) {
      if (rd64 < pattern[lastId]) {      
	to--;
	usleep(50000);
      } else {
	if (VERBOSE2()) {
	  LOGI("LAST: i=%d adr=0x%016lx expect=0x%016lx act=0x%016lx\n",i,lt_base,pattern[lastId],rd64);
	}
	break;
      }
    }
    if (to <= 0) {
      LOGE("LAST: i=%d adr=0x%016lx expect=0x%016lx act=0x%016lx\n",i,lt_base,pattern[lastId],rd64);		  
      errCnt++;
    }
    //
    // NEXT
    //
    to = 100;
    while (to > 0) {
      // check the pattern to make sure it get here
      DC_READ_64(nt_base,rd64);
      //if (rd64 != pattern[nextId]) {
      if (rd64 < pattern[nextId]) {      
	to--;
	usleep(50000);
      } else {
	if (VERBOSE2()) {
	  LOGI("NEXT: i=%d adr=0x%016lx expect=0x%016lx act=0x%016lx\n",i,nt_base,pattern[nextId],rd64);
	}
	break;
      }
    }
    if (to <= 0) {
      LOGE("NEXT: i=%d adr=0x%016lx expect=0x%016lx act=0x%016lx\n",i,nt_base,pattern[nextId],rd64);		  
      errCnt++;
    }
#endif
    //
    // Now check mine
    //
    if (errCnt) { break; }
    // now check my
    DC_READ_64(my_base,rd64);
    if (pattern[id] != rd64) {
      errCnt++;
      LOGE("ME: i=%d adr=0x%016lx expect=0x%016lx act=0x%016lx\n",i,my_base,pattern[id],rd64);	
    } else if (VERBOSE2()) {
      LOGI("ME: i=%d adr=0x%016lx expect=0x%016lx act=0x%016lx\n",i,my_base,pattern[id],rd64);		
    }
    // next
    pattern[id]     += 0x0101010101010101LLU;
    pattern[nextId] += 0x0101010101010101LLU;
    pattern[lastId] += 0x0101010101010101LLU;    
    if (errCnt) break;
#if 0
    //
    // I am done with my turn
    //
    {
      std::lock_guard<std::mutex> iolock(test_mutex);    
      dcache_gCnt++;
    }
    //
    // wait until all 4 cores do the writes
    //
    to = 100;
    while (((dcache_gCnt % MAX_CORES) != 0) && (to > 0)) {
      to--;
      usleep(50000);      
    }
    if (to <= 0) {
      LOGE("dcache_gCnt=%d\n",dcache_gCnt);
      errCnt++;
      cep_set_thr_errCnt(errCnt);         
      return;
    }
#endif
  }
  //
  // AT the end, every core should see the same thing
  //
  if (VERBOSE1()) {  
    u_int64_t xxx[MAX_CORES];
    for (int i=0;i<MAX_CORES;i++) {
      DC_READ_64((u_int64_t)getScratchPtr() + (i*8), xxx[i]);
#if 0
      if (pattern[i] != xxx[i]) {
	LOGE("DCHK: i=%d exp=0x%016lx act=%016lx\n",i,pattern[i],xxx[i]);
      }
#endif
    }
    LOGI("DUMP: 0x%016lx_%016lx_%016lx_%016lx\n",xxx[0],xxx[1],xxx[2],xxx[3]);
  }
  //
  //
  //
  //
  cep_set_thr_errCnt(errCnt);   
}
int run_dcacheCoherency(void) {
  int errCnt = 0;
  clearScratchPtr();
  //
  dcache_gCnt = 0;  
  cep_set_thr_function(dcacheCoherency_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask));
  return errCnt;  
}

//
// ********************
// I-cache Cohererency Test
// ********************
//
//
// to test I-cahce coherency, play with a little self-modifying-code!!!
//

static void icacheCoherency_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  char expByte = id;
  char newByte = get_selfModCodeValue() ;
  int to, l;
  
  //
  if (VERBOSE1()) {
    LOGI("Entering with SMC = %02x\n",(int)newByte);
  }
  for (l=0;l<8;l++) {
    if (errCnt) { break; }
    //
    to = 100;
    while ((newByte != expByte) &&  (to > 0)) {
      to--;
      usleep(50000);
#ifndef LOCAL_MACHINE      
      asm volatile ("fence r,r"); // flush??
//      asm volatile ("fence.i"); // flush??      
#endif
      newByte = get_selfModCodeValue() ;
    }
    if (to <= 0) {
      LOGE("Timeout while waiting for newByte=0x%02x == myId=%d\n",newByte,id);
      errCnt++;
    } else {
      expByte = (expByte + MAX_CORES) & 0xFF;
      if (VERBOSE1()) {
	LOGI("Got SMC = 0x%02x expByte=0x%02x\n",(int)newByte,(int)expByte);
      }
      set_selfModCodeValue((newByte + 1) & 0xFF, GET_VAR_VALUE(verbose));
      newByte = get_selfModCodeValue() ;      
    }
  }
  //
  //
  //
  //
  cep_set_thr_errCnt(errCnt);     
}

 
int run_icacheCoherency(void) {
  int errCnt = 0;

  volatile char expByte = 0x55, actByte;
  errCnt += set_selfModCodeValue(expByte, GET_VAR_VALUE(verbose));
  actByte=get_selfModCodeValue();
  if (actByte != expByte) {
    LOGE("%s: exp=0x%02x act=%02x\n",__FUNCTION__,expByte,actByte);
    errCnt++;
  } else if (VERBOSE1()) {
    LOGI("%s: exp=0x%02x act=%02x\n",__FUNCTION__,expByte,actByte);
  }
  //
  // reset and run
  //
  if (!errCnt) {
    set_selfModCodeValue(0, GET_VAR_VALUE(verbose)); // start from core 0  
    //
    cep_set_thr_function(icacheCoherency_thr);
    errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask));
  }
  //
  return errCnt;  
}
  
//
// ********************
// DDR3 Test
// ********************
//
#include "cepMemTest.h"
#include "cepDdr3MemTest.h"
static void ddr3Test_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  // each core will test 1MBytes => 128Kx8bytes
  int adrWidth = 20; // 1M
  //
  // adjust to physical space...
  //
  u_int64_t mem_base = (u_int64_t)(ddr3_base_adr) + ((u_int64_t)getScratchPtr() & (u_int64_t)(ddr3_base_size-1)) + (u_int64_t)((1 << adrWidth)*id);
  if (VERBOSE1()) {
    LOGI("mem_base=0x%016lx\n",mem_base)
  }
  errCnt += cepDdr3MemTest_runTest(id, mem_base, adrWidth, GET_VAR_VALUE(seed), GET_VAR_VALUE(longRun), GET_VAR_VALUE(verbose));
  //
  cep_set_thr_errCnt(errCnt);     
}
 
int run_ddr3Test(void) {
  int errCnt = 0;
  //
  cep_set_thr_function(ddr3Test_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask));
  //free(scratch_ptr);
  return errCnt;  
 }

//
// ********************
// Scratch Pad Test
// ********************
//
static void smemTest_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  // each core will test 1MBytes => 128Kx8bytes
  int adrWidth = 16 -2; // 64K / 2
  u_int64_t mem_base = (u_int64_t)scratchpad_base_addr + ((1 << adrWidth)*id);
  if (VERBOSE1()) {
    LOGI("mem_base=0x%016lx\n",mem_base)
  }
#if 0
  errCnt += cepMemTest_runTest(id,mem_base, adrWidth, 64,
			       GET_VAR_VALUE(seed),
			       GET_VAR_VALUE(verbose),
			       GET_VAR_VALUE(longRun));
#else
  errCnt += cepDdr3MemTest_runTest(id, mem_base, adrWidth, GET_VAR_VALUE(seed), GET_VAR_VALUE(longRun), GET_VAR_VALUE(verbose));
#endif

  //
  cep_set_thr_errCnt(errCnt);     
}
 
int run_smemTest(void) {
  int errCnt = 0;
  //
  cep_set_thr_function(smemTest_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask));
  return errCnt;  
 }

//
// ********************
// cepMacroMix Test (4 per test)
// ********************
//
#include "cepMacroMix.h"
static void cepMacroMix_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  errCnt += cepMacroMix_runTest(id, GET_VAR_VALUE(cpuMask), GET_VAR_VALUE(coreMask), GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  cep_set_thr_errCnt(errCnt);     
}
 
int run_cepMacroMix(void) {

  // Initialize the error count to zero
  int errCnt = 0;
  
  // Initialize the core data structures
  initConfig();
  
  // Set the threads function pointer
  cep_set_thr_function(cepMacroMix_thr);

  // clear the signature
  DUT_WRITE32_64(reg_base_addr + cep_core0_status, 0);
  
  // Spawn the cepMacroMix thread
  errCnt += run_multiThreads(GET_VAR_VALUE(cpuMask));
  
  return errCnt;  
 }

//
// ********************
// cepMacroMix badKey
// ********************
//
static void cepMacroBadKey_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  errCnt += cepMacroMix_runBadKeysTest(id, GET_VAR_VALUE(cpuMask), GET_VAR_VALUE(coreMask), GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  cep_set_thr_errCnt(errCnt);     
}
 
int run_cepMacroBadKey(void) {

  // Initialize the error count to zero
  int errCnt = 0;

  // Initialize the core data structures
  initConfig();

  // Set the threads function pointer
  cep_set_thr_function(cepMacroBadKey_thr);

  // clear the signature
  DUT_WRITE32_64(reg_base_addr + cep_core0_status, 0);

  // Spawn the cepMacroMix runBadKey thread
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask));

  return errCnt;  
 }

//
// ********************
// I/D-cache flush
// ********************
//
static void cacheFlush_thr(int id) {
   int errCnt = 0;
   // 32Kbytes = size of Icache
   int expVal = GET_VAR_VALUE(seed) + id;
   //
   u_int64_t *smc_base = (u_int64_t *)memalign((1<<15),(1<<15));
   if (smc_base == NULL) {
     LOGE("%s: Can't malloc mem\n",__FUNCTION__);
     errCnt++;
     cep_set_thr_errCnt(errCnt);          
     return;          
   }
   u_int64_t dyn_smc_ptr= (u_int64_t)smc_base;         
   errCnt += mprotect((void *)(dyn_smc_ptr & ~(u_int64_t)0x7FFF), 0x8000, PROT_WRITE | PROT_READ | PROT_EXEC);
   if (errCnt) {
     LOGE("%s: Can't mprotect mem\n",__FUNCTION__);
     errCnt++;
     cep_set_thr_errCnt(errCnt);          
     return;     
   }
   errCnt += do_DIcache_SMC(smc_base,512,expVal,GET_VAR_VALUE(verbose));
   cep_set_thr_errCnt(errCnt);
   return;
 }

 int run_cacheFlush(void) {
   int errCnt = 0;
   //
   cep_set_thr_function(cacheFlush_thr);
   errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask));
   return errCnt;  
  }

//
// ********************
// lockfreeAtomic
// ********************
//
#include "cepLockfreeAtomic.h"

static void cepLockfreeAtomic_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);  
  //
  if (!errCnt) {
    u_int64_t mem_base = (u_int64_t)getScratchPtr();
    //u_int64_t reg_base = phys_to_virt(reg_base_addr);
    u_int64_t reg_base = getCepRegVirtAdr() + (reg_base_addr & ~cep_adr_mask);
    errCnt += cepLockfreeAtomic_runTest(id, mem_base, reg_base, GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  }
  cep_set_thr_errCnt(errCnt);   
}

int run_cepLockfreeAtomic(void) {
  int errCnt = 0;
  //
  clearScratchPtr();
  //bzero(scratch_ptr,cep_cache_size*scratch_blocks);
  cep_set_thr_function(cepLockfreeAtomic_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask)); 
  //free(scratch_ptr);
  return errCnt;
}

//
// ********************
// LrscOps
// ********************
//
uint64_t playUnit;
static void cepLrscOps_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);  
  //
  if (VERBOSE1()) {
    LOGI("scratch_prt=0x%016lx\n",(u_int64_t)getScratchPtr());
  }  
  if (!errCnt) {
    u_int64_t *ptr;
    u_int64_t mySig,partSig,incVal;
    u_int64_t fixSig[4] = {0x12345678ABCD0000ULL,
			  0x1122334455660000ULL,
			  0x778899aabbcc0000ULL,
			  0xddeeff0011220000ULL};
    int loop = 16;
    //
    // ALl core touch this same register
    //
    // back to physical address.
    //
    //u_int32_t mem_base = ddr3_base_adr + (u_int32_t)((u_int64_t)scratch_ptr & (u_int64_t)(ddr3_base_size-1));
    ptr = &playUnit;
    incVal = 4;
    switch (id) {
    case 0:
      mySig   = fixSig[0] + 0x0;
      partSig = fixSig[1] + 0x1; // next coreId
      *ptr = mySig; // DUT_WRITE32_64(ptr, mySig); // start for core 0: need physical adr
      break;
    case 1:
      mySig   = fixSig[1] + 0x1;
      partSig = fixSig[2] + 0x2; // next coreId
      break;
    case 2:
      mySig   = fixSig[2] + 0x2;
      partSig = fixSig[3] + 0x3; // next coreId
      break;
    case 3:
      mySig   = fixSig[3] + 0x3;
      partSig = fixSig[0] + 0x4; // next coreId
      break;
    }
    errCnt += cep_exchAtomicTest(id,ptr, mySig, partSig, incVal, loop);
  }
  cep_set_thr_errCnt(errCnt);   
}

int run_cepLrscOps(void) {
  int errCnt = 0;
  //
  cep_set_thr_function(cepLrscOps_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask)); 
  //free(scratch_ptr);
  return errCnt;
}

//
// ********************
// accessTest
// ********************
//
#include "cepAccessTest.h"

static void cepAccessTest_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);  
  //
  if (!errCnt) {
    errCnt += cepAccessTest_runTest(id, GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));
  }
  cep_set_thr_errCnt(errCnt);   
}

int run_cepAccessTest(void) {
  int errCnt = 0;
  //
  cep_set_thr_function(cepAccessTest_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask)); 
  return errCnt;
}

//
// ********************
// atomicTest
// ********************
//
static void cepAtomicTest_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);  
  //
  if (!errCnt) {
    uint64_t *ptr;
    uint64_t expVal;
    uint64_t atomicOps_playUnit[4];
    u_int64_t reg_base   = getCepRegVirtAdr() + (reg_base_addr & ~cep_adr_mask);
    u_int64_t clint_base = getCepSysVirtAdr() + clint_base_addr;
    int coreId = id;
    //
    //
    //
    for (int l=1;l<3;l++) { // skip clint for now
      switch (l) {
      case 0 : 
	// cbus via CLINT
	ptr = (uint64_t *)(clint_base + clint_mtimecmp_offset + (coreId*8));      
	expVal = ~(clint_base_addr + clint_mtimecmp_offset * (coreId*8));
	break;
	
      case 1 : 
	// mbus: cacheable memory
	ptr = &atomicOps_playUnit[coreId];
	expVal = (reg_base + cep_scratch0_reg * (coreId*8));      
	break;
	
      case 2 :  
	// pbus
	ptr = (uint64_t *)(reg_base + cep_scratch0_reg + (coreId*8));      
	expVal = ~(reg_base + cep_scratch0_reg * (coreId*8));
	break;
	
      }
      //
      //
      if (!errCnt) { errCnt = cep_runAtomicTest(ptr,expVal); }
    }
  }
  cep_set_thr_errCnt(errCnt);   
}

int run_cepAtomicTest(void) {
  int errCnt = 0;
  //
  cep_set_thr_function(cepAtomicTest_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(cpuMask)); 
  return errCnt;
}


//
// ********************
// sortErrorTest
// ********************
//
int run_cepSrotErrTest(void) {

  // Initialize the error count to zero
  int errCnt = 0;
  
   // Initialize the core data structures
  initConfig();

  int id = GET_VAR_VALUE(cpuMask) & 0x3;

  // Initialize the SRoT object 
  cep_srot srot(SROT_INDEX, CEP_VERSION_REG_INDEX, GET_VAR_VALUE(verbose));
 
  // Set the mask... from the seed (which we passed the parent's mask for this test)
  srot.SetCpuActiveMask(1 << id);
  
  // Call the Error Test
  errCnt = srot.LLKI_ErrorTest(id);
  
  return errCnt;
}

//
// ********************
// CEP MultiThread
// ********************
//
#include "cepMultiThread.h"
u_int64_t testLockBuf=0;

static void cepMultiThread_thr(int id) {

    // Set some parameters
    int errCnt = 	0;
    int maxTest = 10;
    int cpuMask = ((1 << maxTest) -1) & GET_VAR_VALUE(coreMask);
    int maxLoop = 5;

    errCnt += cepMultiThread_runThr(id, testLockBuf, cpuMask, maxTest, maxLoop, GET_VAR_VALUE(seed), GET_VAR_VALUE(verbose));

    // Aggregate thread's error count
    cep_set_mthr_errCnt(id, errCnt);     

}
 
int run_cepMultiThread(void) {
 
  // Initialize the error count to zero
  int errCnt = 0;

  // Initialize the core data structures
  initConfig();

  // Set the threads function pointer
  cep_set_thr_function(cepMultiThread_thr);

  int maxTest = 10;

#ifdef USE_ATOMIC  
  testLockBuf = (u_int64_t)getScratchPtr(); // virtual
  clearScratchPtr();
#else  
  testLockBuf = (u_int64_t)(ddr3_base_adr) + ((u_int64_t)getScratchPtr() & (u_int64_t)(ddr3_base_size-1)); // physical
#endif  

  errCnt += cepMultiThread_setup(0, testLockBuf, maxTest, 1, GET_VAR_VALUE(verbose)) ;

  if (!errCnt) { errCnt = run_multiThreadFloats(GET_VAR_VALUE(maxThreads)); }

  return errCnt;  
 }


