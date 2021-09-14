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

#ifndef cepLockfreeAtomic_H
#define cepLockfreeAtomic_H
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

//
// The test itself
//
int cepLockfreeAtomic_runTest(int cpuId, uint64_t mem_base, uint64_t reg_base, int seed, int verbose);

#endif
