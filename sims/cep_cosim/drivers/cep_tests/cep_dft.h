//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cep_dft/idft.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    dft/idft test for CEP
// Notes:          
//************************************************************************
#ifndef cep_dft_H
#define cep_dft_H
// ---------------------------
#include "stdint.h"
#include "cep_crypto.h"

//
// Fixed-point Format: 32.8 (32-bits)
//
typedef int16_t fixp16;

#define MAX_DFT_SAMPLES  64
//
// CEP's DFT 
//
class cep_dft : public cep_crypto { 
  // public stuffs
public: //
  // constructors
  //
  cep_dft(int coreIndex, int seed, int verbose);  
  ~cep_dft(); 

  int RunDftTest(int maxLoop);

protected:
  void adjust_float(double *rl, double *img, int length);
  void adjust_fixp(fixp16 *rl, fixp16 *img, int length);
  void do_dft(const double *rIn, const double *iIn, double *rOut, double *iOut, int len);

  void dft_setX(double *rbuf, double *ibuf, int len) ;
  void dft_getY(double *rbuf, double *ibuf, int len) ;
  void dft_Start(void) ;
  int dft_waitTilDone(int maxTO);
  int dft_CheckSamples(int lpCnt, int startIdx,int samCnt) ;

  double mRin[MAX_DFT_SAMPLES];
  double mIin[MAX_DFT_SAMPLES];
  double mRexp[MAX_DFT_SAMPLES];
  double mIexp[MAX_DFT_SAMPLES];
  double mRact[MAX_DFT_SAMPLES];
  double mIact[MAX_DFT_SAMPLES];    
  
  // tolerance in percentage
  int mRTolerance;
  int mITolerance;  
};

//
#endif
