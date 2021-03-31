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

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepLockfreeAtomic.h"

#include "cep_apis.h"
#include "cep_riscv.h"
#include "portable_io.h"


#ifdef BARE_MODE

#else
#include "simPio.h"
#endif

//#define ATOMIC_16_8_SUPPORTED
//
// =============================
// The test itself
// =============================
//
static int cepLockfreeAtomic_64(int cpuId, uint64_t *adr, int seed, int verbose) {

  int i,errCnt = 0;
  uint64_t mask = (uint64_t)0xFFFF << (cpuId * 16); // each core get to play with 16-bit out of 64
  uint64_t exp  = 0, act;
  uint64_t add  = (uint64_t)1 << (cpuId * 16);
  uint64_t fullAdd = 0x0001000100010001;
  uint64_t expFull = 0;
  if (verbose) {
    LOGI("%s:\n",__FUNCTION__);
  }
  // first clear my bits with fetch_and
  //(void)__atomic_thread_fence(__ATOMIC_SEQ_CST);
  (void)__atomic_fetch_and(adr,~mask,__ATOMIC_SEQ_CST);
  //
  for (i=1;i<=8;i++) {
    act = __atomic_fetch_add(adr,add,__ATOMIC_SEQ_CST);
    // check
    if (((exp ^ act) & mask) != 0) {
      LOGE("%s: ERROR cpuId=%d offset=0x%016lx exp=0x%016lx act=0x%016lx mask=0x%016lx\n",
	   __FUNCTION__,cpuId,(uint64_t)adr,exp,act,mask);
      errCnt = i;
      break;
    } else if (verbose) {
      LOGI("%s: OK cpuId=%d offset=0x%016lx exp=0x%016lx act=0x%016lx mask=0x%016lx\n",
	   __FUNCTION__,cpuId,(uint64_t)adr,exp,act,mask);
    }
    exp += add;
    expFull += fullAdd;
  }
  //
  // at the end read to make sure all 4 are OK
  //
  if (!errCnt) {
    int tout = 20;
    do {
      act = __atomic_load_n(adr,__ATOMIC_SEQ_CST);
      if (expFull == act) {
	break;
      }
      tout--;
      if (tout <= 0) { errCnt += 0x100; break; }
    } while (tout > 0);
  }
  return errCnt;
}

static int cepLockfreeAtomic_32(int cpuId, uint32_t *adr, int seed, int verbose) {

  int i,errCnt = 0;
  uint32_t mask = (uint32_t)0xFF << (cpuId * 8); // each core get to play with 8-bit out of 32
  uint32_t exp  = 0, act;
  uint32_t add  = (uint32_t)1 << (cpuId * 8);
  uint32_t fullAdd = 0x01010101;
  uint32_t expFull = 0;
  if (verbose) {
    LOGI("%s:\n",__FUNCTION__);
  }
  // first clear my bits with fetch_and
  //(void)__atomic_thread_fence(__ATOMIC_SEQ_CST);
  (void)__atomic_fetch_and(adr,~mask,__ATOMIC_SEQ_CST);
  //
  for (i=1;i<=8;i++) {
    act = __atomic_fetch_add(adr,add,__ATOMIC_SEQ_CST);
    // check
    if (((exp ^ act) & mask) != 0) {
      LOGE("%s: ERROR cpuId=%d offset=0x%016lx exp=0x%08x act=0x%08x mask=0x%08x\n",
	   __FUNCTION__,cpuId,(uint64_t)adr,exp,act,mask);
      errCnt = i;
      break;
    } else if (verbose) {
      LOGI("%s: OK cpuId=%d offset=0x%016lx exp=0x%08x act=0x%08x mask=0x%08x\n",
	   __FUNCTION__,cpuId,(uint64_t)adr,exp,act,mask);
    }
    exp += add;
    expFull += fullAdd;
  }
  //
  // at the end read to make sure all 4 are OK
  //
  if (!errCnt) {
    int tout = 20;
    do {
      act = __atomic_load_n(adr,__ATOMIC_SEQ_CST);
      if (expFull == act) {
	break;
      }
      tout--;
      if (tout <= 0) { errCnt += 0x100; break; }
    } while (tout > 0);
  }
  return errCnt;
}

