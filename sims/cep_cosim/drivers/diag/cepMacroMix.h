//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
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
  int cepMacroMix_runTest(int cpuId, int cpuActiveMask, int coreMask, int seed, int verbose);
  int cepMacroMix_runBadKeysTest(int cpuId, int cpuActiveMask, int coreMask, int seed, int verbose);

  #ifdef __cplusplus
}
#endif

#endif
