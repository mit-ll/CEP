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
#define scratchpad_base_size 0x00010000
#define scratchpad_know_pattern 0x600dbabe12345678ULL
#define reg_base_addr        0x700F0000
#define cep_scratch0_reg     0xFE00

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

