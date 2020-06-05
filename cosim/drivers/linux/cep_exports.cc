//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
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
//
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
  errCnt = run_multiThreads(GET_VAR_VALUE(coreMask)); 
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
  lnx_cep_write(offset,dat);
  LOGI("CEP_Write: offset=0x%08x data=0x%016lx\n",offset, dat);
  return 0;
}

int cep_aRead(void)
{
  u_int32_t offset = (u_int32_t)get_token(1)->value;
  u_int64_t dat    = lnx_cep_read(offset);
  LOGI("CEP_Read : offset=0x%08x data=0x%016lx\n",offset, dat);  
  // save for later
  cep_set_data(0,(u_int64_t)dat);
  return 0;
}
// the test
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
  errCnt = run_multiThreads(GET_VAR_VALUE(coreMask)); 
  return errCnt;
}

// lock
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
  errCnt = run_multiThreads(GET_VAR_VALUE(coreMask)); 
  return errCnt;
}
// 4-lock
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
  errCnt = run_multiThreads(GET_VAR_VALUE(coreMask)); 
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
u_int64_t *scratch_ptr;
int scratch_blocks = 1;
void dcacheCoherency_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  int i,to;
  int lastId = (MAX_CORES + id -1) % MAX_CORES; // last
  int nextId = (id + 1) % MAX_CORES; // next
  
  u_int64_t *my_ptr = scratch_ptr;  // make local copy
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
      DC_READ_64((u_int64_t)scratch_ptr + (i*8), xxx[i]);
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
  // allocate memory
  //scratch_ptr = (u_int64_t *)malloc(cep_cache_size*scratch_blocks);
  scratch_ptr = (u_int64_t *)memalign(cep_cache_size,cep_cache_size*scratch_blocks);
  bzero(scratch_ptr,cep_cache_size*scratch_blocks);
  if (VERBOSE1()) {
    LOGI("scrtch_prt=0x%016lx\n",(u_int64_t)scratch_ptr);
  }
  if (scratch_ptr == NULL) {
    LOGE("Can't malloc scratch_mem\n");
    return 1;
  }
  //
  dcache_gCnt = 0;  
  cep_set_thr_function(dcacheCoherency_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(coreMask));
  free(scratch_ptr);
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

void icacheCoherency_thr(int id) {
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
      //asm volatile ("fence.i"); // flush??      
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
    errCnt = run_multiThreads(GET_VAR_VALUE(coreMask));
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
void ddr3Test_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  // each core will test 1MBytes => 128Kx8bytes
  u_int64_t mem_base = (u_int64_t)scratch_ptr + ((1 << 20)*id);
  int adrWidth = 20; // 1M
  if (VERBOSE1()) {
    LOGI("mem_base=0x%016lx\n",mem_base)
  }
  errCnt += cepMemTest_runTest(id,mem_base, adrWidth,
			       GET_VAR_VALUE(seed),
			       GET_VAR_VALUE(verbose),
			       GET_VAR_VALUE(longRun));
  cep_set_thr_errCnt(errCnt);     
}
 
int run_ddr3Test(void) {
  int errCnt = 0;
  // allocate memory
  scratch_ptr = (u_int64_t *)memalign(4*(1<<20),4*(1<<20)); // 4M bytes
  if (scratch_ptr == NULL) {
    LOGE("Can't malloc scratch_mem\n");
    return 1;
  }
  if (VERBOSE1()) {
    LOGI("scrtch_prt=0x%016lx\n",(u_int64_t)scratch_ptr);
  }
  //
  cep_set_thr_function(ddr3Test_thr);
  errCnt = run_multiThreads(GET_VAR_VALUE(coreMask));
  free(scratch_ptr);
  return errCnt;  
 }

//
// ********************
// cepMacroMix Test (4 per test)
// ********************
//
#include "cepregression.h"
#include "cepMacroMix.h"
void cepMacroMix_thr(int id) {
  int errCnt = 	thr_waitTilLock(id, 5);
  errCnt += cepMacroMix_runTest(id,GET_VAR_VALUE(testMask),GET_VAR_VALUE(seed),GET_VAR_VALUE(verbose));
  cep_set_thr_errCnt(errCnt);     
}
 
