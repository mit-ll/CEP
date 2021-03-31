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

#ifndef CEP_RISCV_H
#define CEP_RISCV_H

#include <stdint.h>

#define ATOMIC_OP_EXCH  1
#define ATOMIC_OP_ADD   2
#define ATOMIC_OP_AND   3
#define ATOMIC_OP_OR    4
#define ATOMIC_OP_SUB   5
#define ATOMIC_OP_XOR   6
#define ATOMIC_OP_LOAD  7
#define ATOMIC_OP_STORE 8


#ifdef __cplusplus
extern "C" {
#endif
  /*
   * Function Prototypes (only when target is RISCV)
   */
  volatile char get_selfModCodeValue(void) ;
  int set_selfModCodeValue(char newByte, int verbose);
  int do_DIcache_SMC(uint64_t *smc_base, int blockCnt, int startVal, int verbose);
  // Atomic stuffs
  uint64_t cep_atomic_op(uint64_t *ptr, uint64_t val, int OP) ;
  int cep_runAtomicTest(uint64_t *ptr, uint64_t expVal);
  int cep_exchAtomicTest(int coreId, uint64_t *ptr, uint64_t mySig, uint64_t partSig, uint64_t incVal, int loop);
  
// end of extern
#ifdef __cplusplus
}
#endif


// ===============
#endif
