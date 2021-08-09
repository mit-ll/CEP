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
#include "cepMultiThread.h"

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

//
// 1. Allocate N number of words in Cache, each word associates with 1 unique macro test (aes, des3,...)
// 2. All are intially cleared to zero indicate the macro test is not running by any one...
// 3. M number of threads will be spawn and allow to float, thread's ID must be != 0, each will try to to run all macro tests one-by-one 
//    after it get the lock via LRSC on one of those above.
//

//#define USE_ATOMIC

//
// return test# (0 - N) , thrId MUST != 0, return -1 if fails
//
int cepMultiThread_findATest2Run(int thrId, uint64_t testLockPtr, int doneMask, int maxTest, int verbose) 
{
  //
#ifndef BARE_MODE    
  int maxLoop = 2000;
  while (maxLoop > 0) {
  for (int i=0;i<maxTest;i++) {
    //
    if ((1<<i) & doneMask) {
#ifndef USE_ATOMIC
        uint64_t dat64;
        int lockStat = cep_get_lock(thrId, 0, 100); // get lock#0
        if (lockStat == -1) return -1; // fail
            // check the testLockPtr
        DUT_READ32_64(testLockPtr + (i*8),dat64);
        if (dat64 == 0) {
            // put my signature in there
            //testLockPtr[i] = (1 << 16) | thrId;
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
	if (__atomic_compare_exchange_n(ptr,&myCurSig,(uint64_t)(thrId+1),0,__ATOMIC_ACQ_REL,__ATOMIC_ACQ_REL)) { // next 
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


int cepMultiThread_releaseTestLock(int thrId, int testId,uint64_t testLockPtr)
{
    int errCnt = 0;
#ifndef BARE_MODE    
        //
    uint64_t myCurSig = thrId+1; // expect 0
    uint64_t zeros = 0; // put back 0 to release
#ifndef USE_ATOMIC
        //testLockPtr[testId] = 0;
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
//    
#endif
    return errCnt;
}

//
// SRoT
//
int cepMultiThread_setup(int cpuId, uint64_t testLockPtr,int maxTest, int coreMask, int verbose) 
{
        //
    int errCnt =0;
#ifndef BARE_MODE    
        //
#ifndef USE_ATOMIC    
    for (int i=0;i<maxTest;i++) {
        //testLockPtr[i] = 0;
        DUT_WRITE32_64(testLockPtr + (i*8), 0);    
    }
#endif
    cep_srot srot(verbose);
    srot.SetCpuActiveMask(coreMask);
    errCnt += srot.LLKI_Setup(cpuId);
    srot.freeMe();
#endif
    if (verbose) {
        LOGI("%s cpuId=%d ErrCnt=%d\n",__FUNCTION__,cpuId,errCnt);
    }
    return errCnt;
}

//
// Run Multi-thread
//
int cepMultiThread_runThr(int thrId, uint64_t testLockPtr, int cryptoMask, int maxTest, int maxLoop, int seed, int verbose)
{
  //
    int errCnt = 0;
    if (verbose) {
        LOGI("%s : thrId=%d\n",__FUNCTION__,thrId);
    }
    
#ifndef BARE_MODE    
    int myDoneMask = 0;
        //
    while ((myDoneMask != cryptoMask) && (errCnt == 0)) {
            //
        int testId = cepMultiThread_findATest2Run(thrId, testLockPtr, cryptoMask & ~myDoneMask, maxTest, verbose);
        if (testId == -1) {
            LOGE("%s: can't find a test to run thrId=%d myDoneMask=0x%x\n",__FUNCTION__,thrId,myDoneMask);
            return 1;
        }
        
            //
        myDoneMask |= (1 << testId);
            //
            //
        if (verbose) {
            LOGI("===== RUNNING test=%d thrId=%d doneMask=0x%x\n",testId,thrId,myDoneMask);
        }
        
        switch (testId) {
            case AES_BASE_K : 
            {
                    //
                    // AES
                    //    
                cep_aes aes(seed,verbose);
                aes.init();
                    //
                //maxLoop =  20;
                init_aes();
                errCnt += aes.RunAes192Test(maxLoop);
                    //
                aes.freeMe();
                break;
            }
        
                    //
                    // DES3
                    //
            case DES3_BASE_K : 
            {
                cep_des3 des3(seed,verbose);
                des3.init();      
                    //
                //maxLoop = 10;
                init_des3();
                errCnt += des3.RunDes3Test(maxLoop);
                    //
                des3.freeMe();
                break;
            }
            
            case MD5_BASE_K :
            {                
                cep_md5 md5(seed,verbose);
                md5.init();
                    //
                //maxLoop = 8;
                init_md5();
                errCnt += md5.RunMd5Test(maxLoop);
                    //
                md5.freeMe();
                break;
            }
            
                    //
                    // FIR
                    //
            case FIR_BASE_K :
            {                
                cep_fir fir(seed,verbose);
                fir.init();      
                    //
                //maxLoop = 5;
                init_fir();
                errCnt += fir.RunFirTest(maxLoop);
                    //
                fir.freeMe();
                break;
            }
            
                    //
                    // IIR
                    //
            case IIR_BASE_K:
            {
                
                cep_iir iir(seed,verbose);
                iir.init();
                    //
                //maxLoop = 5;
                init_iir();
                errCnt += iir.RunIirTest(maxLoop);
                    //
                iir.freeMe();
                break;
            }
            
                    //
                    // SHA256
                    //
            case SHA256_BASE_K:
            {
                
                cep_sha256 sha256(seed,verbose);
                sha256.init();
                    //
                //maxLoop = 8;
                init_sha256();
                errCnt += sha256.RunSha256Test(maxLoop);
                    //
                sha256.freeMe();
                break;
            }
            
                    //
                    // GPS
                    //
            case GPS_BASE_K:
            {
                
                cep_gps gps(seed,verbose);    // Note: The GPS constructor does write to the core
                gps.init();  
                    //
                //maxLoop  = 38;
                init_gps();
                errCnt += gps.RunGpsTest(maxLoop);
                    //
                gps.freeMe();
                break;
            }
            
                    //
                    // DFT & IDFT
                    //
            case DFT_BASE_K:
            {
                
                cep_dft dft(seed,verbose);
                dft.init();
                    //
                //maxLoop = 5;
                init_dft();
                init_idft();                
                errCnt += dft.RunDftTest(maxLoop);
                    //
                dft.freeMe();
                break;
            }
            
                    //
                    // RSA
                    //
            case RSA_BASE_K:
            {
                
                cep_rsa rsa(seed,verbose);
                rsa.init();
                    //
                //maxLoop = 4;
                int maxBytes=8;
                    //
                init_rsa();
                if (!errCnt)  {    
                    errCnt += rsa.RunRsaMemTest(0xf, 1024); // 8K bits
                }
                if (!errCnt)  {    
                    errCnt += rsa.RunRsaTest(maxLoop,maxBytes);
                }
                    //
                rsa.freeMe();
                break;
            }
            
        } // switch
        errCnt += cepMultiThread_releaseTestLock(thrId, testId,testLockPtr);
    } // while
#endif
    return errCnt;
}

