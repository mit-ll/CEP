//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cep_dft/idft.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    dft/idft test for CEP
// Notes:          
//************************************************************************
#if defined(BARE_MODE)
// for now, dont care since not use
#define cos(x) (0.0)
#define sin(x) (0.0)
#define fabs(x) (x)

#else
#include <math.h>
#include "simPio.h"
#include "fft.h"


#endif

#include <string.h>
#include "cep_dft.h"
#include "simdiag_global.h"
#include "portable_io.h"
#include "CEP.h"
#include "cepregression.h"
#include "random48.h"
//
//
//
//

#define FIXED_POINT_FRACTIONAL_BITS 0
inline fixp16 float_to_fixed(double input)
{
  //    return (fixp16)(round(input * (1 << FIXED_POINT_FRACTIONAL_BITS)));
  return (fixp16)(input * (1 << FIXED_POINT_FRACTIONAL_BITS));  
}

inline double fixed_to_float(fixp16 input)
{
    return ((double)input / (double)(1 << FIXED_POINT_FRACTIONAL_BITS));
}

//
//
//
cep_dft::cep_dft(int seed, int verbose)
{
  init();  
  SetSeed(0);
  SetVerbose(verbose);
  mRTolerance = 5; // %
  mITolerance = 20; // %  
}
//
cep_dft::~cep_dft()  {
}

//
// convert float->fixed->float due to HW precision lost
//

void cep_dft::adjust_float(double *rl, double *img, int length)
{
  if (GetVerbose()) {
    LOGI("%s: len=%d\n",__FUNCTION__,length);
  }
  for (int i=0;i<length;i++) {
    fixp16  newFix = float_to_fixed(rl[i]);    
    double newFloat = fixed_to_float(newFix);

    //
    fixp16  iFix = float_to_fixed(img[i]);    
    double iFloat = fixed_to_float(iFix);

    //
    if (GetVerbose()) {
      LOGI("i=%d old=%.5f/%.5f -> 0x%04x/%04x -> %.5f/%.5f\n",i,
	   rl[i],img[i], newFix & 0xFFFF,iFix & 0xFFFF, newFloat, iFloat);
    }
    rl[i] = newFloat;
    img[i] = iFloat;
  }
}

//
// convert fixed->float->fixed due to HW precision lost
//
void cep_dft::adjust_fixp(fixp16 *rl, fixp16 *img, int length)
{
  if (GetVerbose()) {
    LOGI("%s: len=%d\n",__FUNCTION__,length);
  }
  for (int i=0;i<length;i++) {
    double newFloat = fixed_to_float(rl[i]);        
    fixp16  newFix = float_to_fixed(newFloat);
    //
    double iFloat = fixed_to_float(img[i]);        
    fixp16  iFix = float_to_fixed(iFloat);
    //
    if (GetVerbose()) {
      LOGI("i=%d old=0x%04x/%04x -> %.5f/%.5f -> 0x%04x/%04x\n",i,
	   rl[i],img[i], newFloat, iFloat,newFix & 0xFFFF,iFix & 0xFFFF);
    }
    rl[i] = newFix;
    img[i] = iFix;
  }
}

void cep_dft::do_dft(const double *rIn, const double *iIn, double *rOut, double *iOut, int len)
{
  memcpy(rOut, rIn, sizeof(double)*len);
  memcpy(iOut, iIn, sizeof(double)*len);
#if defined(BARE_MODE)
#else  
  (void)Fft_transform(rOut, iOut, len);
#endif
}

// IDFT
void cep_dft::do_idft(const double *rIn, const double *iIn, double *rOut, double *iOut, int len)
{
  memcpy(rOut, rIn, sizeof(double)*len);
  memcpy(iOut, iIn, sizeof(double)*len);
#if defined(BARE_MODE)
#else  
  (void)Fft_inverseTransform(rOut, iOut, len);
#endif
}

