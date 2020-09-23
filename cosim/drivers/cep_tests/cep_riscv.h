//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef CEP_RISCV_H
#define CEP_RISCV_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
  /*
   * Function Prototypes (only when target is RISCV)
   */
  volatile char get_selfModCodeValue(void) ;
  int set_selfModCodeValue(char newByte, int verbose);
  int do_DIcache_SMC(uint64_t *smc_base, int blockCnt, int startVal, int verbose);
  
// end of extern
#ifdef __cplusplus
}
#endif


// ===============
#endif
