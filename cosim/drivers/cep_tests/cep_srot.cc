//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cep_aes.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    aes test for CEP
// Notes:          
//************************************************************************

#if defined(BARE_MODE)
#else
//#include <openssl/rand.h>
//#include <openssl/ecdsa.h>
//#include <openssl/obj_mac.h>
//#include <openssl/err.h>
//#include <openssl/pem.h>
#include <openssl/evp.h>
//#include <openssl/hmac.h>

#include "simPio.h"

#endif

#include <string.h>
#include "cep_srot.h"
#include "simdiag_global.h"
#include "portable_io.h"
#include "CEP.h"
#include "cepregression.h"
#include "random48.h"
#include "cep_adrMap.h"
#include "cepregression.h"

cep_srot::cep_srot(int verbose) {
  init();
  maxTO = 5000;
  // Set the verbosity
  SetVerbose(verbose);
  SetCpuActiveMask(0xf);
  //
  init_srot();
  init_cepregs();
<<<<<<< HEAD
  mInvertKey = 0;
  mInvertMask = 0x5555555555555555LLU;
=======
>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0

}

// ------------------------------------------------------------------------------------------------------------
// This method will initialized the Key Index RAM
// ------------------------------------------------------------------------------------------------------------
int cep_srot::InitKeyIndexRAM (void)
{

  int errCnt = 0;

  if (GetVerbose()) {
    LOGI("\n");
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("%s: Initializing the KEY Index RAM...                      \n",__FUNCTION__);
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("\n");
  }
  for (int i = 0; i < SROT_KEYINDEXRAM_SIZE; i++) {
    cep_writeNcapture(SROT_BASE_K, SROT_KEYINDEXRAM_ADDR + (i*8), 0);
  }

  return errCnt;

} // cep_srot::InitKeyIndexRAM
// ------------------------------------------------------------------------------------------------------------



// ------------------------------------------------------------------------------------------------------------
// This method will load the specified Key Index and Key into the appropriate RAMs in the SRoT
// ------------------------------------------------------------------------------------------------------------
// Parameter list:
//    KeyIndex    - Selected Index into the Key Index RAM
//    CoreIndex   - Which core is this key for? The XXXX_BASE_K constants in CEP.h should be used
//    LowPointer  - The location of the first key word in the KeyRAM
//    HighPointer - The location of the last key word in the KeyRAM
//    Key         - Contains the actual key bits (length = High_Pointer - Low_Pointer + 1)
<<<<<<< HEAD
//    InvertType  - INVERT_ALTERNATE or INVERT_ALL_BITS: for negative testing (NA if mInvertKey==0)..Default = 0..
//
int cep_srot::LoadLLKIKey (uint8_t KeyIndex, uint8_t CoreIndex, uint16_t LowPointer, uint16_t HighPointer, const uint64_t *Key, int invertType)
=======
int cep_srot::LoadLLKIKey (uint8_t KeyIndex, uint8_t CoreIndex, uint16_t LowPointer, uint16_t HighPointer, const uint64_t *Key)
>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0
{

  int errCnt = 0;

  // Load the Key material into the Key RAM
  if (GetVerbose()) {
    LOGI("\n");
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("%s: Loading a key into the Key RAM...                      \n",__FUNCTION__);
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("\n");
  }
<<<<<<< HEAD
  if (invertType == INVERT_ALTERNATE) {
    mInvertMask = 0x5555555555555555LLU;
  } else {
    mInvertMask = 0xFFFFFFFFFFFFFFFFLLU;
  }
  for (int i = 0; i < (HighPointer - LowPointer + 1); i++) {
    // not invert all the bits, just the odd bits, IIR will give the same results if flip all bits
    cep_writeNcapture(SROT_BASE_K, SROT_KEYRAM_ADDR + LowPointer*8 + (i*8), mInvertKey ? (Key[i] ^ mInvertMask) : Key[i]);
=======

  for (int i = 0; i < (HighPointer - LowPointer + 1); i++) {
    cep_writeNcapture(SROT_BASE_K, SROT_KEYRAM_ADDR + LowPointer*8 + (i*8), Key[i]);
>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0
  }
  
  // Load a valid key index
  if (GetVerbose()) {
    LOGI("\n");
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("%s: Loading a valid key index...                        \n",__FUNCTION__);
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("\n");
  }
  cep_writeNcapture(SROT_BASE_K, SROT_KEYINDEXRAM_ADDR + KeyIndex*8, key_index_pack(
                      LowPointer,     // low pointer
                      HighPointer,    // high pointer
                      CoreIndex,      // core index
                      0x1));          // Valid

  return errCnt;


} // cep_srot::LoadLLKIKey
// ------------------------------------------------------------------------------------------------------------



// ------------------------------------------------------------------------------------------------------------
// This method will set the two operation mode bits
// in the Surrogate Root of Trust, thus preventing any
// further RISC-V access to Key Index and Key Index RAMs
// ------------------------------------------------------------------------------------------------------------
int cep_srot::SetOperationalMode (void)
{

  int errCnt = 0;

  if (GetVerbose()) {
    LOGI("\n");
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("%s: Switching the LLKI from DEBUG to OPERATIONAL mode...\n",__FUNCTION__);
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("\n");
  }
  cep_writeNcapture(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_MODEBIT0_MASK | SROT_CTRLSTS_MODEBIT1_MASK);

  return errCnt;

} // cep_srot::SetOperationalMode
// ------------------------------------------------------------------------------------------------------------



// ------------------------------------------------------------------------------------------------------------
// The following method will send a Load Key Request to the SRoT with the specified key index
// ------------------------------------------------------------------------------------------------------------
int cep_srot::EnableLLKI (uint8_t KeyIndex)
{

  int errCnt = 0;

  if (GetVerbose()) {
    LOGI("\n");
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("%s: Issuing a Load Key Request to the SRoT and waiting  \n",__FUNCTION__);
    LOGI("%s: for the response...                                 \n",__FUNCTION__);
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("\n");
  }
  // Write a word to the LLKI C2 Send FIFO
  cep_writeNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                    LLKI_MID_C2LOADKEYREQ,  // Message ID
                    0x00,                   // Status (unused)
                    0x01,                   // Message Length
                    KeyIndex,               // Key Index
                    0xDEADBEEF)); 
  
  // Poll the response waiting bit
  cep_readNspin(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, 100);
  
  // Read the response
  int response_status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
  
  if (parse_message_status(response_status, GetVerbose())) {
    errCnt++;
  }

  return errCnt;

} // cep_srot::EnableLLKI
// ------------------------------------------------------------------------------------------------------------



