//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cep_fir.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    fir test for CEP
// Notes:          
//************************************************************************
#if defined(BARE_MODE)

extern void *memcpy(void* dest, const void* src, int len);

#else
#include <math.h>
#include "simPio.h"

#endif

#include <string.h>
#include "cep_fir.h"
#include "simdiag_global.h"
#include "portable_io.h"
#include "CEP.h"
#include "cepregression.h"
#include "random48.h"
//
//
//
//
#include <math.h>
#define FIXED_POINT_FRACTIONAL_BITS 8


fixp32_8 cep_fir::float_to_fixed(double input)
{
  //    return (fixp32_8)(round(input * (1 << FIXED_POINT_FRACTIONAL_BITS)));
  return (fixp32_8)(input * (1 << FIXED_POINT_FRACTIONAL_BITS));  
}

double cep_fir::fixed_to_float(fixp32_8 input)
{
    return ((double)input / (double)(1 << FIXED_POINT_FRACTIONAL_BITS));
}

//
//
//
cep_fir::cep_fir(int seed, int verbose)
{
  init();  
  SetSeed(28147424);
  SetVerbose(verbose);
  mTolerance = 1; // %
  //
  // These to match HW RTL  
  //
  double fir_a_k[] = {1.000000e+000	,
		      1.0,
		      0.0,
		      0.0,
		      0.0,
		      0.0,
		      0.0,
		      0.0,
		      0.0,
		      0.0,
		      0.0,
		      0.0};

  
  double fir_b_k[] = {1.618571e-003,
		      1.780428e-002,
		      8.902138e-002,
		      2.670642e-001,
		      5.341283e-001,
		      7.477796e-001,
		      7.477796e-001,
		      5.341283e-001,
		      2.670642e-001,
		      8.902138e-002,
		      1.780428e-002,
		      1.618571e-003};
  //
  // to match HW fixed points since HW convert double to fixed point 32.8 so we need to do the same here
  //
  memcpy(mBconst, fir_b_k, sizeof(fir_b_k));
  memcpy(mAconst, fir_a_k, sizeof(fir_a_k));  
}
//
cep_fir::~cep_fir()  {
}

//
// convert float->fixed->float due to HW precision lost
//
void cep_fir::adjust_float(double *inout, int length)
{
  if (GetVerbose()) {
    LOGI("%s: len=%d\n",__FUNCTION__,length);
  }
  for (int i=0;i<length;i++) {
    fixp32_8 newFix = float_to_fixed(inout[i]);
    double newFloat = fixed_to_float(newFix);
    if (GetVerbose()) {
      LOGI("i=%d old=%.10f -> 0x%08x -> %.10f\n",i,inout[i],newFix,newFloat);
    }
    inout[i] = newFloat;    
  }
}

//
// convert fixed->float->fixed due to HW precision lost
//
void cep_fir::adjust_fixp(fixp32_8 *inout, int length)
{
  if (GetVerbose()) {
    LOGI("%s: len=%d\n",__FUNCTION__,length);
  }
  for (int i=0;i<length;i++) {
    double newFloat = fixed_to_float(inout[i]);        
    fixp32_8 newFix = float_to_fixed(newFloat);
    if (GetVerbose()) {
      LOGI("i=%d old=0x%08x -> %.10f -> 0x%08x\n",i,inout[i],newFloat,newFix);
    }
    inout[i] = newFix;
  }
}

void cep_fir::do_filter(const double *b, const double *a, int filterLength, const double *in, double *out, int length) {
    const double a0 = a[0];
    const double *a_end = &a[filterLength-1];
    const double *out_start = out;
    a++;
    out--;
    int m;
    for (m = 0; m < length; m++) {
        const double *b_macc = b;
        const double *in_macc = in;
        const double *a_macc = a;
        const double *out_macc = out;
        double b_acc = (*in_macc--) * (*b_macc++);
        double a_acc = 0;
        while (a_macc <= a_end && out_macc >= out_start) {
            b_acc += (*in_macc--) * (*b_macc++);
            a_acc += (*out_macc--) * (*a_macc++);
        }
        *++out = (b_acc - a_acc) / a0;
        in++;
    }
}

void cep_fir::LoadInSamples(int samCnt) {
  uint64_t word;
  for(int i = 0; i < samCnt; i++) { //  8-bytes/word
    word = (uint64_t)float_to_fixed(mInput[i]);
    cep_writeNcapture(FIR_BASE_K, FIR_IN_DATA, word); //Write data
    cep_writeNcapture(FIR_BASE_K, FIR_IN_ADDR, i);     //Write addr
    cep_writeNcapture(FIR_BASE_K, FIR_IN_WRITE, 0x02);  //Load data
    cep_writeNcapture(FIR_BASE_K, FIR_IN_WRITE, 0x00);  //Stop
  }
}

