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
#include "riscv_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif
  
#if 1
//int main(void)

unsigned long long d0=0x12345678LL;
unsigned long long d1=0x11223344LL;
unsigned long long d2=0x55667788LL;
unsigned long long d3=0x0;

void riscvTest_entry(void) 
{
  //

  d3 += d0;
  d3 += d1;
  d3 += d2;

  return;
}
#endif

#ifdef __cplusplus
}
#endif
  