int run_cepMacroMix(void) {
  int errCnt = 0;
  //
  cep_set_thr_function(cepMacroMix_thr);
  initConfig();
  errCnt = run_multiThreads(GET_VAR_VALUE(coreMask));
  return errCnt;  
 }

int run_cepAllMacros(void) {
  int errCnt =0;
  errCnt += cepregression_test(GET_VAR_VALUE(testMask));
  return errCnt;
}

#include "cep_aes.h"
int run_cep_AES      (void) {
  int errCnt = 0;
  initConfig();
  //errCnt += cep_AES_test();
  cep_aes aes(GET_VAR_VALUE(seed),GET_VAR_VALUE(verbose));
  //
  int maxLoop = 300;
  errCnt += aes.RunAes192Test(maxLoop);
  //
  return errCnt;
}

#include "cep_des3.h"
int run_cep_DES3      (void) {
  int errCnt = 0;
  initConfig();
  
  //errCnt += cep_DES3_test();
  cep_des3 des3(GET_VAR_VALUE(seed),GET_VAR_VALUE(verbose));  
  int maxLoop = 300;
  errCnt += des3.RunDes3Test(maxLoop);
  return errCnt;
}

#include "cep_dft.h"
int run_cep_DFT      (void) {
  int errCnt = 0;
  initConfig();
  //errCnt += cep_DFT_test();
  cep_dft dft(GET_VAR_VALUE(seed),GET_VAR_VALUE(verbose));  
  int maxLoop = 10;
  errCnt += dft.RunDftTest(maxLoop);
  //
  return errCnt;
}
int run_cep_IDFT      (void) {
  int errCnt = 0;
  initConfig();
  errCnt += cep_IDFT_test();
  return errCnt;
}
#include "cep_fir.h"
int run_cep_FIR      (void) {
  int errCnt = 0;
  initConfig();
  //errCnt += cep_FIR_test();
  cep_fir fir(GET_VAR_VALUE(seed),GET_VAR_VALUE(verbose));  
  int maxLoop = 20;
  errCnt += fir.RunFirTest(maxLoop);
  
  return errCnt;
}
#include "cep_iir.h"
int run_cep_IIR      (void) {
  int errCnt = 0;
  initConfig();
  //errCnt += cep_IIR_test();
  cep_iir iir(GET_VAR_VALUE(seed),GET_VAR_VALUE(verbose));  
  int maxLoop = 20;
  errCnt += iir.RunIirTest(maxLoop);
  //
  return errCnt;
}
#include "cep_gps.h"
int run_cep_GPS      (void) {
  int errCnt = 0;
  initConfig();
  //errCnt += cep_GPS_test();
  cep_gps gps(GET_VAR_VALUE(seed),GET_VAR_VALUE(verbose));  
  int maxLoop = 37;
  errCnt += gps.RunGpsTest(maxLoop);
  return errCnt;
}
#include "cep_md5.h"
int run_cep_MD5      (void) {
  int errCnt = 0;
  initConfig();
  //errCnt += cep_MD5_test();
  cep_md5 md5(GET_VAR_VALUE(seed),GET_VAR_VALUE(verbose));  
  int maxLoop = 32;
  errCnt += md5.RunMd5Test(maxLoop);
  return errCnt;
}
int run_cep_RSA      (void) {
  int errCnt = 0;
  initConfig();
  errCnt += cep_RSA_test();
  return errCnt;
}

#include "cep_sha256.h"
int run_cep_SHA256      (void) {
  int errCnt = 0;
  initConfig();
  //errCnt += cep_SHA256_test();
  cep_sha256 sha256(GET_VAR_VALUE(seed),GET_VAR_VALUE(verbose));  
  int maxLoop = 32;
  errCnt += sha256.RunSha256Test(maxLoop);
  //
  
  return errCnt;
}
//
// ********************
// I/D-cache flush
// ********************
//
 void cacheFlush_thr(int id) {
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
   errCnt = run_multiThreads(GET_VAR_VALUE(coreMask));
   return errCnt;  
  }
