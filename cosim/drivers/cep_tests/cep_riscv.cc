// ************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
// ************************************************************************


//
// for self-modifying code!!!!! look for 0x5A!!!
//
/*

// *************************
// This is from native Linux's objdump 
// *************************
volatile char get_selfModCodeValue(void) {
 74:	55                   	push   %rbp
    75:	48 89 e5             	mov    %rsp,%rbp
   /home/aduong/CEP/CEP-master/software/freedom-u-sdk/buildroot/package/cep_diag/src/cep_riscv.cc:33
   return (volatile char)0x5A; // this value will be modified to test I-cache coherency
 78:	b8 5a 00 00 00       	mov    $0x5a,%eax
   /home/aduong/CEP/CEP-master/software/freedom-u-sdk/buildroot/package/cep_diag/src/cep_riscv.cc:34
   }
7d:	5d                   	pop    %rbp
7e:	c3                   	retq  
// *************************
// This is from RISCV's objdump under Linux
// *************************
//
// from OBJDUMP
get_selfModCodeValue():
/home/aduong/CEP/CEP-master/software/freedom-u-sdk/buildroot/package/cep_diag/src/cep_exports.cc:321

char get_selfModCodeValue(void) {
 14a0:	1141                	addi	sp,sp,-16
    14a2:	e422                	sd	s0,8(sp)
    14a4:	0800                	addi	s0,sp,16
    
    00000000000014a6 <.L0 >:
  /home/aduong/CEP/CEP-master/software/freedom-u-sdk/buildroot/package/cep_diag/src/cep_exports.cc:322
    return (volatile char)0x5A; // this value will be modified to test I-cache coherency
 14a6:	05a00793          	li	a5,90
    //
    // *************************	
    // This is from RISCV's objdump under BARE mode
    // *************************	
    //	
    volatile char get_selfModCodeValue(void) {
    return (volatile char)0x5A; // this value will be modified to test I-cache coherency
  }
 80001a12:	05a00513          	li	a0,90
    80001a16:	8082                	ret
#endif
*/

// Only for RISCV
#ifdef RISCV_CPU


#include "v2c_cmds.h"
#include "portable_io.h"

#ifdef SIM_ENV_ONLY
#include "simPio.h"
#else
// SW or BARE in sim
#include <string.h>
#include <mutex>
#include <malloc.h>
#include <stdint.h>
#endif
//
#include "cep_adrMap.h"
#include "cep_apis.h"
#include "cep_riscv.h"

#ifdef BARE_MODE
#include "encoding.h"
#else
#include <sys/mman.h>
#endif


//
// Atomic
// return old value before operation
//
uint64_t cep_atomic_op(uint64_t *ptr, uint64_t val,int OP) {
  switch (OP) {
  case ATOMIC_OP_EXCH    : return __atomic_exchange_n(ptr,val,__ATOMIC_SEQ_CST); break;
  case ATOMIC_OP_ADD     : return __atomic_fetch_add(ptr,val,__ATOMIC_SEQ_CST); break;
  case ATOMIC_OP_AND     : return __atomic_fetch_and(ptr,val,__ATOMIC_SEQ_CST); break;
  case ATOMIC_OP_OR      : return __atomic_fetch_or(ptr,val,__ATOMIC_SEQ_CST); break;
  case ATOMIC_OP_SUB     : return __atomic_fetch_sub(ptr,val,__ATOMIC_SEQ_CST); break;
  case ATOMIC_OP_XOR     : return __atomic_fetch_xor(ptr,val,__ATOMIC_SEQ_CST); break;
  case ATOMIC_OP_LOAD    : return __atomic_load_n(ptr,__ATOMIC_SEQ_CST); break;
  case ATOMIC_OP_STORE   : __atomic_store_n(ptr,val,__ATOMIC_SEQ_CST); return 0; break;
  }
  return -1;
}
//
// Lock via atomic exchange (load-reserved/store-conditional ONLY on cached region, see Sifive Manual on Atominc Operation)
//
int cep_exchAtomicTest(int coreId, uint64_t *ptr, uint64_t mySig, uint64_t partSig, uint64_t incVal, int loop) {
  //
  //
  //
  int errCnt = 0;
  int timeOut = 2000;
  int i=1;
  uint64_t myCurSig = mySig;
  uint32_t regAdr = reg_base_addr + cep_scratch0_reg + (coreId * 16);
  //
  while (i <= loop) {
    // strong compare
    //    if (__atomic_compare_exchange_n(ptr,&myCurSig,partSig,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)) { // next
    if (__atomic_compare_exchange_n(ptr,&myCurSig,partSig,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQ_REL)) { // next    
      DUT_WRITE32_64(regAdr,mySig);
      DUT_WRITE32_64(regAdr+8,partSig);
      timeOut = 2000;
      mySig   += incVal;
      partSig += incVal;
      i++;
      //
    } else {
      DUT_WRITE32_64(regAdr,myCurSig); // what I got when false!!!
      DUT_WRITE32_64(regAdr+8,(i << 16) | timeOut);
      timeOut--;
      USEC_SLEEP(10);
      if (timeOut <= 0) {
	errCnt = i;
	return errCnt;
      }
    }
    myCurSig = mySig; // reset since myCurSig got exchange!!
  }
  return errCnt;
}