// ------------------------------------------------------------------------------------------------------------
// The following method will send a Clear Key Request to the SRoT with the specified key index
// ------------------------------------------------------------------------------------------------------------
int cep_srot::DisableLLKI (uint8_t KeyIndex)
{

  int errCnt = 0;

  if (GetVerbose()) {
    LOGI("\n");
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("%s: Issuing a Clear Key Request to the SRoT and waiting  \n",__FUNCTION__);
    LOGI("%s: for the response...                                  \n",__FUNCTION__);
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("\n");
  }
  // Write a word to the LLKI C2 Send FIFO
  cep_writeNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                    LLKI_MID_C2CLEARKEYREQ, // Message ID
                    0x00,                   // Status (unused)
                    0x01,                   // Message Length
                    KeyIndex,               // Key Index
                    0xDEADBEEF)); 
  
  // Poll the response waiting bit
  cep_readNspin(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, 100);
  
  // Read the response
  int response_status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
  
  if (parse_message_status(response_status, GetVerbose())) {
    errCnt++;
  }

  return errCnt;

}
// ------------------------------------------------------------------------------------------------------------




// ------------------------------------------------------------------------------------------------------------
// The following method will set a bit in the cep_core0_status register in order to indicate to the other
// cores that the LLKI operations have been completed.
//
// For core0, the method will set the appropriate bit in the cep_core0_status register
// For the other cores, this function will poll the status bit continuously until it is set
//
// ------------------------------------------------------------------------------------------------------------
int cep_srot::LLKIInitComplete (int cpuId)
{

  int errCnt  = 0;
  int maxTO   = 50000; // Maximum number of "counts" to try before continuing

  
  // If I am core 0
  if (cpuId == 0) {
    
    if (GetVerbose()) {
      LOGI("\n");
      LOGI("----------------------------------------------------------------------------\n");
      LOGI("%s: Core 0 setting interlock bit...                                         \n",__FUNCTION__);
      LOGI("----------------------------------------------------------------------------\n");
      LOGI("\n");
    }
    
    // Write to the core0 status register indicating that the LLKI operations are complete
      cep_writeNcapture(CEP_VERSION_REG_K, cep_core0_status, 0x0000000000000001);
      
  } else {
    
    if (GetVerbose()) {
      LOGI("\n");
      LOGI("----------------------------------------------------------------------------\n");
      LOGI("%s: Core %d waiting for interlock bit...                                    \n", __FUNCTION__, cpuId);
      LOGI("----------------------------------------------------------------------------\n");
      LOGI("\n");
    }
    
    // Wait for the interlock bit to be set (and capture if a timeout occurs)
    errCnt += cep_readNspin(CEP_VERSION_REG_K, cep_core0_status, 0x0000000000000001, 0x0000000000000001, maxTO); 
    
    if (GetVerbose()) {
      LOGI("\n");
      LOGI("----------------------------------------------------------------------------\n");
      LOGI("%s: Core %d interlock bit received...                                       \n", __FUNCTION__, cpuId);
      LOGI("----------------------------------------------------------------------------\n");
      LOGI("\n");
    }
  } // else if (cpuId == 0)
  return errCnt;

} // cep_srot::LLKIInitComplete
// ------------------------------------------------------------------------------------------------------------




