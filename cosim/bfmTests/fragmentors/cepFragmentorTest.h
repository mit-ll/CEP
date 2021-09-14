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

#ifndef cepFragmentorTest_H
#define cepFragmentorTest_H
#include "stdint.h"
#include "stdio.h"

// This test is for BFM mode only because RISCV core can't do burst access
int cepFragmentorTest_atomicTest(int cpuId, uint32_t baseAdr, int mask, int checkEn, int seed, int verbose);
int cepFragmentorTest_baseTest(int cpuId, uint32_t baseAdr, int wrW, int rdW, uint64_t mask, int seed, int verbose);
int cepFragmentorTest_verifyMaskRom(int cpuId, uint32_t baseAdr, char *file, int wordCnt, int seed, int verbose);

#endif
