//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
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

#include "cep_aes.h"
#include "cep_des3.h"
#include "cep_md5.h"
#include "cep_sha256.h"
#include "cep_gps.h"
#include "cep_fir.h"
#include "cep_iir.h"
#include "cep_dft.h"
#include "cep_rsa.h"


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
  cep_sha256 sha256(seed,verbose);
  cep_md5 md5(seed,verbose);
  cep_gps gps(seed,verbose);
  cep_fir fir(seed,verbose);
  cep_iir iir(seed,verbose);
  cep_dft dft(seed,verbose);
  cep_rsa rsa(seed,verbose);    
  
  int maxBytes = 8;
  //
  int captureOn = 0;
#ifdef CAPTURE_CMD_SEQUENCE
  captureOn = CAPTURE_CMD_SEQUENCE;
#endif
  int maxLoop;
  //
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
    if (errCnt) return errCnt;
    
    break;
    
  case 1 : 
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
    maxBytes=8;
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
  }
  // else do nothing
#endif
  
  return errCnt;
}

