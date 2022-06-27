//************************************************************************
// Copyright 2022 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepMultiThread.h"
#include "cep_apis.h"
#include "portable_io.h"

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

//
// 1. Allocate N number of words in Cache, each word associates with 1 unique macro test (aes, des3,...)
// 2. All are intially cleared to zero indicate the macro test is not running by any one...
// 3. M number of threads will be spawn and allow to float, thread's ID must be != 0, each will try to 
//    run all macro tests one-by-one 
//    after it get the lock via LRSC on one of those above.
//

//#define USE_ATOMIC
#define IS_ON(c) ((1 << c) & coreMask)

// return test# (0 - N) , thrId MUST != 0, return -1 if fails
int cepMultiThread_findATest2Run(int thrId, uint64_t testLockPtr, int doneMask, int maxTest, int verbose) {
#ifndef BARE_MODE    
  int maxLoop = 2000;
  while (maxLoop > 0) {
    for (int i = 0; i < maxTest; i++) {
      if ((1 << i) & doneMask) {
#ifndef USE_ATOMIC
        uint64_t dat64;
        int lockStat = cep_get_lock(thrId, 0, 1000); // get lock#0
        if (lockStat == -1) return -1; // fail  
        DUT_READ32_64(testLockPtr + (i*8),dat64);
        if (dat64 == 0) {
          DUT_WRITE32_64(testLockPtr + (i*8),(1 << 16) | thrId);
          cep_release_lock(thrId, 0);
          return i;
        } else {
          cep_release_lock(thrId, 0);             
        }
#else
	// check if no other test running it..
	uint64_t myCurSig = 0; // expect 0
        uint64_t *ptr = (uint64_t *)(testLockPtr + (i*8)); // for this case testLockPtr = virtual address
	if (__atomic_compare_exchange_n(ptr,&myCurSig,(uint64_t)(thrId+1),0,__ATOMIC_ACQ_REL,__ATOMIC_ACQ_REL)) { 
          return i; // the testId to run	
	}
#endif
      }        
    }
    USEC_SLEEP(5000); // and try again..
    maxLoop--;
  }
#endif
  return -1;
}


int cepMultiThread_releaseTestLock(int thrId, int testId,uint64_t testLockPtr) {
  int errCnt = 0;
#ifndef BARE_MODE    
  uint64_t myCurSig = thrId+1; // expect 0
  uint64_t zeros = 0; // put back 0 to release
#ifndef USE_ATOMIC
  DUT_WRITE32_64(testLockPtr + (testId*8), 0);
#else
  uint64_t *ptr = (uint64_t *)(testLockPtr + (testId*8)); // for this case testLockPtr = virtual address    
  if (__atomic_compare_exchange_n(ptr,&myCurSig,zeros,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQ_REL)) { // next
    if (VERBOSE1()) {
      LOGI("OK: Lock is released. testId=%d thrId=0x%x, saw =0x%016lx\n",testId,thrId,myCurSig);            
    }
  } else {
    LOGE("ERROR: Can't release the lock testId=%d thrId=0x%x, saw =0x%016lx\n",testId,thrId,myCurSig);
    errCnt++;
  }
#endif  
#endif
  return errCnt;
}

//
// SRoT
//
int cepMultiThread_setup(int cpuId, uint64_t testLockPtr, int coreMask, int maxTest, int cpuActiveMask, int verbose) {
  
  int errCnt = 0;
#ifndef BARE_MODE    

  // Instantiate and initialize an SRoT object
  cep_srot srot(SROT_INDEX, CEP_VERSION_REG_INDEX, verbose);
  srot.SetCpuActiveMask(cpuActiveMask);
  srot.SetCoreMask(coreMask);

  // Initialize the LLKI (One a single core will perform this function)
  errCnt += srot.LLKI_Setup(cpuId);
  if (errCnt) return errCnt;

  // The SRoT is no longer needed for this test.
  srot.freeMe();

#ifndef USE_ATOMIC    
  // Initialize memory space for test status to zero
  for (int i = 0; i < maxTest; i++) { 
    DUT_WRITE32_64(testLockPtr + (i*8), 0);   
  }
#endif

#endif
  if (verbose) {
    LOGI("%s cpuId=%d ErrCnt=%d\n",__FUNCTION__,cpuId,errCnt);
  }
  return errCnt;
}


