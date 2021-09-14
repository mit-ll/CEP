//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cep_idft.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    dft/idft test for CEP
// Notes:          
//************************************************************************
#ifndef cep_idft_H
#define cep_idft_H
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
class cep_idft : public cep_crypto { 
  // public stuffs
public: //
  // constructors
  //
  cep_idft(int coreIndex, int seed, int verbose);  
  ~cep_idft(); 

  int RunIdftTest(int maxLoop);

protected:
  void adjust_float(double *rl, double *img, int length);
  void adjust_fixp(fixp16 *rl, fixp16 *img, int length);
  void do_idft(const double *rIn, const double *iIn, double *rOut, double *iOut, int len);
  //
  //
  void idft_setX(double *rbuf, double *ibuf, int len) ;
  void idft_getY(double *rbuf, double *ibuf, int len) ;
  void idft_Start(void) ;
  int idft_waitTilDone(int maxTO);
  int idft_CheckSamples(int lpCnt, int startIdx,int samCnt) ;
  //
  double mRin[MAX_DFT_SAMPLES];
  double mIin[MAX_DFT_SAMPLES];
  //
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