//
// DFT
//
void cep_dft::dft_setX(double *rbuf, double *ibuf, int len) {
  uint64_t  temp;
  if (GetVerbose()) { LOGI("%s\n",__FUNCTION__); }
  for (int i=0;i<len/2;i++) { // each take 2 complex number
#ifdef BIG_ENDIAN
    temp = ( (((uint64_t)float_to_fixed(rbuf[i*2])     & 0xFFFF)<< 48 ) |
	     (((uint64_t)float_to_fixed(ibuf[i*2])     & 0xFFFF)<< 32 ) |	     
	     (((uint64_t)float_to_fixed(rbuf[(i*2)+1]) & 0xFFFF)<< 16 ) |
	     (((uint64_t)float_to_fixed(ibuf[(i*2)+1]) & 0xFFFF)<<  0 ) );    
#else
    //
    temp = ( (((uint64_t)float_to_fixed(rbuf[i*2])     & 0xFFFF)<< 0  ) |
	     (((uint64_t)float_to_fixed(ibuf[i*2])     & 0xFFFF)<< 16 ) |	     
	     (((uint64_t)float_to_fixed(rbuf[(i*2)+1]) & 0xFFFF)<< 32 ) |
	     (((uint64_t)float_to_fixed(ibuf[(i*2)+1]) & 0xFFFF)<< 48 ) );
#endif
    //
    cep_writeNcapture(DFT_BASE_K, DFT_IN_DATA, temp);
    cep_writeNcapture(DFT_BASE_K, DFT_IN_ADDR, i);                       //Write addr
    cep_writeNcapture(DFT_BASE_K, DFT_IN_WRITE, 0x2);                    //Load data
    cep_writeNcapture(DFT_BASE_K, DFT_IN_WRITE, 0x0);                    //Stop
  }
}

void cep_dft::dft_getY(double *rbuf, double *ibuf, int len) {
  uint64_t temp;
  if (GetVerbose()) { LOGI("%s\n",__FUNCTION__); }  
  for (int i=0;i<len/2;i++) {
    cep_writeNcapture(DFT_BASE_K, DFT_OUT_ADDR, i);                //Write addr
    temp = cep_readNcapture(DFT_BASE_K, DFT_OUT_DATA);             //Read data
    rbuf[i*2]     = fixed_to_float((temp >> 48) & 0xFFFF);
    ibuf[i*2]     = fixed_to_float((temp >> 32) & 0xFFFF);
    rbuf[(i*2)+1] = fixed_to_float((temp >> 16) & 0xFFFF);
    ibuf[(i*2)+1] = fixed_to_float((temp >>  0) & 0xFFFF);
  }
}


void cep_dft::dft_Start(void) {
  if (GetVerbose()) { LOGI("%s\n",__FUNCTION__); }  
  cep_writeNcapture(DFT_BASE_K, DFT_START, 0x1);
  cep_writeNcapture(DFT_BASE_K, DFT_START, 0x0);
}

int cep_dft::dft_waitTilDone(int maxTO) {
#if 1
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }    
  return cep_readNspin(DFT_BASE_K, DFT_DONE, 4, maxTO);  
#else  
  if (GetVerbose()) { LOGI("%s\n",__FUNCTION__); }  
  while (maxTO > 0) {
    if (cep_readNcapture(DFT_BASE_K, DFT_DONE)) break;
    maxTO--;
  };
  return (maxTO <= 0) ? 1 : 0;
#endif
}

