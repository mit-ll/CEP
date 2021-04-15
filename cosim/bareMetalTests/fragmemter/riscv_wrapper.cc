//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
//
// For bareMetal mode ONLY
//
#ifdef BARE_MODE

#include "cep_adrMap.h"
#include "cep_apis.h"
#include "portable_io.h"

#include "cepRegTest.h"
#include "CEP.h"

//#define printf(...) { return 0; }

// prevent C++ to mangle the name
extern "C" {

extern void* memcpy(void* dest, const void* src, size_t len);

void thread_entry(int cid, int nc);
}

#define flush_page(addr) asm volatile ("sfence.vma %0" : : "r" (addr) : "memory")

  //
  // Read from MaskRom instead
  //
  uint64_t eData[8] __attribute__((aligned(64))) = {
    0, // 0x12345678abcdef10LL,
    0, // 0x1112131415161718LL,
    0, // 0x191a1b1c1d1e1f20LL,
    0, // 0x2122232425262728LL,
    0, // 0x292a2b2c2d2e2f30LL,
    0, // 0x3132333435363738LL,
    0, // 0x3940414243444546LL,
    0}; // 0x600DBABEDEADBEEFLL } ;

  //
  // use memcpy 
  //
  uint64_t aData[8][8] __attribute__((aligned(64)));
  
//
// templating with different access types
//
template <typename T>
int runSingle(int coreId, uint64_t mask, uint32_t destAdr, int cpySize, int writeEn) 
{
  //
  int errCnt = 0,i;
  int memSize = cpySize; // sizeof(eData);
  uint64_t dat64;
  //
  // core 0 only for now
  //
  // 
  T *dptr, *sptr;
  //
  // load to Peripheral : 64-bits only
  //
  dptr = (T *)destAdr; // (reg_base_addr + cep_scratch0_reg);
  sptr = (T *)eData;
  //
  if (writeEn) {
    if (coreId == 0) {
      memcpy(dptr, sptr,memSize);
    } 
    else {
      // other just read last location of  until != 0
      do {
	//DUT_READ32_64(reg_base_addr + cep_scratch7_reg, dat64);
	DUT_READ32_64(destAdr+(7*8), dat64);
      } while (dat64 == 0);
    }
  }
  //
  // Copy back and check via appropriate 8/16/32/64-bit pointers
  //
  dptr = (T *)(aData[coreId]);
  sptr = (T *)destAdr; // (reg_base_addr + cep_scratch0_reg);
  memcpy(dptr, sptr,memSize);
  flush_page(dptr);
  //
  //
  for (i=0;i<8;i++) {
    if (((eData[i] ^ aData[coreId][i]) & mask) != 0) {
      //DUT_WRITE32_64(reg_base_addr + cep_core2_status,aData[coreId][i]);
      //DUT_WRITE32_64(reg_base_addr + cep_core3_status,eData[i]);
      errCnt++;
      break;
    } else {
      aData[coreId][i] = 0; // clear for next run
    }
  }
  return errCnt;
}

int doGroupTest(int coreId,int testId, uint64_t mask, uint32_t destAdr,int memSize, int wrEn)  
{
  int errCnt = 0;
  if (!errCnt) { 
    set_cur_status(CEP_NEXT_TEST + (testId << 8) + 8); 
    errCnt += runSingle<uint8_t>(coreId, mask, destAdr,memSize,wrEn);  
  }  
  if (!errCnt) { 
    set_cur_status(CEP_NEXT_TEST + (testId << 8) + 16); 
    errCnt += runSingle<uint16_t>(coreId,mask, destAdr,memSize,0);  
  } 
  if (!errCnt) { 
    set_cur_status(CEP_NEXT_TEST + (testId << 8) + 32); 
    errCnt += runSingle<uint32_t>(coreId,mask, destAdr,memSize,0);  
  } 
  if (!errCnt) { 
    set_cur_status(CEP_NEXT_TEST + (testId << 8) + 64); 
    errCnt += runSingle<uint64_t>(coreId,mask, destAdr,memSize,0);  
  }
    return errCnt;
  }

  
//
// Main Entrance!!!
//
void thread_entry(int cid, int nc)
{
  //
  int errCnt = 0;
  int coreId = read_csr(mhartid);
  uint32_t destAdr;
  int memSize;
  uint64_t mask, dat64;
  int testId;
  //
  // MaskRom (readOnly) and shouls be the first test!!!
  //
  destAdr = bootrom_base_addr;
  memSize = sizeof(eData);
  mask    = (uint64_t)-1;
  testId  = 3;
  // to emulate RO
  //eData[7] = 0;
  if (coreId == 0) {
    memcpy((uint64_t *)eData,(uint64_t *)destAdr,memSize);
    flush_page(eData);
  } else {
    // other just read scratch7 until != 0
    do { DUT_READ32_64(&eData[7],dat64); } while (dat64 == 0);
  }
  if (!errCnt) { errCnt += doGroupTest(coreId, testId, mask, destAdr, memSize,0); }
  //
  //
  // MD5 Registers 
  //
  destAdr = md5_base_addr + MD5_MSG_BASE;
  memSize = sizeof(eData);
  mask    = (uint64_t)-1;
  testId  = 2;
  if (!errCnt) { errCnt += doGroupTest(coreId, testId, mask, destAdr, memSize,1); }

  //
  // CEP Registers 
  //
  destAdr = reg_base_addr + cep_scratch0_reg;
  memSize = sizeof(eData);
  mask    = (uint64_t)-1;
  testId  = 1;
  if (!errCnt) { errCnt += doGroupTest(coreId, testId, mask, destAdr, memSize,1); }
  //
  //
  //

  //
  // Done
  //
  set_status(errCnt,errCnt);
  //
  // Stuck here forever...
  //
  exit(errCnt);
}

#endif
