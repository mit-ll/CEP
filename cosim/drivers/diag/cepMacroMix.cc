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

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepMacroMix.h"

#include "cep_apis.h"
#include "portable_io.h"

#include "cepregression.h"

#ifdef BARE_MODE

#else
#include "simPio.h"
#endif

#include "CEP.h"
#include "cep_aes.h"
#include "cep_des3.h"
#include "cep_md5.h"
#include "cep_sha256.h"
#include "cep_gps.h"
#include "cep_fir.h"
#include "cep_iir.h"
#include "cep_dft.h"
#include "cep_rsa.h"
#include "cep_srot.h"

#define CHECK4ERR(e,name) do {			\
    if (!errCnt) { \
      if (srot.GetVerbose()) { \
	LOGE("ERROR: test=%s No error detected.. bad!!!\n",name);	\
      } \
      errCnt++; \
      return errCnt; \
    } else { \
      if (srot.GetVerbose()) { \
	LOGI("test=%s: Error detected as expected.. Good!!!\n",name);	\
      } \
      errCnt = 0; \
    } \
} while (0)

//
// =============================
// The test itself
// =============================
//
// 04/07/21 : added cryptoMask to allow running indiviual crypto core
//
#define IS_ON(c) (1 << c ## _BASE_K) & cryptoMask

int cepMacroMix_runTest(int cpuId, int mask, int cryptoMask, int seed, int verbose) {
  //
  int errCnt = 0;

#ifndef BARE_MODE
  //
  // ONly if in the cryptoMask
  //








  //
  int captureOn = 0;
#ifdef CAPTURE_CMD_SEQUENCE
  captureOn = CAPTURE_CMD_SEQUENCE;
#endif
  int maxLoop;
  if (VERBOSE1()) {
    LOGI("%s: mask=0x%x cryptoMask=0x%08x\n",__FUNCTION__,mask,cryptoMask);
  }
  //
  //
  // do the LLKI unlock here
  //
  cep_srot srot(verbose);
  srot.SetCpuActiveMask(mask);
  srot.SetSrotFlag(captureOn);
  errCnt += srot.LLKI_Setup(cpuId);
  if (errCnt) return errCnt;
  //
  //
  switch (cpuId) {
  case 0 :
    //
    // AES
    //    
    if (IS_ON(AES)) {
      cep_aes aes(seed,verbose);
      aes.init();
      aes.SetCaptureMode(captureOn,"../../drivers/vectors","aes");      
      //
      maxLoop =  200;
      init_aes();
      errCnt += aes.RunAes192Test(maxLoop);
      //
      aes.freeMe();
      if (errCnt) return errCnt;
    } 
    //
    // DES3
    //
    if (IS_ON(DES3)) {
      cep_des3 des3(seed,verbose);
      des3.init();      
      des3.SetCaptureMode(captureOn,"../../drivers/vectors","des3");      
      //
      maxLoop = 150;
      init_des3();
      errCnt += des3.RunDes3Test(maxLoop);
      //
      des3.freeMe();
      if (errCnt) return errCnt;
    } 
    //
    // MD5
    //
    if (IS_ON(MD5)) {
      cep_md5 md5(seed,verbose);
      md5.init();
      md5.SetCaptureMode(captureOn,"../../drivers/vectors","md5");          
      //
      maxLoop = 32;
      init_md5();
      errCnt += md5.RunMd5Test(maxLoop);
      //
      md5.freeMe();
    } 
    // Free the srot object at the end of thread 0 (freeing it too soon will result
    // problems with the other threads who use the object)

    if (errCnt) return errCnt;
    break;
    
  case 1 : 
    //
    // FIR
    //
    if (IS_ON(FIR)) {
      cep_fir fir(seed,verbose);
      fir.init();      
      fir.SetCaptureMode(captureOn,"../../drivers/vectors","fir");      
      //
      maxLoop = 10;
      init_fir();
      errCnt += fir.RunFirTest(maxLoop);
      //
      fir.freeMe();
      if (errCnt) return errCnt;
    } 
    //
    // IIR
    //
    if (IS_ON(IIR)) {    
      cep_iir iir(seed,verbose);
      iir.init();
      iir.SetCaptureMode(captureOn,"../../drivers/vectors","iir");          
      //
      maxLoop = 10;
      init_iir();
      errCnt += iir.RunIirTest(maxLoop);
      //
      iir.freeMe();
      if (errCnt) return errCnt;
    } 
    //
    // SHA256
    //
    if (IS_ON(SHA256)) {
      cep_sha256 sha256(seed,verbose);
      sha256.init();
      sha256.SetCaptureMode(captureOn,"../../drivers/vectors","sha256");          
      //
      maxLoop = 32;
      init_sha256();
      errCnt += sha256.RunSha256Test(maxLoop);
      //
      sha256.freeMe();
      if (errCnt) return errCnt;
    } 

    break;

  case 2:
    //
    // GPS
    //
    if (IS_ON(GPS)) {
      cep_gps gps(seed,verbose);    // Note: The GPS constructor does write to the core
      gps.init();  
      gps.SetCaptureMode(captureOn,"../../drivers/vectors","gps");  
      //
      maxLoop  = 38;
      init_gps();
      errCnt += gps.RunGpsTest(maxLoop);
      //
      gps.freeMe();
      if (errCnt) return errCnt;    
    } 
    //
    // DFT & IDFT
    //
    if (IS_ON(DFT)) {
      cep_dft dft(seed,verbose);
      dft.init();
      dft.SetCaptureMode(captureOn,"../../drivers/vectors","dft");          
      //
      maxLoop = 10;
      init_dft();
      errCnt += dft.RunDftTest(maxLoop);
      //
      dft.freeMe();
    } 
    if (errCnt) return errCnt;    
    break;
  
  case 3:
    //
    // RSA
    //
    if (IS_ON(RSA)) {
      cep_rsa rsa(seed,verbose);
      rsa.init();
      rsa.SetCaptureMode(captureOn,"../../drivers/vectors","rsa");      
      //
      maxLoop = 4;
      int maxBytes=8;
      //
      init_rsa();
      if (!errCnt)  {
	errCnt += rsa.RunRsaTest2(maxLoop,256/8);
      }    
      if (!errCnt)  {    
	errCnt += rsa.RunRsaMemTest(0xf, 1024); // 8K bits
      }
      if (!errCnt)  {    
	errCnt += rsa.RunRsaTest(maxLoop,maxBytes);
      }
      //
      rsa.freeMe();
    } 
    if (errCnt) return errCnt;    
    break;

  } // switch (cpuId)
  srot.freeMe();  

// else do nothing
#endif
  return errCnt;
}