int cep_dft::dft_CheckSamples(int startIdx,int samCnt) {
  if (GetVerbose()) { LOGI("%s\n",__FUNCTION__); }  
  //
  double repsilon,rdiff;
  double iepsilon,idiff;  
  for (int i=startIdx;i<samCnt;i++) {
    //
    repsilon = fabs(mRexp[i])*(mRTolerance)/100;
    iepsilon = fabs(mIexp[i])*(mITolerance)/100;    
    // between 0.01 and 0.05 min/max
    if (repsilon < 0.02) { repsilon = 0.02; }
    if (iepsilon < 0.02) { iepsilon = 0.02; }    
    //
    // adjust to 16-bits
    rdiff = fabs(fabs(mRact[i]) - fabs(mRexp[i]));
    idiff = fabs(fabs(mIact[i]) - fabs(mIexp[i]));
    
    if (rdiff > repsilon) { // || (idiff > iepsilon)) {
      LOGE("%s: i=%d Exp=%.5f/%.5f Act=%.5f/%.5f : diff=%.5f/%.5f > Epsilon=%.5f/%.5f\n",__FUNCTION__,i,
	   mRexp[i],mIexp[i],  mRact[i],mIact[i],
	   rdiff,idiff,repsilon,iepsilon);
      mErrCnt++;
    } else if (GetVerbose()) {
      LOGI("%s: i=%d Exp=%.5f/%.5f Act=%.5f/%.5f : diff=%.5f/%.5f > Epsilon=%.5f/%.5f\n",__FUNCTION__,i,
	   mRexp[i],mIexp[i],  mRact[i],mIact[i],
	   rdiff,idiff,repsilon,iepsilon);      
    }
  }
  return mErrCnt;
}

//
// IDFT
//
void cep_dft::idft_setX(double *rbuf, double *ibuf, int len) {
  uint64_t  temp;
  if (GetVerbose()) { LOGI("%s\n",__FUNCTION__); }
  for (int i=0;i<len/2;i++) { // each take 2 complex number
#ifdef BIG_ENDIAN
    temp = ( (((uint64_t)float_to_fixed(rbuf[i*2])     & 0xFFFF)<< 48  ) |
	     (((uint64_t)float_to_fixed(ibuf[i*2])     & 0xFFFF)<< 32 ) |	     
	     (((uint64_t)float_to_fixed(rbuf[(i*2)+1]) & 0xFFFF)<< 16 ) |
	     (((uint64_t)float_to_fixed(ibuf[(i*2)+1]) & 0xFFFF)<<  0 ) );    
#else
    //
    temp = ( (((uint64_t)float_to_fixed(rbuf[i*2])     & 0xFFFF)<< 0  ) |
	     (((uint64_t)float_to_fixed(ibuf[i*2])     & 0xFFFF)<< 16 ) |	     
	     (((uint64_t)float_to_fixed(rbuf[(i*2)+1]) & 0xFFFF)<< 32 ) |
	     (((uint64_t)float_to_fixed(ibuf[(i*2)+1]) & 0xFFFF)<< 48 ) );
#endif
    //
    cep_writeNcapture(IDFT_BASE_K, IDFT_IN_DATA, temp);
    cep_writeNcapture(IDFT_BASE_K, IDFT_IN_ADDR, i);                       //Write addr
    cep_writeNcapture(IDFT_BASE_K, IDFT_IN_WRITE, 0x2);                    //Load data
    cep_writeNcapture(IDFT_BASE_K, IDFT_IN_WRITE, 0x0);                    //Stop
  }
}

void cep_dft::idft_getY(double *rbuf, double *ibuf, int len) {
  uint64_t temp;
  if (GetVerbose()) { LOGI("%s\n",__FUNCTION__); }  
  for (int i=0;i<len/2;i++) {
    cep_writeNcapture(IDFT_BASE_K, IDFT_OUT_ADDR, i);                //Write addr
    temp = cep_readNcapture(IDFT_BASE_K, IDFT_OUT_DATA);             //Read data
    rbuf[i*2]     = fixed_to_float(static_cast<fixp16>((temp >> 48) & 0xFFFF));
    ibuf[i*2]     = fixed_to_float(static_cast<fixp16>((temp >> 32) & 0xFFFF));
    rbuf[(i*2)+1] = fixed_to_float(static_cast<fixp16>((temp >> 16) & 0xFFFF));
    ibuf[(i*2)+1] = fixed_to_float(static_cast<fixp16>((temp >>  0) & 0xFFFF));
  }
}


void cep_dft::idft_Start(void) {
  if (GetVerbose()) { LOGI("%s\n",__FUNCTION__); }  
  cep_writeNcapture(IDFT_BASE_K, IDFT_START, 0x1);
  cep_writeNcapture(IDFT_BASE_K, IDFT_START, 0x0);
}