//
// Simple Atomic test (all ops)
//
int cep_runAtomicTest(uint64_t *ptr, uint64_t expVal) {
  int errCnt = 0;
  uint64_t actVal, oriVal=expVal;

  //
  // load  the location with expVal
  //
  __atomic_store_n(ptr,expVal,__ATOMIC_SEQ_CST);
  //
  // readback and check
  //
  actVal = __atomic_load_n(ptr,__ATOMIC_SEQ_CST);
  if (actVal != expVal) {
    errCnt = 1 << 0;
    return errCnt;
  }
  //
  // add 1's complement to it to make it all 0xFF...FF
  //
  actVal = __atomic_fetch_add(ptr,~expVal,__ATOMIC_SEQ_CST);
  if (actVal != expVal) {
    errCnt = 1 << 1;
    return errCnt;
  }
  expVal = (uint64_t)-1;
  //
  // Add 1 to get zero
  //
  actVal = __atomic_fetch_add(ptr,1,__ATOMIC_SEQ_CST);
  if (actVal != expVal) {
    errCnt = 1 << 1;
    return errCnt;
  }
  expVal = 0;
  //
  // walk a 1 via OR to make all 0xFF 
  //
  for (int i=0;i<64;i++) {
    actVal = __atomic_fetch_or(ptr,((uint64_t)1 << i),__ATOMIC_SEQ_CST);
    if (actVal != expVal) {
      errCnt = (i << 16) | (1 << 2);
      return errCnt;
    }
    expVal |= ((uint64_t)1 << i);
  }
  //
  // Subract to get 1's complement
  //
  actVal = __atomic_fetch_sub(ptr,oriVal,__ATOMIC_SEQ_CST);
  if (actVal != expVal) {
    errCnt = 1 << 3;
    return errCnt;
  }
  expVal = ~oriVal;
  //
  // XOR to get all 0xFF...FF
  //
  actVal = __atomic_fetch_xor(ptr,~expVal,__ATOMIC_SEQ_CST);
  if (actVal != expVal) {
    errCnt = 1 << 4;
    return errCnt;
  }
  expVal = (uint64_t)-1;
  //
  // walk a 0 via AND to make all 0
  //
  for (int i=0;i<64;i++) {
    actVal = __atomic_fetch_and(ptr,~((uint64_t)1 << i),__ATOMIC_SEQ_CST);
    if (actVal != expVal) {
      errCnt = (i << 16) | (1 << 5);
      return errCnt;
    }
    expVal &= ~((uint64_t)1 << i);
  }
  //
  // Exchange
  //
  actVal = __atomic_exchange_n(ptr,oriVal,__ATOMIC_SEQ_CST);
  if (actVal != expVal) {
    errCnt = (1 << 6);
    return errCnt;
  }
  expVal = oriVal;
  //
  // Final load
  //
  actVal = __atomic_load_n(ptr,__ATOMIC_SEQ_CST);
  if (actVal != expVal) {
    errCnt = 1 << 7;
    return errCnt;
  }
  //
  return errCnt;
}
    
