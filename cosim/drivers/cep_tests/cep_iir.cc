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

#if defined(BARE_MODE)

extern void *memcpy(void* dest, const void* src, int len);

#else
#include <math.h>
#include "simPio.h"

#endif

#include <string.h>
#include "cep_iir.h"
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


fixp32_8 cep_iir::float_to_fixed(double input)
{
  //    return (fixp32_8)(round(input * (1 << FIXED_POINT_FRACTIONAL_BITS)));
  return (fixp32_8)(input * (1 << FIXED_POINT_FRACTIONAL_BITS));  
}

double cep_iir::fixed_to_float(fixp32_8 input)
{
    return ((double)input / (double)(1 << FIXED_POINT_FRACTIONAL_BITS));
}

//
//
//
cep_iir::cep_iir(int seed, int verbose)
{
  init();  
  SetSeed(15145975);
  SetVerbose(verbose);
  mTolerance = 1; // %
  //
  // These to match HW RTL  
  //
  double iir_a_k[] = {1.000000e+000	,
		      8.274631e-016	,
		      1.479294e+000	,
		      1.089388e-015	,
		      7.013122e-001	,
		      4.861197e-016	,
		      1.262132e-001	,
		      9.512539e-017	,
		      7.898376e-003	,
		      -4.496047e-018	,
		      1.152699e-004	,
		      -6.398766e-021	};
  
  double iir_b_k[] = {1.618571e-003,
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
  memcpy(mBconst, iir_b_k, sizeof(iir_b_k));
  memcpy(mAconst, iir_a_k, sizeof(iir_a_k));  
}
//
cep_iir::~cep_iir()  {
}

//
// convert float->fixed->float due to HW precision lost
//
void cep_iir::adjust_float(double *inout, int length)
{
  if (GetVerbose(2)) {
    LOGI("%s: len=%d\n",__FUNCTION__,length);
  }
  for (int i=0;i<length;i++) {
    fixp32_8 newFix = float_to_fixed(inout[i]);
    double newFloat = fixed_to_float(newFix);
    if (GetVerbose(2)) {
      LOGI("i=%d old=%.10f -> 0x%08x -> %.10f\n",i,inout[i],newFix,newFloat);
    }
    inout[i] = newFloat;    
  }
}

//
// convert fixed->float->fixed due to HW precision lost
//
void cep_iir::adjust_fixp(fixp32_8 *inout, int length)
{
  if (GetVerbose(2)) {
    LOGI("%s: len=%d\n",__FUNCTION__,length);
  }
  for (int i=0;i<length;i++) {
    double newFloat = fixed_to_float(inout[i]);        
    fixp32_8 newFix = float_to_fixed(newFloat);
    if (GetVerbose(2)) {
      LOGI("i=%d old=0x%08x -> %.10f -> 0x%08x\n",i,inout[i],newFloat,newFix);
    }
    inout[i] = newFix;
  }
}

void cep_iir::do_filter(const double *b, const double *a, int filterLength, const double *in, double *out, int length) {
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

void cep_iir::LoadInSamples(int samCnt) {
  uint64_t word;
  for(int i = 0; i < samCnt; i++) { //  8-bytes/word
    word = (uint64_t)float_to_fixed(mInput[i]);
    // duplicate 32 to 64 for coverage
    word |= (word << 32) | (word & 0xFFFFFFFFLL);
    //
    cep_writeNcapture(IIR_BASE_K, IIR_IN_DATA, word); //Write data
    cep_writeNcapture(IIR_BASE_K, IIR_IN_ADDR, i);     //Write addr
    cep_writeNcapture(IIR_BASE_K, IIR_IN_WRITE, 0x02);  //Load data
    cep_writeNcapture(IIR_BASE_K, IIR_IN_WRITE, 0x00);  //Stop
  }
}

void cep_iir::Start(void) {
  cep_writeNcapture(IIR_BASE_K, IIR_START, 0x1);
  cep_writeNcapture(IIR_BASE_K, IIR_START, 0x0);
}

void cep_iir::Reset(int assert) {
  cep_writeNcapture(IIR_BASE_K, IIR_RESET, assert);
}

void cep_iir::Reset(void) {
  cep_writeNcapture(IIR_BASE_K, IIR_RESET, 0x1);
  cep_writeNcapture(IIR_BASE_K, IIR_RESET, 0x0);
}

int cep_iir::waitTilDone(int maxTO) {
#if 1
   if (GetVerbose(2)) {  LOGI("%s\n",__FUNCTION__); }    
   return cep_readNspin(IIR_BASE_K, IIR_DONE, 0x4, maxTO);
#else
  while (maxTO > 0) {
    if (cep_readNcapture(IIR_BASE_K, IIR_DONE) & 0x4) break;
    maxTO--;
  };
  return (maxTO <= 0) ? 1 : 0;
#endif
}

void cep_iir::ReadOutSamples(int samCnt) {
  uint64_t word;
  for(int i = 0; i < samCnt; i++) { //  8-bytes/word
    cep_writeNcapture(IIR_BASE_K, IIR_OUT_ADDR, i);    //Write addr
    word=cep_readNcapture(IIR_BASE_K, IIR_OUT_DATA); //Read data
    mAct[i] = fixed_to_float(word);
  }
}

int cep_iir::CheckSamples(int samCnt) {
  double epsilon,diff;
  for (int i=0;i<samCnt;i++) {
    epsilon = fabs(mExp[i])*(mTolerance)/100;
    // between 0.01 and 0.05 min/max
    if (epsilon < 0.01) { epsilon = 0.025; }
    // adjust epsilon if the expect != 0x8....... = negative -8million
    // only happen in FIR
    //
    //else if ((float_to_fixed(mExp[i]) >> 28) != 0x8) { epsilon = 0.05; }
    else if (epsilon > 0.1) { epsilon = 0.1; }
    //
    diff = fabs(fabs(mAct[i]) - fabs(mExp[i]));
    if (diff > epsilon) {
      if (!GetExpErr()) {
	LOGE("%s: iir i=%d Exp=%.5f Act=%.5f : diff=%.5f > Epsilon=%.5f\n",__FUNCTION__,i,mExp[i],mAct[i],diff, epsilon);
      }
      mErrCnt++;
    } else if (GetVerbose(2)) {
      LOGI("%s: i=%d Exp=%.5f Act=%.5f : diff=%.5f > Epsilon=%.5f\n",__FUNCTION__,i,mExp[i],mAct[i],diff, epsilon);      
    }
  }
  return mErrCnt;
}

int cep_iir::RunIirTest(int maxLoop) {
  //
  // adjust b_k
  //
  #if 1
  adjust_float(mAconst,MAX_CONST);  
  adjust_float(mBconst,MAX_CONST);
  #endif
  //
  //
  // Need to load the inData FIFO with know data and then issue a reset because
  // the outData just follow the in and if input are unknown, restuls are *off*
  //
  for (int j=0; j < MAX_SAMPLES; j++) {
    mInput[j] = 0;
  }  
  LoadInSamples(MAX_SAMPLES);
  //
  //
  //
  Random48_srand48(GetSeed());
  for (int i=0;i<maxLoop;i++) {
    if (GetVerbose()) {
      LOGI("%s: Loop %d\n",__FUNCTION__,i);
    }
    // must do this reset to clear the internal state else output will be mismatch
    Reset();
    //
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

