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
  
  // Default Timeout Parameter
  maxTO = 5000;
  
  // Set the verbosity
  SetVerbose(verbose);
  
  // Set the default CPU Active Mask
  SetCpuActiveMask(0xf);
  
  // Initialize some of the core datastructures
  init_srot();
  init_cepregs();
  mInvertKey = 0;
  mInvertMask = 0x5555555555555555LLU;

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
  for (int i = 0; i < (int)SROT_KEYINDEXRAM_SIZE; i++) {
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
//    InvertType  - INVERT_ALTERNATE or INVERT_ALL_BITS: for negative testing (NA if mInvertKey==0)..Default = 0..
//
int cep_srot::LoadLLKIKey (uint8_t KeyIndex, uint8_t CoreIndex, uint16_t LowPointer, uint16_t HighPointer, const uint64_t *Key, int invertType)
{

  int errCnt = 0;

  // Load the Key material into the Key RAM
  if (GetVerbose(2)) {
    LOGI("\n");
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("%s: Loading a key into the Key RAM...                      \n",__FUNCTION__);
    LOGI("----------------------------------------------------------------------------\n");
    LOGI("\n");
  }
  if (invertType == INVERT_ALTERNATE) {
    mInvertMask = 0x5555555555555555LLU;
  } else {
    mInvertMask = 0xFFFFFFFFFFFFFFFFLLU;
  }
  for (int i = 0; i < (HighPointer - LowPointer + 1); i++) {
    // not invert all the bits, just the odd bits, IIR will give the same results if flip all bits
    cep_writeNcapture(SROT_BASE_K, SROT_KEYRAM_ADDR + LowPointer*8 + (i*8), mInvertKey ? (Key[i] ^ mInvertMask) : Key[i]);
  }
  
  // Load a valid key index
  if (GetVerbose(2)) {
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

  if (GetVerbose(2)) {
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

  int 			errCnt = 0;
  uint8_t		status = 0;

  //
  if (GetVerbose(2)) {
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
  
  // Read and check the response
  status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
  CHECK_RESPONSE(status, LLKI_STATUS_GOOD, GetVerbose());

  // Return the error count
  return errCnt;

} // cep_srot::EnableLLKI
// ------------------------------------------------------------------------------------------------------------



// ------------------------------------------------------------------------------------------------------------
// The following method will send a Clear Key Request to the SRoT with the specified key index
// ------------------------------------------------------------------------------------------------------------
int cep_srot::DisableLLKI (uint8_t KeyIndex)
{

  int 			errCnt = 0;
  uint8_t		status = 0;

  if (GetVerbose(2)) {
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
  
  // Read and check the response
  status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
  CHECK_RESPONSE(status, LLKI_STATUS_GOOD, GetVerbose());

  // Return the error count
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


// ------------------------------------------------------------------------------------------------------------
// Public Method for setting up the LLKI
// ------------------------------------------------------------------------------------------------------------
int cep_srot::LLKI_Setup(int cpuId) {
  int errCnt = 0;

  // pick the first LSB in cpuActiveMask as master
  //
  int iAMmaster = 0;
  for (int i = 0; i < 4; i++) {
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

#ifdef SIM_ENV_ONLY
    if (mSrotFlag) {
      LOGI("%s: cpu#%d: enable SROT vector capture\n",__FUNCTION__,cpuId);
      DUT_WRITE_DVT(DVTF_SROT_START_CAPTURE_BIT, DVTF_SROT_START_CAPTURE_BIT , 1);
    }
#endif

    if (GetVerbose()) {
      LOGI("%s: cpu#%d is the master. mask=0x%x\n",__FUNCTION__,cpuId,GetCpuActiveMask());
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
    DisableLLKI(AES_BASE_K    );    EnableLLKI(AES_BASE_K    );
    DisableLLKI(DES3_BASE_K   );    EnableLLKI(DES3_BASE_K   );
    DisableLLKI(DFT_BASE_K    );    EnableLLKI(DFT_BASE_K    );
    DisableLLKI(FIR_BASE_K    );    EnableLLKI(FIR_BASE_K    );
    DisableLLKI(GPS_BASE_K    );    EnableLLKI(GPS_BASE_K    );
    DisableLLKI(IDFT_BASE_K   );    EnableLLKI(IDFT_BASE_K   );
    DisableLLKI(IIR_BASE_K    );    EnableLLKI(IIR_BASE_K    );
    DisableLLKI(MD5_BASE_K    );    EnableLLKI(MD5_BASE_K    );
    DisableLLKI(RSA_BASE_K    );    EnableLLKI(RSA_BASE_K    );
    DisableLLKI(SHA256_BASE_K );    EnableLLKI(SHA256_BASE_K );

    // Write to the core0 status register indicating that the LLKI operations are complete
    cep_writeNcapture(CEP_VERSION_REG_K, cep_core0_status, CEP_OK2RUN_SIGNATURE);
    //
    // Done!!
    //
#ifdef SIM_ENV_ONLY
    if (mSrotFlag) {
      DUT_WRITE_DVT(DVTF_SROT_STOP_CAPTURE_BIT, DVTF_SROT_STOP_CAPTURE_BIT , 1);
    }
#endif
  
  } else {
    if (GetVerbose()) {
      LOGI("%s: cpu#%d will be the slave. mask=0x%x\n",__FUNCTION__,cpuId,GetCpuActiveMask());
    }
    errCnt += cep_readNspin(CEP_VERSION_REG_K, cep_core0_status, CEP_OK2RUN_SIGNATURE, 0xFFFFFFFF,maxTO); 
  }
  if (GetVerbose()) {
    LOGI("%s: Done ...cpu=%d !!!\n",__FUNCTION__,cpuId);
  }  
  //
  return errCnt;
} // cep_srot::LLKI_Setup
// ------------------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------------------
// Public Method to perform error testing on the SRoT / LLKI_PP blocks
// ------------------------------------------------------------------------------------------------------------
int cep_srot::LLKI_ErrorTest(int cpuId) {

  int 			errCnt = 0;
  uint8_t		status = 0;

  // pick the first LSB in cpuActiveMask as master
  //
  int iAMmaster = 0;
#if 0
  for (int i = 0 ; i < 4 ; i++) {
    if (((1 << i) & GetCpuActiveMask())) { // found the first LSB=1 bit
      if (i == cpuId) {
        iAMmaster = 1;
      }
      break;
    }
  }
#else
  iAMmaster =1;
#endif
  //
  // Only one core should be talking to the SRoT
  //
  if (iAMmaster) {

    if (GetVerbose()) {
      LOGI("%s: cpu#%d is the master. mask=0x%x\n",__FUNCTION__,cpuId,GetCpuActiveMask());
    }

    // Initialize the Key Index RAM
    errCnt += InitKeyIndexRAM();

    // --------------------------------------------------------------------------------------------------------
    // Load some Key Indexes - Some good, some bad
    // --------------------------------------------------------------------------------------------------------
    // Load some good keys and indexes
    errCnt += LoadLLKIKey(0     , AES_BASE_K,     0,  1, AES_MOCK_TSS_KEY,    INVERT_ALL_BITS);
    errCnt += LoadLLKIKey(1     , AES_BASE_K,     2,  6, GPS_MOCK_TSS_KEY,    INVERT_ALL_BITS);

    // Disable LLKI to clear it.. to make it independent of the states of the chip, tony d.
    errCnt += DisableLLKI (0);


    // Load a Key Index with a bad pointer pair (low pointer > high pointer)
    cep_writeNcapture(SROT_BASE_K, SROT_KEYINDEXRAM_ADDR + 2*8, key_index_pack(
                      2,              // low pointer
                      1,              // high pointer
                      AES_BASE_K,     // core index
                      0x1));          // Valid

    // Load a Key Index with a bad pointer pair (pointer exceeds key ram size)
    cep_writeNcapture(SROT_BASE_K, SROT_KEYINDEXRAM_ADDR + 3*8, key_index_pack(
                      0,              // low pointer
                      256,            // high pointer
                      AES_BASE_K,     // core index
                      0x1));          // Valid

    // Load a Key Index with a bad core index
    cep_writeNcapture(SROT_BASE_K, SROT_KEYINDEXRAM_ADDR + 4*8, key_index_pack(
                      0,              // low pointer
                      1,              // high pointer
                      31,             // core index
                      0x1));          // Valid

    // --------------------------------------------------------------------------------------------------------


    // Perform a normal key load for the AES core
    errCnt += EnableLLKI (0);

    // --------------------------------------------------------------------------------------------------------
    // Peform a redundant key load load for the AES Core
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x01,                   // Message Length
                      0,                      // Key Index
                      0xDEADBEEF)); 
  
    // Poll the response waiting bit
    cep_readNspin(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, 100);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_KL_KEY_OVERWRITE, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // Clear the AES Core key
    errCnt += DisableLLKI (0);


    // --------------------------------------------------------------------------------------------------------
    // Load a bad length key into the AES Core
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x01,                   // Message Length
                      1,                      // Key Index
                      0xDEADBEEF)); 
  
    // Poll the response waiting bit
    cep_readNspin(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, 100);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_KL_BAD_KEY_LEN, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Send a BAD message ID
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      0xFF,                   // Bad Message ID
                      0x00,                   // Status (unused)
                      0x01,                   // Message Length
                      0,                      // Key Index
                      0xDEADBEEF)); 
  
    // Poll the response waiting bit
    cep_readNspin(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, 100);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_BAD_MSG_ID, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Send a BAD message Length  (all LLKI-C2 messages are length 1)
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x02,                   // BAD Message Length
                      0,                      // Key Index
                      0xDEADBEEF)); 
  
    // Poll the response waiting bit
    cep_readNspin(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, 100);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
	CHECK_RESPONSE(status, LLKI_STATUS_BAD_MSG_LEN, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Stimulate the LLKI_STATUS_KEY_INDX_EXCEED error
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x01,                   // Message Length
                      32,                     // Key Index
                      0xDEADBEEF)); 
  
    // Poll the response waiting bit
    cep_readNspin(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, 100);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
	CHECK_RESPONSE(status, LLKI_STATUS_KEY_INDEX_EXCEED, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Stimulate the LLKI_STATUS_KEY_INDEX_INVALID error
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x01,                   // Message Length
                      31,                     // Key Index
                      0xDEADBEEF)); 
  
    // Poll the response waiting bit
    cep_readNspin(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, 100);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_KEY_INDEX_INVALID, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Stimulate the LLKI_STATUS_BAD_POINTER_PAIR error (low > high)
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x01,                   // Message Length
                      2,                      // Key Index
                      0xDEADBEEF)); 
  
    // Poll the response waiting bit
    cep_readNspin(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, 100);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
	CHECK_RESPONSE(status, LLKI_STATUS_BAD_POINTER_PAIR, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Stimulate the LLKI_STATUS_BAD_POINTER_PAIR error (high pointer > Key RAM size)
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x01,                   // Message Length
                      3,                      // Key Index
                      0xDEADBEEF)); 
  
    // Poll the response waiting bit
    cep_readNspin(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, 100);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_BAD_POINTER_PAIR, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Stimulate the LLKI_STATUS_BAD_CORE_INDEX error
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x01,                   // Message Length
                      4,                      // Key Index
                      0xDEADBEEF)); 
  
    // Poll the response waiting bit
    cep_readNspin(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, 100);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_BAD_CORE_INDEX, GetVerbose());
    // --------------------------------------------------------------------------------------------------------

    // Perform a normal key load for the AES core (and verify things are still in a good state)
    errCnt += EnableLLKI (0);

    // Clear the AES core key to allow for multiple loops of this test
    errCnt += DisableLLKI (0);

  } // end if {iAMmaster)

  // Return the error count
  return errCnt;

} // cep_srot::LLKI_Error