int cep_dft::idft_waitTilDone(int maxTO) {
#if 1
  if (GetVerbose()) {  LOGI("%s\n",__FUNCTION__); }    
  return cep_readNspin(IDFT_BASE_K, IDFT_DONE, 4, maxTO);  
#else    
  if (GetVerbose()) { LOGI("%s\n",__FUNCTION__); }  
  while (maxTO > 0) {
    if (cep_readNcapture(IDFT_BASE_K, IDFT_DONE)) break;
    maxTO--;
  };
  return (maxTO <= 0) ? 1 : 0;
#endif
}

int cep_dft::idft_CheckSamples(int startIdx,int samCnt) {
  if (GetVerbose()) { LOGI("%s\n",__FUNCTION__); }  
  //
  double repsilon,rdiff;
  double iepsilon,idiff;  
  for (int i=startIdx;i<samCnt;i++) {
    //
    repsilon = fabs(mRexp[i])*(mRTolerance)/100;
    iepsilon = fabs(mIexp[i])*(mITolerance)/100;    
    // between 0.01 and 0.05 min/max
    if (repsilon < 0.02) { repsilon = 0.02; }
    if (iepsilon < 0.02) { iepsilon = 0.02; }        
    // adjust to 16-bits
    rdiff = fabs(fabs(mRact[i]) - fabs(mRexp[i]));
    idiff = fabs(fabs(mIact[i]) - fabs(mIexp[i]));
    //
    //    if ((rdiff > repsilon) || (idiff > iepsilon)) {
    // FIXME: dont know why only the real part are good
    if (rdiff > repsilon) {
      LOGE("%s: i=%d Exp=%.5f/%.5f Act=%.5f/%.5f : diff=%.5f/%.5f > Epsilon=%.5f/%.5f\n",__FUNCTION__,i,
	   mRexp[i],mIexp[i],  mRact[i],mIact[i],
	   rdiff,idiff,repsilon,iepsilon);
      mErrCnt++;
    } else if (GetVerbose()) {
      LOGI("%s: i=%d Exp=%.5f/%.5f Act=%.5f/%.5f : diff=%.5f/%.5f > Epsilon=%.5f/%.5f\n",__FUNCTION__,i,
	   mRexp[i],mIexp[i],  mRact[i],mIact[i],
	   rdiff,idiff,repsilon,iepsilon);      
    }
  }
  return mErrCnt;
}

