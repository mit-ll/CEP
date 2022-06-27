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

#ifndef cepSrotTest_H
#define cepSrotTest_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include "cep_crypto.h"

  //
  // The test itself
  //
uint8_t send_srot_message(int verbose, uint64_t message);
//
int cepSrotTest_maxKeyTest(int cpuId, int verbose);

#endif
