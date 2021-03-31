//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cep_fir.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    fir test for CEP
// Notes:          
//************************************************************************
#ifndef cep_fir_H
#define cep_fir_H
// ---------------------------
#include "stdint.h"
#include "cep_crypto.h"

//
// Fixed-point Format: 32.8 (32-bits)
//
typedef int32_t fixp32_8;

#define MAX_SAMPLES  32
#define MAX_CONST    12
//
// CEP's FIR 
//
class cep_fir : public cep_crypto { 
  // public stuffs
public: //
  // constructors
  //
  cep_fir(int seed, int verbose);
  cep_fir() { }
  ~cep_fir(); 

  void Reset(void);  
  void Reset(int assert);  

  int RunFirTest(int maxLoop);

 protected:
  fixp32_8 float_to_fixed(double input);
  double fixed_to_float(fixp32_8 input);
  void adjust_float(double *inout, int length);
  void adjust_fixp(fixp32_8 *inout, int length);
  void do_filter(const double *b, const double *a, int filterLength, const double *in, double *out, int length);
  void LoadInSamples(int samCnt);
  void Start(void);
  int  waitTilDone(int maxTO);
  void ReadOutSamples(int samCnt);
  int CheckSamples(int samCnt);
  //
  double mInput[MAX_SAMPLES];
  double mExp[MAX_SAMPLES];
  double mAct[MAX_SAMPLES];  
  //
  double mAconst[MAX_CONST];
  double mBconst[MAX_CONST];
  // tolerance in percentage
  int mTolerance;
};

//
#endif
