//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cepMacroMix.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    Allows for selective test of CEP acclerator cores
// Notes:          CEP.h defines the core types as well as the currently
//                 enabled cores.  The cryptomix parameter defines which
//                 of the currently enabled cores to test.  Bit definition
//                 (up to 32) corresponds to the index in the cep_core_info
//                 array.
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
#include "cep_idft.h"
#include "cep_rsa.h"
#include "cep_srot.h"

#define IS_ON(c) ((1 << c) & coreMask)
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

int cepMacroMix_runTest(int cpuId, int cpuActiveMask, int coreMask, int seed, int verbose) {
  //
  int errCnt = 0;

#ifndef BARE_MODE
  //
  // ONly if in the coreMask
  //
  //
  int captureOn = 0;
#ifdef CAPTURE_CMD_SEQUENCE
  captureOn = CAPTURE_CMD_SEQUENCE;
#endif
  int maxLoop;
  if (VERBOSE1()) {
    LOGI("%s: cpuActiveMask=0x%x coreMask=0x%08x\n",__FUNCTION__, cpuActiveMask, coreMask);
  }

  // Instantiate and initialize an SRoT object
  cep_srot srot(SROT_INDEX, CEP_VERSION_REG_INDEX, verbose);
  
  srot.SetCpuActiveMask(cpuActiveMask);
  srot.SetSrotFlag(captureOn);
  srot.SetCoreMask(coreMask);

  // Initialize the LLKI (One a single core will perform this function)
  errCnt += srot.LLKI_Setup(cpuId);
  
  if (errCnt) return errCnt;

  int maxCoreIndex = CEP_LLKI_CORES;

  for (int coreIndex = 0; coreIndex < maxCoreIndex; coreIndex ++) {
    cep_core_info_t core = cep_core_info[coreIndex];
    core_type_t coreType = core.type;
    if (IS_ON(coreIndex) && core.enabled) {
      switch (coreType) {
      case AES_CORE :
        //
        // AES
        //
        if (cpuId == 0) {
          cep_aes aes(coreIndex,seed,verbose);
          aes.SetCaptureMode(captureOn,"../../drivers/vectors","aes");

          maxLoop =  200;
          errCnt += aes.RunAes192Test(maxLoop);

          aes.freeMe();
        }
      break;
      case DES3_CORE:
        //
        // DES3
        //
        if (cpuId == 0) {
          cep_des3 des3(coreIndex,seed,verbose);
          des3.SetCaptureMode(captureOn,"../../drivers/vectors","des3");

          maxLoop = 150;
          errCnt += des3.RunDes3Test(maxLoop);

          des3.freeMe();
        } 
      break;
      case MD5_CORE:
        //
        // MD5
        //
        if (cpuId == 0) {
          cep_md5 md5(coreIndex,seed,verbose);
          md5.SetCaptureMode(captureOn,"../../drivers/vectors","md5");

          maxLoop = 32;
          errCnt += md5.RunMd5Test(maxLoop);

          md5.freeMe();
        } 
      break;
      case FIR_CORE:
        //
        // FIR
        //
        if (cpuId == 1) {
          cep_fir fir(coreIndex,seed,verbose);
          fir.SetCaptureMode(captureOn,"../../drivers/vectors","fir");

          maxLoop = 10;
          errCnt += fir.RunFirTest(maxLoop);

          fir.freeMe();
        }
      break;
      case IIR_CORE:
        //
        // IIR
        //
        if (cpuId == 1) {
          cep_iir iir(coreIndex,seed,verbose);
          iir.SetCaptureMode(captureOn,"../../drivers/vectors","iir");

          maxLoop = 10;
          errCnt += iir.RunIirTest(maxLoop);

          iir.freeMe();
        } 
      break;
      case SHA256_CORE:
        //
        // SHA256
        //
        if (cpuId == 1) {
          cep_sha256 sha256(coreIndex,seed,verbose);
          sha256.SetCaptureMode(captureOn,"../../drivers/vectors","sha256");

          maxLoop = 32;
          errCnt += sha256.RunSha256Test(maxLoop);

          sha256.freeMe();
        } 
      break;
      case GPS_CORE:
        //
        // GPS
        //
        if (cpuId == 2) {
          cep_gps gps(coreIndex,seed,verbose);
          gps.SetCaptureMode(captureOn,"../../drivers/vectors","gps");

          maxLoop  = 38;
          errCnt += gps.RunGpsTest(maxLoop);

          gps.freeMe();
        } 
      break;
      case DFT_CORE:
        //
        // DFT
        //
        if (cpuId == 2) {
          cep_dft dft(coreIndex,seed,verbose);
          dft.SetCaptureMode(captureOn,"../../drivers/vectors","dft");

          maxLoop = 10;
          errCnt += dft.RunDftTest(maxLoop);

          dft.freeMe();
        }
      break;
      case IDFT_CORE:
        //
        // IDFT
        //
        if (cpuId == 2) {
          cep_idft idft(coreIndex,seed,verbose);
          idft.SetCaptureMode(captureOn,"../../drivers/vectors","idft");

          maxLoop = 10;
          errCnt += idft.RunIdftTest(maxLoop);

          idft.freeMe();
        }
      
      break;
      case RSA_CORE:
        //
        // RSA
        //
        if (cpuId == 3) {
          cep_rsa rsa(coreIndex,seed,verbose);
          rsa.SetCaptureMode(captureOn,"../../drivers/vectors","rsa");

          maxLoop = 4;
          errCnt += rsa.RunRsaTest(maxLoop,8);

          rsa.freeMe();
        }
        break;

      } // switch (cpuId)
      if (errCnt) return errCnt;
    } // if isOn(coreIndex)
  } // for (coreIndex)

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
int cepMacroMix_runBadKeysTest(int cpuId, int cpuActiveMask, int coreMask, int seed, int verbose) {
  //
  int errCnt = 0;

// Do nothing in Bare Mode
#ifndef BARE_MODE

  // No capturing for this test
  int captureOn = 0;
  int maxLoop;
  
  // Instantiate and initialize an SRoT object
  cep_srot srot(SROT_INDEX, CEP_VERSION_REG_INDEX, verbose);
  
  srot.SetCpuActiveMask(cpuActiveMask);
  srot.SetSrotFlag(captureOn);
  srot.SetCoreMask(coreMask);
  srot.LLKI_invertKey(1);     // all test should fails!!

  // Initialize the LLKI (One a single core will perform this function)
  errCnt += srot.LLKI_Setup(cpuId);

  if (errCnt) return errCnt;

  int maxCoreIndex = CEP_LLKI_CORES;

  for (int coreIndex = 0; coreIndex < maxCoreIndex; coreIndex ++) {
    cep_core_info_t core = cep_core_info[coreIndex];
    core_type_t coreType = core.type;
    if (IS_ON(coreIndex) && core.enabled) {
      switch (coreType) {
      case AES_CORE :
        //
        // AES
        //
        if (cpuId == 0) {
          cep_aes aes(coreIndex,seed,verbose);
          aes.SetCaptureMode(captureOn,"../../drivers/vectors","aes");
          aes.SetExpErr(1);

          maxLoop =  2;
          errCnt += aes.RunAes192Test(maxLoop);

          aes.freeMe();

          CHECK4ERR(errCnt,"aes");
        }
      break;
      case DES3_CORE:
        //
        // DES3
        //
        if (cpuId == 0) {
          cep_des3 des3(coreIndex,seed,verbose);
          des3.SetCaptureMode(captureOn,"../../drivers/vectors","des3");
          des3.SetExpErr(1);

          maxLoop = 2;
          errCnt += des3.RunDes3Test(maxLoop);

          des3.freeMe();

          CHECK4ERR(errCnt,"des3");
        } 
      break;
      case MD5_CORE:
        //
        // MD5
        //
        if (cpuId == 0) {
          cep_md5 md5(coreIndex,seed,verbose);
          md5.SetCaptureMode(captureOn,"../../drivers/vectors","md5");
          md5.SetExpErr(1);

          maxLoop = 2;
          errCnt += md5.RunMd5Test(maxLoop);

          md5.freeMe();

          CHECK4ERR(errCnt,"md5");
        } 
      break;
      case FIR_CORE:
        //
        // FIR
        //
        if (cpuId == 1) {
          cep_fir fir(coreIndex,seed,verbose);
          fir.SetCaptureMode(captureOn,"../../drivers/vectors","fir");
          fir.SetExpErr(1);

          maxLoop = 2;
          errCnt += fir.RunFirTest(maxLoop);

          fir.freeMe();

          CHECK4ERR(errCnt,"fir");
        }
      break;
      case IIR_CORE:
        //
        // IIR
        //
        if (cpuId == 1) {
          cep_iir iir(coreIndex,seed,verbose);
          iir.SetCaptureMode(captureOn,"../../drivers/vectors","iir");
          iir.SetExpErr(1);

          maxLoop = 2;
          errCnt += iir.RunIirTest(maxLoop);

          iir.freeMe();

          CHECK4ERR(errCnt,"iir");
        } 
      break;
      case SHA256_CORE:
        //
        // SHA256
        //
        if (cpuId == 1) {
          cep_sha256 sha256(coreIndex,seed,verbose);
          sha256.SetCaptureMode(captureOn,"../../drivers/vectors","sha256");
          sha256.SetExpErr(1);

          maxLoop = 2;
          errCnt += sha256.RunSha256Test(maxLoop);

          sha256.freeMe();

          CHECK4ERR(errCnt,"sha256");
        } 
      break;
      case GPS_CORE:
        //
        // GPS
        //
        if (cpuId == 2) {
          cep_gps gps(coreIndex,seed,verbose);
          gps.SetCaptureMode(captureOn,"../../drivers/vectors","gps");
          gps.SetExpErr(1);

          maxLoop  = 2;
          errCnt += gps.RunGpsTest(maxLoop);

          gps.freeMe();

          CHECK4ERR(errCnt,"gps");
        } 
      break;
      case DFT_CORE:
        //
        // DFT
        //
        if (cpuId == 2) {
          cep_dft dft(coreIndex,seed,verbose);
          dft.SetCaptureMode(captureOn,"../../drivers/vectors","dft");
          dft.SetExpErr(1);

          maxLoop = 2;
          errCnt += dft.RunDftTest(maxLoop);

          dft.freeMe();

          CHECK4ERR(errCnt,"dft");
        }
      break;
      case IDFT_CORE:
        //
        // IDFT
        //
        if (cpuId == 2) {
          cep_idft idft(coreIndex,seed,verbose);
          idft.SetCaptureMode(captureOn,"../../drivers/vectors","idft");
          idft.SetExpErr(1);

          maxLoop = 2;
          errCnt += idft.RunIdftTest(maxLoop);

          idft.freeMe();

          CHECK4ERR(errCnt,"idft");
        }
      
      break;
      case RSA_CORE:
        //
        // RSA
        //
        if (cpuId == 3) {
          cep_rsa rsa(coreIndex,seed,verbose);
          rsa.SetCaptureMode(captureOn,"../../drivers/vectors","rsa");
          rsa.SetExpErr(1);

          maxLoop = 2;
          errCnt += rsa.RunRsaTest(maxLoop,8);

          rsa.freeMe();

          CHECK4ERR(errCnt,"rsa");
        }
        break;

      } // switch (cpuId)
      if (errCnt) return errCnt;
    } // if isOn(coreIndex)
  } // for (coreIndex)

  srot.freeMe();

// else do nothing
#endif
  return errCnt;
}