#ifdef ATOMIC_16_8_SUPPORTED
static int cepLockfreeAtomic_16(int cpuId, uint16_t *adr, int seed, int verbose) {

  int i,errCnt = 0;
  uint16_t mask = (uint16_t)0xF << (cpuId * 4); // each core get to play with 4-bit out of 16
  uint16_t exp  = 0, act;
  uint16_t add  = (uint16_t)1 << (cpuId * 4);
  uint16_t fullAdd = 0x1111;
  uint16_t expFull = 0;
  if (verbose) {
    LOGI("%s:\n",__FUNCTION__);
  }
  // first clear my bits with fetch_and
  //(void)__atomic_thread_fence(__ATOMIC_SEQ_CST);

//(void)__atomic_fetch_and(adr,~mask,__ATOMIC_SEQ_CST);
(void)__sync_fetch_and_and(adr,~mask);
  //
  for (i=1;i<=8;i++) {
    //act = __atomic_fetch_add(adr,add,__ATOMIC_SEQ_CST);
    act = __sync_fetch_and_add(adr,add);
    // check
    if (((exp ^ act) & mask) != 0) {
      LOGE("%s: ERROR cpuId=%d offset=0x%08x exp=0x%04x act=0x%04x mask=0x%04x\n",
	   __FUNCTION__,cpuId,adr,exp,act,mask);
      errCnt = i;
      break;
    } else if (verbose) {
      LOGI("%s: OK cpuId=%d offset=0x%08x exp=0x%04x act=0x%04x mask=0x%04x\n",
	   __FUNCTION__,cpuId,adr,exp,act,mask);
    }
    exp += add;
    expFull += fullAdd;
  }
  //
  // at the end read to make sure all 4 are OK
  //
  if (!errCnt) {
    int tout = 20;
    do {
      act = __atomic_load_n(adr,__ATOMIC_SEQ_CST);
      if (expFull == act) {
	break;
      }
      tout--;
      if (tout <= 0) { errCnt += 0x100; break; }
    } while (tout > 0);
  }
  return errCnt;
}

static int cepLockfreeAtomic_8(int cpuId, uint8_t *adr, int seed, int verbose) {

  int i,errCnt = 0;
  uint8_t mask = (uint8_t)0x3 << (cpuId * 2); // each core get to play with 2-bit out of 8
  uint8_t exp  = 0, act;
  uint8_t add  = (uint8_t)1 << (cpuId * 2);
  uint8_t fullAdd = 0x55; // 01010101
  uint8_t expFull = 0;
  if (verbose) {
    LOGI("%s:\n",__FUNCTION__);
  }
  // first clear my bits with fetch_and
  //(void)__atomic_thread_fence(__ATOMIC_SEQ_CST);

  (void)__atomic_fetch_and(adr,~mask,__ATOMIC_SEQ_CST);
  //
  for (i=1;i<=3;i++) {
    act = __atomic_fetch_add(adr,add,__ATOMIC_SEQ_CST);
    // check
    if (((exp ^ act) & mask) != 0) {
      LOGE("%s: ERROR cpuId=%d offset=0x%08x exp=0x%02x act=0x%02x mask=0x%02x\n",
	   __FUNCTION__,cpuId,adr,exp,act,mask);
      errCnt = i;
      break;
    } else if (verbose) {
      LOGI("%s: OK cpuId=%d offset=0x%08x exp=0x%02x act=0x%02x mask=0x%02x\n",
	   __FUNCTION__,cpuId,adr,exp,act,mask);
    }
    exp += add;
    expFull += fullAdd;
  }
  //
  // at the end read to make sure all 4 are OK
  //
  if (!errCnt) {
    int tout = 20;
    do {
      act = __atomic_load_n(adr,__ATOMIC_SEQ_CST);
      if (expFull == act) {
	break;
      }
      tout--;
      if (tout <= 0) { errCnt += 0x100; break; }
    } while (tout > 0);
  }
  return errCnt;
}
#endif


int cepLockfreeAtomic_runTest(int cpuId, int seed, int verbose) {
  int errCnt = 0;
  //
  uint64_t *ptr64;
  uint32_t *ptr32;
  //  uint16_t *ptr16;
  //uint8_t *ptr8;
  //
  //
  //  Memory Space
  //
  if (!errCnt) {
    ptr64 = (uint64_t *)(0x90000000);
    errCnt += cepLockfreeAtomic_64(cpuId,ptr64, seed, verbose);
  }
  if (!errCnt) {
    ptr32 = (uint32_t *)(0x90000100);
    errCnt += cepLockfreeAtomic_32(cpuId,ptr32, seed, verbose);
  }
#ifdef ATOMIC_16_8_SUPPORTED
  if (!errCnt) {
    ptr16 = (uint16_t *)(0x90000200);
    errCnt += cepLockfreeAtomic_16(cpuId,ptr16, seed, verbose);
  }
  if (!errCnt) {
    ptr8 = (uint8_t *)(0x90000300);
    errCnt += cepLockfreeAtomic_8(cpuId,ptr8, seed, verbose);
  }
#endif
  //
  // 64 via reg space
  //
  if (!errCnt) {
    ptr64 = (uint64_t *)(reg_base_addr + cep_scratch0_reg);
    errCnt += cepLockfreeAtomic_64(cpuId,ptr64, seed, verbose);
  }
  //
  return errCnt;
}