// Run Multi-thread
int cepMultiThread_runThr(int thrId, uint64_t testLockPtr, int coreMask, int maxTest, int maxLoop, int seed, int verbose) {
  int errCnt = 0;
  if (verbose) {
    LOGI("%s : thrId=%d\n",__FUNCTION__,thrId);
  }
    
#ifndef BARE_MODE    
  int myDoneMask = 0;
  
  //while ((myDoneMask != coreMask) && (errCnt == 0)) {
  while ((myDoneMask != (coreMask & 0xFFFF)) && (errCnt == 0)) {
    int coreIndex = cepMultiThread_findATest2Run(thrId, testLockPtr, coreMask & ~myDoneMask, maxTest, verbose);
    if (coreIndex == -1) {
      LOGE("%s: can't find a test to run thrId=%d myDoneMask=0x%x\n",__FUNCTION__,thrId,myDoneMask);
      return 1;
    }
        
    myDoneMask |= (1 << coreIndex);
    
    if (verbose) {
      LOGI("===== RUNNING thrId=%d test=%d doneMask=0x%x\n",thrId,coreIndex,myDoneMask);
    }

    cep_core_info_t core = cep_core_info[coreIndex];
    core_type_t type     = core.type;

    if (IS_ON(coreIndex) && core.enabled && thrId == core.preferred_cpuId) {
      //if ( IS_ON(coreIndex) && core.enabled ) {
      switch (type) {
       
        case AES_CORE: {
          cep_aes aes(coreIndex,seed,verbose);
          errCnt += aes.RunAes192Test(core.maxLoop);
          aes.freeMe();
          break;
        }
   
        case DES3_CORE: {
          cep_des3 des3(coreIndex,seed,verbose);
          errCnt += des3.RunDes3Test(core.maxLoop);
          des3.freeMe();
          break;
        }
            
        case MD5_CORE: {                
          cep_md5 md5(coreIndex,seed,verbose);
           errCnt += md5.RunMd5Test(core.maxLoop);
          md5.freeMe();
          break;
        }
            
        case FIR_CORE: {                
          cep_fir fir(coreIndex,seed,verbose);
          errCnt += fir.RunFirTest(core.maxLoop);
          fir.freeMe();
          break;
        }
            
        case IIR_CORE: {        
          cep_iir iir(coreIndex,seed,verbose);
          errCnt += iir.RunIirTest(core.maxLoop);
          iir.freeMe();
          break;
        }
            
        case SHA256_CORE: {         
          cep_sha256 sha256(coreIndex,seed,verbose);
          errCnt += sha256.RunSha256Test(core.maxLoop);
          sha256.freeMe();
          break;
        }
            
        case GPS_CORE: {         
          cep_gps gps(coreIndex,seed,verbose);   
          errCnt += gps.RunGpsTest(core.maxLoop);
          gps.freeMe();
          break;
        }

        case GPS_STATIC_CORE: {         
          cep_gps gps(coreIndex,seed,1,verbose);   
          errCnt += gps.RunGpsTest(core.maxLoop);
          gps.freeMe();
          break;
        }
          
        case DFT_CORE: {
          cep_dft dft(coreIndex,seed,verbose);
          errCnt += dft.RunDftTest(maxLoop);
          dft.freeMe();
          break;
        }

        case IDFT_CORE: {
          cep_idft idft(coreIndex,seed,verbose);
          errCnt += idft.RunIdftTest(maxLoop);
          idft.freeMe();
          break;
        }

        case RSA_CORE: {
          cep_rsa rsa(coreIndex,seed,verbose);
          int maxBytes=8;
          if (!errCnt)  {    
            errCnt += rsa.RunRsaMemTest(0xf, 1024); // 8K bits
          }
          if (!errCnt)  {    
            errCnt += rsa.RunRsaTest(maxLoop,maxBytes);
          }
          rsa.freeMe();
          break;
        }
      } // switch (type)
    } // if ( IS_ON(coreIndex) && core.enabled ) 
    errCnt += cepMultiThread_releaseTestLock(thrId, coreIndex, testLockPtr);
  } // while ((myDoneMask != coreMask) && (errCnt == 0))
#endif
  return errCnt;
}