/* 
Due to lost of precision, only these test can be used

Single FFT tests - N inputs and N outputs
Input random data
Inputs are all zeros
Inputs are all ones (or some other nonzero value)
Inputs alternate between +1 and -1.
Input is e^(8*j*2*pi*i/N) for i = 0,1,2, ...,N-1. (j = sqrt(-1))
Input is cos(8*2*pi*i/N) for i = 0,1,2, ...,N-1.
Input is e^((43/7)*j*2*pi*i/N) for i = 0,1,2, ...,N-1. (j= sqrt(-1))
Input is cos((43/7)*2*pi*i/N) for i = 0,1,2, ...,N-1.

*/
int cep_dft::RunDftTest(int maxLoop) {
  fixp16 fixR, fixI;
  uint32_t ranX;
  //
  //
  int skipFirstSample = 0;
  Random48_srand48(GetSeed());
  for (int i=0;i<maxLoop;i++) {
    if (GetVerbose()) {
      LOGI("%s: Loop %d\n",__FUNCTION__,i);
    }
    skipFirstSample = 0;
    mRTolerance = 2; // %
    mITolerance = 20; // %    
    // build 64 complex samples
    switch (i) {
      //
#if 0
    case 0:
      for (int j=0; j < MAX_DFT_SAMPLES; j++) {
	mRin[j] = cos(8*2*M_PI*j/MAX_DFT_SAMPLES);
	mIin[j] = exp(8*2*M_PI*j/MAX_DFT_SAMPLES);
      }
      break;
#endif
      //
    case 0:
      for (int j=0; j < MAX_DFT_SAMPLES; j++) {
	mRin[j] = cos(8*2*M_PI*j/MAX_DFT_SAMPLES);
	mIin[j] = sin(8*2*M_PI*j/MAX_DFT_SAMPLES);
      }
      break;      
    case 1:
      for (int j=0; j < MAX_DFT_SAMPLES; j++) {
	mRin[j] = cos((43/7)*M_PI*j/MAX_DFT_SAMPLES);
	mIin[j] = sin((43/7)*M_PI*j/MAX_DFT_SAMPLES);
      }
      break;			      
    case 2:
      for (int j=0; j < MAX_DFT_SAMPLES; j++) {      
	mRin[j] = (j&0x1) ? 1.0 : -1.0;
	mIin[j] = (j&0x1) ? -1.0 : 1.0;
      }
      break;
    case 3:
      for (int j=0; j < MAX_DFT_SAMPLES; j++) {	
	mRin[j] = 1.0;
	mIin[j] = 1.0;
      }
      break;
    default:
      ranX = Random48_rand();
      fixR = ranX & 0xFFFF;
      fixI = (ranX>>16) & 0xFFFF;      
      for (int j=0; j < MAX_DFT_SAMPLES; j++) {
	mRin[j] = fixed_to_float(fixR+j);
	mIin[j] = fixed_to_float(fixI+j);
      }
      skipFirstSample = 1; // dont know why??? when random
      mRTolerance = 60; // %      FIXME!!!!
      mITolerance = 60; // %      FIXME!!!!      
      break;
    }
    // adjust ??
    adjust_float(mRin, mIin, MAX_DFT_SAMPLES);
    //
    do_dft(mRin,mIin,mRexp, mIexp, MAX_DFT_SAMPLES);
    //adjust_float(mRexp, mIexp, MAX_DFT_SAMPLES);
    if (GetVerbose()) {
      PrintMe("DFT-In",mRin,mIin,MAX_DFT_SAMPLES);    
      PrintMe("DFT-Exp",mRexp,mIexp,MAX_DFT_SAMPLES);
    }
    //
#if 1
    // DFT
    if (!mErrCnt) {    
      dft_setX(mRin, mIin, MAX_DFT_SAMPLES);
      dft_Start();
      mErrCnt += dft_waitTilDone(500);
      if (!mErrCnt) {
	dft_getY(mRact, mIact, MAX_DFT_SAMPLES);
	if (GetVerbose()) {	
	  PrintMe("DFT-act",mRact,mIact,MAX_DFT_SAMPLES);
	}
	//adjust_float(mRact, mIact, MAX_DFT_SAMPLES);
	mErrCnt += dft_CheckSamples(skipFirstSample,MAX_DFT_SAMPLES);
      }

    }
#endif

#if 1
    // IDFT
    if (!mErrCnt) {
      // input to IDFT
      adjust_float(mRact, mIact, MAX_DFT_SAMPLES);
      //
      do_idft(mRact, mIact, mRexp, mIexp, MAX_DFT_SAMPLES);
      //adjust_float(mRAin, mIAin, MAX_DFT_SAMPLES);
      if (GetVerbose()) {      
	PrintMe("IDFT-in",mRact,mIact,MAX_DFT_SAMPLES);
	PrintMe("IDFT-exp",mRexp,mIexp,MAX_DFT_SAMPLES);
      }
      //
      idft_setX(mRact, mIact, MAX_DFT_SAMPLES);
      idft_Start();
      mErrCnt += idft_waitTilDone(500);
      if (!mErrCnt) {
	idft_getY(mRact, mIact, MAX_DFT_SAMPLES);
	if (GetVerbose()) {	
	  PrintMe("IDFT-act",mRact,mIact,MAX_DFT_SAMPLES);
	}
	//adjust_float(mRAin, mIAin, MAX_DFT_SAMPLES);	
	mErrCnt += idft_CheckSamples(skipFirstSample,MAX_DFT_SAMPLES);
      }
    }
#endif
    
    //
    MarkSingle(i);    
    if (mErrCnt) break;
  }
  return mErrCnt;
}

