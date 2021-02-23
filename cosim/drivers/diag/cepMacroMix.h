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

#ifndef cepMacroMix_H
#define cepMacroMix_H

#ifdef __cplusplus
extern "C" {
#endif
  int cepMacroMix_runTest(int cpuId, int mask, int seed, int verbose);
  //  int cepMacroMix_runTest2(int cpuId, int mask, int seed, int verbose);  

  #ifdef __cplusplus
}
#endif

#endif
