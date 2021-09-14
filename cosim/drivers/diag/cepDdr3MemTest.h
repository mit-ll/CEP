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

#ifndef cepDdr3MemTest_H
#define cepDdr3MemTest_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

//
// The test itself
//
int cepDdr3MemTest_runTest(int cpuId, uint32_t mem_base, int adrWidth, int seed, int full, int verbose);

#endif