void cep_fir::Start(void) {
  cep_writeNcapture(FIR_BASE_K, FIR_START, 0x1);
  cep_writeNcapture(FIR_BASE_K, FIR_START, 0x0);
}

void cep_fir::Reset(void) {
  cep_writeNcapture(FIR_BASE_K, FIR_RESET, 0x1);
  cep_writeNcapture(FIR_BASE_K, FIR_RESET, 0x0);
}

int cep_fir::waitTilDone(int maxTO) {
#if 1
   if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }    
   return cep_readNspin(FIR_BASE_K, FIR_DONE, 0x4, maxTO);  
#else
  while (maxTO > 0) {
    if (cep_readNcapture(FIR_BASE_K, FIR_DONE) & 0x4) break;
    maxTO--;
  };
  return (maxTO <= 0) ? 1 : 0;
#endif
}

void cep_fir::ReadOutSamples(int samCnt) {
  uint64_t word;
  for(int i = 0; i < samCnt; i++) { //  8-bytes/word
    cep_writeNcapture(FIR_BASE_K, FIR_OUT_ADDR, i);    //Write addr
    word=cep_readNcapture(FIR_BASE_K, FIR_OUT_DATA); //Read data
    mAct[i] = fixed_to_float(word);
  }
}

int cep_fir::CheckSamples(int samCnt) {
  double epsilon,diff;
  for (int i=0;i<samCnt;i++) {
    epsilon = fabs(mExp[i])*(mTolerance)/100;
    // between 0.01 and 0.05 min/max
    if (epsilon < 0.01) { epsilon = 0.02; }
    // adjust epsilon if the expect != 0x8....... = negative -8million
    // only happen in FIR
    //
    //else if ((float_to_fixed(mExp[i]) >> 28) != 0x8) { epsilon = 0.05; }
    else if (epsilon > 0.05) { epsilon = 0.05; }
    //
    diff = fabs(fabs(mAct[i]) - fabs(mExp[i]));
    if (diff > epsilon) {
      LOGE("%s: fir i=%d Exp=%.5f Act=%.5f : diff=%.5f > Epsilon=%.5f\n",__FUNCTION__,i,mExp[i],mAct[i],diff, epsilon);
      mErrCnt++;
    } else if (GetVerbose()) {
      LOGI("%s: fir i=%d Exp=%.5f Act=%.5f : diff=%.5f > Epsilon=%.5f\n",__FUNCTION__,i,mExp[i],mAct[i],diff, epsilon);      
    }
  }
  return mErrCnt;
}

int cep_fir::RunFirTest(int maxLoop) {
  if (GetVerbose()) {
    LOGI("%s: seed=%d\n",__FUNCTION__,GetSeed());
  }
  //
  // adjust b_k
  //
  #if 1
  adjust_float(mAconst,MAX_CONST);  
  adjust_float(mBconst,MAX_CONST);
  #endif
  //
    //
  //
  // Need to load the inData FIFO with know data and then issue a reset because
  // the outData just follow the in and if input are unknown, restuls are *off*
  //
  for (int j=0; j < MAX_SAMPLES; j++) {
    mInput[j] = 0;
  }  
  //LoadInSamples(MAX_SAMPLES);
  //
  Random48_srand48(GetSeed());
  for (int i=0;i<maxLoop;i++) {
    if (GetVerbose()) {
      LOGI("%s: Loop %d\n",__FUNCTION__,i);
    }
    //Reset();
    // build 32 samples
    for (int j=0; j < MAX_SAMPLES; j++) {
      mInput[j] = fixed_to_float((Random48_rand() + (int32_t)(j+1)*4) & 0x3FFFFFFF);
    }
    // adjust ??
    adjust_float(mInput, MAX_SAMPLES);
    //
    // compute exptected
    //
    do_filter(mBconst, mAconst, MAX_CONST, mInput, mExp, MAX_SAMPLES);
    adjust_float(mExp, MAX_SAMPLES);    
#if 1
    LoadInSamples(MAX_SAMPLES);
    Start();
    mErrCnt += waitTilDone(10);
    if (!mErrCnt) {
      ReadOutSamples(MAX_SAMPLES);
      adjust_float(mAct, MAX_SAMPLES);      
      mErrCnt += CheckSamples(MAX_SAMPLES);
    }
#endif
    //
    MarkSingle(i);    
    if (mErrCnt) break;
  }
  return mErrCnt;
}