// ------------------------------------------------------------------------------------------------------------
// Simple method that will clear the cep_core0_status register if this is cpuId 0
// ------------------------------------------------------------------------------------------------------------
int cep_srot::LLKIClearComplete (int cpuId)
{

  int errCnt = 0;

    // If I am core 0
    if (cpuId == 0) {

      if (GetVerbose()) {
        LOGI("\n");
        LOGI("----------------------------------------------------------------------------\n");
        LOGI("%s: Core 0 clearing interlock bit...                                        \n",__FUNCTION__);
        LOGI("----------------------------------------------------------------------------\n");
        LOGI("\n");
      }

      // Write to the core0 status register indicating that the LLKI operations are complete
      cep_writeNcapture(CEP_VERSION_REG_K, cep_core0_status, 0);

      return errCnt;

    } else {

      return errCnt;

    } // else if (cpuId == 0)


} // cep_srot::LLKIInitComplete
// ------------------------------------------------------------------------------------------------------------


// This should only be the public method the test will see
int cep_srot::LLKI_Setup(int cpuId) {
  int errCnt = 0;
<<<<<<< HEAD

  // pick the first LSB in cpuActiveMask as master
  //
=======
  // pick the first LSB in cpuActiveMask as master
>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0
  int iAMmaster = 0;
  for (int i=0;i<4;i++) {
    if (((1 << i) & GetCpuActiveMask())) { // found the first LSB=1 bit
      if (i == cpuId) {
	iAMmaster = 1;
      }
      break;
    }
  }
  //
  // Master stuffs
  //
  if (iAMmaster) {
<<<<<<< HEAD
#ifdef SIM_ENV_ONLY
    if (mSrotFlag) {
      LOGI("%s: cpu#%d: enable SROT vectore capture!!!\n",__FUNCTION__,cpuId);
      DUT_WRITE_DVT(DVTF_SROT_START_CAPTURE_BIT, DVTF_SROT_START_CAPTURE_BIT , 1);
    }
#endif

=======
>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0
    if (GetVerbose()) {
      LOGI("%s: cpu#%d is the master. mask=0x%x!!!\n",__FUNCTION__,cpuId,GetCpuActiveMask());
    }
    //
    // Initiate the LLKI for all cores
    // For multi-core tests, core0 will be responsible for the following:
    // - Initializing the LLKI Key Index RAM for ALL KEYS
    // - Loading all keys into the SRoT
    // - Enabling the LLKI for all cores
    
    //
    // Initialize the LLKI
    //
    InitKeyIndexRAM();
    
    // Load the all the keys (Core indicies MUST correspond
    // to those identified in the LLKI_CORE_INDEX_ARRAY as
    // listed in llki_pkg.sv
<<<<<<< HEAD
    // tony duong 02/26/21:
    /* for the sake of easy  testing, use keyIndex same as core Index for now */

    LoadLLKIKey(AES_BASE_K     , AES_BASE_K,     0,  1, AES_MOCK_TSS_KEY,    INVERT_ALL_BITS);
    LoadLLKIKey(DES3_BASE_K    , DES3_BASE_K,    2,  2, DES3_MOCK_TSS_KEY,   INVERT_ALL_BITS);
    LoadLLKIKey(DFT_BASE_K     , DFT_BASE_K,     3,  3, DFT_MOCK_TSS_KEY,    INVERT_ALL_BITS);
    LoadLLKIKey(FIR_BASE_K     , FIR_BASE_K,     4,  4, FIR_MOCK_TSS_KEY,    INVERT_ALTERNATE);
    LoadLLKIKey(GPS_BASE_K     , GPS_BASE_K,     5,  9, GPS_MOCK_TSS_KEY,    INVERT_ALL_BITS);
    LoadLLKIKey(IDFT_BASE_K    , IDFT_BASE_K,   10, 10, IDFT_MOCK_TSS_KEY,   INVERT_ALL_BITS);
    LoadLLKIKey(IIR_BASE_K     , IIR_BASE_K,    11, 11, IIR_MOCK_TSS_KEY,    INVERT_ALTERNATE); // can't use invert_all_bits
    LoadLLKIKey(MD5_BASE_K     , MD5_BASE_K,    12, 19, MD5_MOCK_TSS_KEY,    INVERT_ALL_BITS);
    LoadLLKIKey(RSA_BASE_K     , RSA_BASE_K,    20, 20, RSA_MOCK_TSS_KEY,    INVERT_ALL_BITS);
    LoadLLKIKey(SHA256_BASE_K  , SHA256_BASE_K, 21, 28, SHA256_MOCK_TSS_KEY, INVERT_ALL_BITS);
    
    // Enable the LLKI for all the cores
    EnableLLKI(AES_BASE_K    );
    EnableLLKI(DES3_BASE_K   );
    EnableLLKI(DFT_BASE_K    );
    EnableLLKI(FIR_BASE_K    );
    EnableLLKI(GPS_BASE_K    );
    EnableLLKI(IDFT_BASE_K   );
    EnableLLKI(IIR_BASE_K    );
    EnableLLKI(MD5_BASE_K    );
    EnableLLKI(RSA_BASE_K    );
    EnableLLKI(SHA256_BASE_K );

    // Write to the core0 status register indicating that the LLKI operations are complete
    cep_writeNcapture(CEP_VERSION_REG_K, cep_core0_status, 0x0000000000000001);
    //
    // Done!!
    //
#ifdef SIM_ENV_ONLY
    if (mSrotFlag) {
      DUT_WRITE_DVT(DVTF_SROT_STOP_CAPTURE_BIT, DVTF_SROT_STOP_CAPTURE_BIT , 1);
    }
#endif
  
=======
    LoadLLKIKey(0, AES_BASE_K, 0, 1, AES_MOCK_TSS_KEY);
    LoadLLKIKey(1, DES3_BASE_K, 2, 2, DES3_MOCK_TSS_KEY);
    LoadLLKIKey(2, DFT_BASE_K, 3, 3, DFT_MOCK_TSS_KEY);
    LoadLLKIKey(3, FIR_BASE_K, 4, 4, FIR_MOCK_TSS_KEY);
    LoadLLKIKey(4, GPS_BASE_K, 5, 9, GPS_MOCK_TSS_KEY);
    LoadLLKIKey(5, IDFT_BASE_K, 10, 10, IDFT_MOCK_TSS_KEY);
    LoadLLKIKey(6, IIR_BASE_K, 11, 11, IIR_MOCK_TSS_KEY);
    LoadLLKIKey(7, MD5_BASE_K, 12, 19, MD5_MOCK_TSS_KEY);
    LoadLLKIKey(8, RSA_BASE_K, 20, 20, RSA_MOCK_TSS_KEY);
    LoadLLKIKey(9, SHA256_BASE_K, 21, 28, SHA256_MOCK_TSS_KEY);
    
    // Enable the LLKI for all the cores
    EnableLLKI(0);
    EnableLLKI(1);
    EnableLLKI(2);
    EnableLLKI(3);
    EnableLLKI(4);
    EnableLLKI(5);
    EnableLLKI(6);
    EnableLLKI(7);
    EnableLLKI(8);
    EnableLLKI(9);

    // Write to the core0 status register indicating that the LLKI operations are complete
    cep_writeNcapture(CEP_VERSION_REG_K, cep_core0_status, 0x0000000000000001);

>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0
  } else {
    if (GetVerbose()) {
      LOGI("%s: cpu#%d will be the slave. mask=0x%x !!!\n",__FUNCTION__,cpuId,GetCpuActiveMask());
    }
    errCnt += cep_readNspin(CEP_VERSION_REG_K, cep_core0_status, 0x0000000000000001, 0x0000000000000001, maxTO); 
  }
  //
  return errCnt;
<<<<<<< HEAD
=======

>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0
}
