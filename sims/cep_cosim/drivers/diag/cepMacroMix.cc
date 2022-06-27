//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
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
#include "portable_io.h"

#include <math.h>

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
#define CHECK4ERR(e,name) do {  \
  if (!errCnt) {                \
    if (srot.GetVerbose()) {    \
        LOGE("ERROR: test=%s No error detected.. bad!!!\n",name); \
    } \
    errCnt++; \
    return errCnt; \
  } else { \
    if (srot.GetVerbose()) { \
        LOGI("test=%s: Error detected as expected.. Good!!!\n",name); \
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
  
  int captureOn = 0;

  #ifdef TL_CAPTURE
    captureOn = TL_CAPTURE;
  #endif

  int maxLoop;
  if (VERBOSE1()) {
    LOGI("%s: cpuId=%d, cpuActiveMask=0x%08x coreMask=0x%08x, seed=0x%08x\n",__FUNCTION__, cpuId, cpuActiveMask, coreMask, seed);
  }

  // Instantiate and initialize an SRoT object
  cep_srot srot(SROT_INDEX, CEP_VERSION_REG_INDEX, verbose);
  srot.SetCpuActiveMask(cpuActiveMask);
  srot.SetCoreMask(coreMask);
  
  // Capture mode should only be called by Core 0 (which is doing all the SRoT operations)
  if (cpuId == 0) {srot.SetCaptureMode(captureOn, VECTOR_D, "SROT");}

  // Initialize the LLKI (One a single core will perform this function)
  errCnt += srot.LLKI_Setup(cpuId); 
  if (errCnt) return errCnt;

  // The SRoT is no longer needed for this test.
  srot.freeMe();

  // Loop through all of the LLKI-enabled cores
  for (int coreIndex = 0; coreIndex < CEP_LLKI_CORES; coreIndex++) {
    cep_core_info_t core = cep_core_info[coreIndex];
    core_type_t type     = core.type;

    if (IS_ON(coreIndex) && core.enabled && cpuId == core.preferred_cpuId) {

      switch (type) {
      
        case AES_CORE: {
          cep_aes aes(coreIndex,seed,verbose);
          aes.SetCaptureMode(captureOn,VECTOR_D, core.name);
          errCnt += aes.RunAes192Test(core.maxLoop);
          aes.freeMe();
          break;
        }
        case DES3_CORE: {
          cep_des3 des3(coreIndex,seed,verbose);
          des3.SetCaptureMode(captureOn,VECTOR_D, core.name);
          errCnt += des3.RunDes3Test(core.maxLoop);
          des3.freeMe();
          break;
        }
        case MD5_CORE: {
          cep_md5 md5(coreIndex,seed,verbose);
          md5.SetCaptureMode(captureOn,VECTOR_D, core.name);
          errCnt += md5.RunMd5Test(core.maxLoop);
          md5.freeMe();
          break;
        }
        case FIR_CORE: {
          cep_fir fir(coreIndex,seed,verbose);
          fir.SetCaptureMode(captureOn,VECTOR_D, core.name);
          errCnt += fir.RunFirTest(core.maxLoop);
          fir.freeMe();
          break;
        }
        case IIR_CORE: {
          cep_iir iir(coreIndex,seed,verbose);
          iir.SetCaptureMode(captureOn,VECTOR_D, core.name);
          errCnt += iir.RunIirTest(core.maxLoop);
          iir.freeMe();
          break;
        }
        case SHA256_CORE: {
          cep_sha256 sha256(coreIndex,seed,verbose);
          sha256.SetCaptureMode(captureOn,VECTOR_D, core.name);
          errCnt += sha256.RunSha256Test(core.maxLoop);
          sha256.freeMe();
          break;
        }
        case GPS_CORE: {
          cep_gps gps(coreIndex, seed, verbose);
          gps.SetCaptureMode(captureOn, VECTOR_D, core.name);
          errCnt += gps.RunGpsTest(core.maxLoop);
          gps.freeMe();
          break;
        }
        case GPS_STATIC_CORE: {
          cep_gps gps(coreIndex,seed, 1, verbose);  // PCode initialization is static
          gps.SetCaptureMode(captureOn,VECTOR_D, core.name);
          errCnt += gps.RunGpsTest(core.maxLoop);
          gps.freeMe();
          break;
        }
        case DFT_CORE: {
          cep_dft dft(coreIndex,seed,verbose);
          dft.SetCaptureMode(captureOn,VECTOR_D, core.name);
          errCnt += dft.RunDftTest(core.maxLoop);
          dft.freeMe();
          break;
        }
        case IDFT_CORE: {
          cep_idft idft(coreIndex,seed,verbose);
          idft.SetCaptureMode(captureOn,VECTOR_D, core.name);
          errCnt += idft.RunIdftTest(core.maxLoop);
          idft.freeMe();
          break;
        }
        case RSA_CORE: {
          cep_rsa rsa(coreIndex,seed,verbose);
          rsa.SetCaptureMode(captureOn,VECTOR_D, core.name);
          errCnt += rsa.RunRsaTest(core.maxLoop, 8);
          rsa.freeMe();
          break;
        }
      } // switch (cpuId)
    } // if (IS_ON(coreIndex) && core.enabled && cpuId == core.preferred_cpuId)
  } // for (coreIndex)

// else do nothing
#endif // #ifndef BARE_MODE
  
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
  srot.SetCoreMask(coreMask);
  srot.LLKI_invertKey(1);     // all test should fails!!

  // Initialize the LLKI (One a single core will perform this function)
  errCnt += srot.LLKI_Setup(cpuId);
  if (errCnt) return errCnt;

  // Loop through all of the LLKI-enabled cores
  for (int coreIndex = 0; coreIndex < CEP_LLKI_CORES; coreIndex++) {
    cep_core_info_t core = cep_core_info[coreIndex];
    core_type_t type     = core.type;

    if (IS_ON(coreIndex) && core.enabled && cpuId == core.preferred_cpuId) {

      switch (type) {
      
        case AES_CORE: {
          cep_aes aes(coreIndex,seed,verbose);
          errCnt += aes.RunAes192Test(core.maxLoop);
          aes.freeMe();
          CHECK4ERR(errCnt, core.name);
          break;
        }
        case DES3_CORE: {
          cep_des3 des3(coreIndex,seed,verbose);
          errCnt += des3.RunDes3Test(core.maxLoop);
          des3.freeMe();
          CHECK4ERR(errCnt, core.name);
          break;
        }
        case MD5_CORE: {
          cep_md5 md5(coreIndex,seed,verbose);
          errCnt += md5.RunMd5Test(core.maxLoop);
          md5.freeMe();
          CHECK4ERR(errCnt, core.name);
          break;
        }
        case FIR_CORE: {
          cep_fir fir(coreIndex,seed,verbose);
          errCnt += fir.RunFirTest(core.maxLoop);
          fir.freeMe();
          CHECK4ERR(errCnt, core.name);
          break;
        }
        case IIR_CORE: {
          cep_iir iir(coreIndex,seed,verbose);
          errCnt += iir.RunIirTest(core.maxLoop);
          iir.freeMe();
          CHECK4ERR(errCnt, core.name);
          break;
        }
        case SHA256_CORE: {
          cep_sha256 sha256(coreIndex,seed,verbose);
          errCnt += sha256.RunSha256Test(core.maxLoop);
          sha256.freeMe();
          CHECK4ERR(errCnt, core.name);
          break;
        }
        case GPS_CORE: {
          cep_gps gps(coreIndex,seed,verbose);
          errCnt += gps.RunGpsTest(core.maxLoop);
          gps.freeMe();
          CHECK4ERR(errCnt, core.name);
          break;
        }
        case DFT_CORE: {
          cep_dft dft(coreIndex,seed,verbose);
          errCnt += dft.RunDftTest(core.maxLoop);
          dft.freeMe();
          CHECK4ERR(errCnt, core.name);
          break;
        }
        case IDFT_CORE: {
          cep_idft idft(coreIndex,seed,verbose);
          errCnt += idft.RunIdftTest(core.maxLoop);
          idft.freeMe();
          CHECK4ERR(errCnt, core.name);
          break;
        }
        case RSA_CORE: {
          cep_rsa rsa(coreIndex,seed,verbose);
          errCnt += rsa.RunRsaTest(core.maxLoop, 8);
          rsa.freeMe();
          CHECK4ERR(errCnt, core.name);
          break;
        }
      } // switch (cpuId)
    } // if (IS_ON(coreIndex) && core.enabled && cpuId == core.preferred_cpuId)
  } // for (coreIndex)

srot.freeMe();

// else do nothing
#endif // #ifndef BARE_MODE

  return errCnt;
}
