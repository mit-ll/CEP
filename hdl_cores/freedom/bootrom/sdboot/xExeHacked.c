//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

// See LICENSE for license details.
#include <stdint.h>

#include <platform.h>

#include "common.h"


//
//
//
//#include "cep_adrMap.h"

//
// This will force the use of stack!!!
//
#define scratchpad_base_addr 0x64800000
#define scratchpad_exe_base  0x64808000
#define scratchpad_base_size 0x00010000
#define scratchpad_know_pattern 0x600dbabe12345678ULL
#define reg_base_addr        0x700F0000
#define cep_scratch0_reg     0xFE00
#define cep_cache_size       64

void* local_memcpy(void* dest, const void* src, int len)
{
  if ((((uintptr_t)dest | (uintptr_t)src | len) & (sizeof(uintptr_t)-1)) == 0) {
    const uintptr_t* s = src;
    uintptr_t *d = dest;
    while (d < (uintptr_t*)(dest + len))
      *d++ = *s++;
  } else {
    const char* s = src;
    char *d = dest;
    while (d < (char*)(dest + len))
      *d++ = *s++;
  }
  return dest;
}

int xExeTestAPI(uint64_t offset, uint64_t in) { 
  int errCnt = 0;
  uint64_t wrDat = in;
  uint64_t rdDat ;  
  _REG64(scratchpad_base_addr,offset) =  wrDat;
  rdDat = _REG64(scratchpad_base_addr, offset);
  if (wrDat != rdDat) {
    errCnt++;
  }
  return errCnt;
}
//
// This block of code is to be copied over to scratch pad and execute from there to verify that SRAM is a RWXC memory
// Assume limit to 2 cache blocks!!!
//

int doTheWriteFromSratchPad(void) {
  uint64_t offS = reg_base_addr + cep_scratch0_reg;
  for (int i=0;i<8;i++) {
    *(volatile uint64_t *)(offS) = offS;
    offS += 8; // next reg
  }
  return 0xAA55;
}

int doTheCheck(void) {
  int errCnt  = 0;
  uint64_t offS = reg_base_addr + cep_scratch0_reg;
  uint64_t rdDat;
  for (int i=0;i<8;i++) {
    rdDat = *(volatile uint64_t *)(offS);
    if (rdDat != offS) { errCnt++;}
    offS += 8; // next reg
  }
  return errCnt;
}

//
// Some code to verify that we can move the stack point to scratchpad and execute code out of bootrom without main memory
// (act as place holder for real APIs)
//
int xExeInit(void)
{
  uint64_t wrDat = scratchpad_know_pattern;
  uint64_t rdDat ;
  int errCnt=0;
  uint64_t offset_array[8]; // local space via stack  
  //
  // EXecute out of scratch pad!!!
  //
#ifdef _TEST_EXE
  //
  uint64_t offS = (uint64_t)doTheWriteFromSratchPad & 0x3F; // same offset within the block
  uint64_t dyn_smc_ptr= (uint64_t)(scratchpad_exe_base + offS);
  local_memcpy((void *)dyn_smc_ptr,(void *)doTheWriteFromSratchPad,cep_cache_size*2);
  // flush
  asm volatile ("fence w,r"); // tell other cores to flush
  asm volatile ("fence.i"); // tell myself to do the same  
  //
  // execute out of scratchpad
  //
  int val = ((int (*)(void *))dyn_smc_ptr)(0);
  if (val != 0xAA55) { errCnt++; return 1;}
  //
  // read back and verify from bootrom
  //
  errCnt += doTheCheck();
  if (errCnt) { return 1; }
  //
#endif
  //
  //  offset, use array to force push on stack
  //
  for (int i=0;i<8;i++) {
    offset_array[i] = i * 0x100;
  }
  //
  //
  // Write loop
  //  
  wrDat = scratchpad_know_pattern;
  for (int i=0;i<8;i++) {
    errCnt += xExeTestAPI(offset_array[i],wrDat); // call some API to use stack!!
    wrDat++;
  }
  if (errCnt ) { return 1; }
  //
  // Read loop
  //
  wrDat = scratchpad_know_pattern;
  for (int i=0;i<8;i++) {
    rdDat = _REG64(scratchpad_base_addr, offset_array[i]);
    if (wrDat != rdDat) {
      errCnt++;
      break;
    }
    wrDat++;
  }
  if (errCnt ) { return 1; }
  //
  //
  // play with some internal registers to act as AXI read/write
  //
  wrDat = ~wrDat;
  _REG64(reg_base_addr, cep_scratch0_reg) = wrDat;
  rdDat = _REG64(reg_base_addr, cep_scratch0_reg);
  if (wrDat != rdDat) {
    return 1;
  }
  //
  //
  //
  __asm__ __volatile__ ("fence.i" : : : "memory");
  return errCnt;
}