//
// KEEP THIS ONE UNDER 64bytes (cache block) and do not change unless ...
//
volatile char get_selfModCodeValue(void) {
  return (volatile char)0x5A; // this value will be modified to test I-cache coherency
}
//
// MOdify the byte according to the comment at top of file
//    
static void SMC_change(uint64_t *ptr, char newB, int verbose) {
  // the fifth byte
  uint64_t oldWord, newWord,adr;
#ifdef LOCAL_MACHINE
  // bit 47:40 if oldWord0
  adr = (uint64_t)ptr[0];      
  oldWord = ptr[0]; 
  newWord = ((oldWord & ~((uint64_t)0xFF << 40)) |
	     ((uint64_t)(newB & 0xFF) << 40));
  ptr[0] = newWord;
#else
  // under BARE mode it is xxxx05a00513 (bit 27:20) of oldWord0
#ifdef BARE_MODE
  oldWord = ptr[0];
  adr = (uint64_t)ptr[0];
  newWord = ((oldWord & ~((uint64_t)0xFF << 20)) |
	    ((uint64_t)(newB & 0xFF) << 20));
  ptr[0] = newWord;  
  asm volatile ("fence w,r"); // tell other cores to flush
  asm volatile ("fence.i"); // tell myself to do the same  
#else
  // 05a0xxxx  bit 11:4 of oldWord1 in LINUX_MODE
  oldWord = ptr[1];
  adr = (uint64_t)ptr[1];
  newWord = ((oldWord & ~((uint64_t)0xFF << 4)) |
	     ((uint64_t)(newB & 0xFF) << 4));
  ptr[1] = newWord;
  asm volatile ("fence w,r"); // tell other cores to flush
  asm volatile ("fence.i"); // tell myself to do the same  
#endif
  //  
#endif
  if (verbose) {
    LOGI("@0x%016lx newByte=0x%02x old=0x%016lx new=0x%016lx\n",adr,newB,oldWord,newWord);
  }
}

int _cep_mprotect_done = 0;
static int cep_allow_SMC(int verbose) {
  int errCnt = 0;
  uint64_t selfModCodeAdr = (uint64_t)&get_selfModCodeValue;
  if (!_cep_mprotect_done) {
    //
    // BEFORE modify
    //
    if (verbose) {
      LOGI("selfModCode Adress = 0x%016lx Data=0x%016lx_0x%016lx ValueReturn=0x%02x\n",
	   selfModCodeAdr,((uint64_t *)selfModCodeAdr)[0], ((uint64_t *)selfModCodeAdr)[1], get_selfModCodeValue());
    }
    // change my protection 4Kbytes page
    #ifndef BARE_MODE
    errCnt += mprotect((void *)(selfModCodeAdr & ~(uint64_t)0xFFF), 0x1000, PROT_WRITE | PROT_READ | PROT_EXEC);
    #endif
    if (!errCnt) { _cep_mprotect_done = 1; }
  }
  return errCnt;
}

int set_selfModCodeValue(char newByte, int verbose) {
  int errCnt = cep_allow_SMC(verbose); // make sure it is set
  uint64_t selfModCodeAdr = (uint64_t)&get_selfModCodeValue;
  if (!errCnt) {
    //
    SMC_change((uint64_t *)selfModCodeAdr, newByte, verbose);
  }
  return errCnt;
}

//
// ********************
// self-modifying code calling a function from a known address
// to flush entire I-cache for a single core 
// ********************
 int do_DIcache_SMC(uint64_t *smc_base, int blockCnt, int startVal, int verbose) {
   int errCnt = 0;
   // 32Kbytes = size of Icache
   int actVal,expVal = startVal;
   //
   uint64_t dyn_smc_ptr= (uint64_t)smc_base;         
   //
   uint64_t selfModCodeAdr = (uint64_t)&get_selfModCodeValue;
   //
   // replicate the program blockCnt times and each return different value
   for (int i=0;i<blockCnt;i++) {
     memcpy((void *)(dyn_smc_ptr + (i*cep_cache_size)),(void *)selfModCodeAdr,64);
     // change the byte 0x55 to i
     SMC_change((uint64_t *)(dyn_smc_ptr+ (i*cep_cache_size)), (char)(expVal+i), verbose);
   }
   //
   // call each and check
   //
   for (int i=0;i<blockCnt;i++) {
     if (errCnt) { break; }
     if (verbose) {
       LOGI("smc_base=%016lx Dyn=0x%016lx Data=0x%016lx_0x%016lx\n",
	    (uint64_t)smc_base,dyn_smc_ptr,((uint64_t *)dyn_smc_ptr)[0],((uint64_t *)dyn_smc_ptr)[1]);
     }
     actVal = ((char (*)(void *))dyn_smc_ptr)(0);
     if ((actVal ^ expVal) & 0xff) { // error
       LOGE("i=%d Exp=0x%02x Act=0x%02x\n",i,expVal & 0xFF,actVal & 0xFF);
       errCnt++;
     } else if (verbose) {
       LOGI("i=%d Exp=0x%02x Act=0x%02x\n",i,expVal & 0xFF,actVal & 0xFF);       
     }
     dyn_smc_ptr += cep_cache_size;
     expVal++;
   }
   //
   return errCnt;             
 }
 
  //
#endif
