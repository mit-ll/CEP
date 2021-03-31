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
int cepMacroMix_runTest(int cpuId, int mask, int seed, int verbose) {
  //
  int errCnt = 0;

#ifndef BARE_MODE
  cep_aes aes(seed,verbose);
  cep_des3 des3(seed,verbose);
  cep_md5 md5(seed,verbose);
  cep_sha256 sha256(seed,verbose);
  cep_fir fir(seed,verbose);
  cep_iir iir(seed,verbose);
  cep_gps gps(seed,verbose);    // Note: The GPS constructor does write to the core
  cep_dft dft(seed,verbose);
  cep_rsa rsa(seed,verbose);

  //
  int captureOn = 0;
#ifdef CAPTURE_CMD_SEQUENCE
  captureOn = CAPTURE_CMD_SEQUENCE;
#endif
  int maxLoop;
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
    aes.init();
    aes.SetCaptureMode(captureOn,"../../drivers/vectors","aes");      
    //
    maxLoop =  200;
    init_aes();
    errCnt += aes.RunAes192Test(maxLoop);
    //
    aes.freeMe();
    if (errCnt) return errCnt;

    //
    // DES3
    //
    des3.init();      
    des3.SetCaptureMode(captureOn,"../../drivers/vectors","des3");      
    //
    maxLoop = 150;
    init_des3();
    errCnt += des3.RunDes3Test(maxLoop);
    //
    des3.freeMe();
    if (errCnt) return errCnt;
    
    //
    // MD5
    //
    md5.init();
    md5.SetCaptureMode(captureOn,"../../drivers/vectors","md5");          
    //
    maxLoop = 32;
    init_md5();
    errCnt += md5.RunMd5Test(maxLoop);
    //
    md5.freeMe();

    // Free the srot object at the end of thread 0 (freeing it too soon will result
    // problems with the other threads who use the object)

    if (errCnt) return errCnt;
    break;
    
  case 1 : 
    //
    // FIR
    //
    fir.init();      
    fir.SetCaptureMode(captureOn,"../../drivers/vectors","fir");      
    //
    maxLoop = 10;
    init_fir();
    errCnt += fir.RunFirTest(maxLoop);
    //
    fir.freeMe();
    if (errCnt) return errCnt;
    
    //
    // IIR
    //    
    iir.init();
    iir.SetCaptureMode(captureOn,"../../drivers/vectors","iir");          
    //
    maxLoop = 10;
    init_iir();
    errCnt += iir.RunIirTest(maxLoop);
    //
    iir.freeMe();
    if (errCnt) return errCnt;

    //
    // SHA256
    //
    sha256.init();
    sha256.SetCaptureMode(captureOn,"../../drivers/vectors","sha256");          
    //
    maxLoop = 32;
    init_sha256();
    errCnt += sha256.RunSha256Test(maxLoop);
    //
    sha256.freeMe();
    if (errCnt) return errCnt;
  

    break;

  case 2:
    //
    // GPS
    //
    gps.init();  
    gps.SetCaptureMode(captureOn,"../../drivers/vectors","gps");  
    //
    maxLoop  = 38;
    init_gps();
    errCnt += gps.RunGpsTest(maxLoop);
    //
    gps.freeMe();
    if (errCnt) return errCnt;    
    //
    // DFT & IDFT
    //
    dft.init();
    dft.SetCaptureMode(captureOn,"../../drivers/vectors","dft");          
    //
    maxLoop = 10;
    init_dft();
    errCnt += dft.RunDftTest(maxLoop);
    //
    dft.freeMe();

    if (errCnt) return errCnt;    
    break;
  
  case 3:
    //
    // RSA
    //
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
  cep_aes aes(seed,verbose);
  cep_des3 des3(seed,verbose);
  cep_md5 md5(seed,verbose);
  cep_sha256 sha256(seed,verbose);
  cep_fir fir(seed,verbose);
  cep_iir iir(seed,verbose);
  cep_gps gps(seed,verbose);    // Note: The GPS constructor does write to the core
  cep_dft dft(seed,verbose);
  cep_rsa rsa(seed,verbose);

  //
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
  case 0 :
    //
    // AES
    //    
    aes.init();
    aes.SetCaptureMode(captureOn,"../../drivers/vectors","aes");      
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
    des3.init();      
    des3.SetCaptureMode(captureOn,"../../drivers/vectors","des3");      
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
    md5.init();
    md5.SetCaptureMode(captureOn,"../../drivers/vectors","md5");          
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
    
  case 1 : 
    //
    // IIR
    //    
    iir.init();
    iir.SetCaptureMode(captureOn,"../../drivers/vectors","iir");          
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
    fir.init();      
    fir.SetCaptureMode(captureOn,"../../drivers/vectors","fir");      
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
    sha256.init();
    sha256.SetCaptureMode(captureOn,"../../drivers/vectors","sha256");          
    //
    maxLoop = 2;
    init_sha256();
    errCnt += sha256.RunSha256Test(maxLoop);
    //
    sha256.freeMe();
    srot.DisableLLKI(SHA256_BASE_K); // use as KeyIndex
    CHECK4ERR(errCnt,"sha256");

    break;

  case 2:
    //
    // GPS
    //
    gps.init();  
    gps.SetCaptureMode(captureOn,"../../drivers/vectors","gps");  
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
    dft.init();
    dft.SetCaptureMode(captureOn,"../../drivers/vectors","dft");          
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
  
  case 3:
    //
    // RSA
    //
    rsa.init();
    rsa.SetCaptureMode(captureOn,"../../drivers/vectors","rsa");      
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

  } // switch (cpuId)
  srot.freeMe();    
// else do nothing
#endif
  //

  return errCnt;
}