//
// =============================
// With bad keys
// =============================
//
int cepMacroMix_runBadKeysTest(int cpuId, int mask, int seed, int verbose) {
  //
  int errCnt = 0;

#ifndef BARE_MODE
  //
  //
  int captureOn = 0; //  no capturing!!!
  int maxLoop;
  //
  //
  // do the LLKI unlock here
  //
  cep_srot srot(verbose);
  srot.SetCpuActiveMask(mask);
  srot.SetSrotFlag(captureOn);
  srot.LLKI_invertKey(1); // all test should fails!!
  errCnt += srot.LLKI_Setup(cpuId);
  if (errCnt) return errCnt;
  //
  //
  switch (cpuId) {
  case 0 : {
    //
    // AES
    //    
    cep_aes aes(seed,verbose);
    aes.init();
    aes.SetCaptureMode(captureOn,"../../drivers/vectors","aes");      
    aes.SetExpErr(1);
    //
    maxLoop =  2;
    init_aes();
    errCnt += aes.RunAes192Test(maxLoop);
    //
    aes.freeMe();
    srot.DisableLLKI(AES_BASE_K); // use as KeyIndex
    //
    CHECK4ERR(errCnt,"aes");
    //
    // DES3
    //
    cep_des3 des3(seed,verbose);
    des3.init();      
    des3.SetCaptureMode(captureOn,"../../drivers/vectors","des3");      
    des3.SetExpErr(1);
    //
    maxLoop = 2;
    init_des3();
    errCnt += des3.RunDes3Test(maxLoop);
    //
    des3.freeMe();
    srot.DisableLLKI(DES3_BASE_K); // use as KeyIndex
    CHECK4ERR(errCnt,"des3");
    //
    // MD5
    //
    cep_md5 md5(seed,verbose);
    md5.init();
    md5.SetCaptureMode(captureOn,"../../drivers/vectors","md5");          
    md5.SetExpErr(1);
    //
    maxLoop = 2;
    init_md5();
    errCnt += md5.RunMd5Test(maxLoop);
    //
    md5.freeMe();
    srot.DisableLLKI(MD5_BASE_K); // use as KeyIndex
    // Free the srot object at the end of thread 0 (freeing it too soon will result
    // problems with the other threads who use the object)
    CHECK4ERR(errCnt,"md5");
    break;
  }
    
  case 1 : {
    //
    // IIR
    //    
    cep_iir iir(seed,verbose);
    iir.init();
    iir.SetCaptureMode(captureOn,"../../drivers/vectors","iir");          
    iir.SetExpErr(1);
    //
    maxLoop = 2;
    init_iir();
    errCnt += iir.RunIirTest(maxLoop);
    //
    iir.freeMe();
    srot.DisableLLKI(IIR_BASE_K); // use as KeyIndex
    CHECK4ERR(errCnt,"iir");
    //
    // FIR
    //
    cep_fir fir(seed,verbose);
    fir.init();      
    fir.SetCaptureMode(captureOn,"../../drivers/vectors","fir");      
    fir.SetExpErr(1);
    //
    maxLoop = 2;
    init_fir();
    errCnt += fir.RunFirTest(maxLoop);
    //
    fir.freeMe();
    srot.DisableLLKI(FIR_BASE_K); // use as KeyIndex
    CHECK4ERR(errCnt,"fir");

    //
    // SHA256
    //
    cep_sha256 sha256(seed,verbose);
    sha256.init();
    sha256.SetCaptureMode(captureOn,"../../drivers/vectors","sha256");          
    sha256.SetExpErr(1);
    //
    maxLoop = 2;
    init_sha256();
    errCnt += sha256.RunSha256Test(maxLoop);
    //
    sha256.freeMe();
    srot.DisableLLKI(SHA256_BASE_K); // use as KeyIndex
    CHECK4ERR(errCnt,"sha256");

    break;
  }

  case 2: {
    //
    // GPS
    //
    cep_gps gps(seed,verbose);    // Note: The GPS constructor does write to the core
    gps.init();  
    gps.SetCaptureMode(captureOn,"../../drivers/vectors","gps");  
    gps.SetExpErr(1);
    //
    maxLoop  = 2;
    init_gps();
    errCnt += gps.RunGpsTest(maxLoop);
    //
    gps.freeMe();
    srot.DisableLLKI(GPS_BASE_K); // use as KeyIndex
    CHECK4ERR(errCnt,"gps");
    //
    // DFT & IDFT
    //
    cep_dft dft(seed,verbose);
    dft.init();
    dft.SetCaptureMode(captureOn,"../../drivers/vectors","dft");          
    dft.SetExpErr(1);
    //
    maxLoop = 2;
    init_dft();
    errCnt += dft.RunDftTest(maxLoop);
    //
    dft.freeMe();
    srot.DisableLLKI(DFT_BASE_K); // use as KeyIndex
    srot.DisableLLKI(IDFT_BASE_K); // use as KeyIndex
    CHECK4ERR(errCnt,"dft");
    break;
  }
  
  case 3: {
    //
    // RSA
    //
    cep_rsa rsa(seed,verbose);
    rsa.init();
    rsa.SetCaptureMode(captureOn,"../../drivers/vectors","rsa");      
    rsa.SetExpErr(1);
    //
    maxLoop = 2;
    int maxBytes=8;
    //
    init_rsa();
    if (!errCnt)  {    
      errCnt += rsa.RunRsaTest(maxLoop,maxBytes);
    }
    //
    rsa.freeMe();
    srot.DisableLLKI(RSA_BASE_K); // use as KeyIndex
    CHECK4ERR(errCnt,"rsa");
    break;
  }

  } // switch (cpuId)
  srot.freeMe();    
// else do nothing
#endif
  //

  return errCnt;
}

