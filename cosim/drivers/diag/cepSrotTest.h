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

#ifndef cepSrotTest_H
#define cepSrotTest_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include "cep_crypto.h"

  //
  // The test itself
  //
<<<<<<< HEAD
uint8_t send_srot_message(int verbose, uint64_t message);
//
int cepSrotTest_maxKeyTest(int cpuId, int verbose);
=======
  uint8_t send_srot_message(int verbose, uint64_t message);
  int cepSrotTest_runTest(int cpuId, int seed, int verbose);
>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0

#endif
