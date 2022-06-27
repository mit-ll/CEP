//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:       cep_srot.cc/h
// Program:         Common Evaluation Platform (CEP)
// Description:     Functions / Test for the CEP Surrogate Root of Trust
// Notes:           In the constructor function below, there are two
//                  indicies (coreIndex & statusIndex).  These are
//                  references to the ipCores data structure 
//                  (cepregression.cc/h).  coreIndex for the surrogate
//                  root of trust and statusIndex is for the 
//                  CEP Registers.
//              
//************************************************************************

#if defined(BARE_MODE)
#else

#include "simPio.h"

#endif

#include <string.h>
#include "cep_srot.h"
#include "simdiag_global.h"
#include "CEP.h"
#include "random48.h"
#include "cep_adrMap.h"
#include "portable_io.h"

#define IS_ON(c) ((1 << c) & GetCoreMask())

cep_srot::cep_srot(int coreIndex, int statusIndex, int verbose) {
  
  init(coreIndex);

  mStatusIndex = statusIndex;
  
  // Default Timeout Parameter
  maxTO = 6000;
  
  // Set the verbosity
  SetVerbose(verbose);
  
  // Set the default CPU Active Mask
  SetCpuActiveMask(0xf);

  // Initialize some of the core datastructures
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
    LOGI("%s: Initializing the KEY Index RAM\n",__FUNCTION__);
  }

  // The cep_writeNcapture function uses an offset from the current "core's" base address
  for (int i = 0; i < (int)SROT_KEYINDEXRAM_SIZE; i++) {
    cep_writeNcapture(SROT_KEYINDEXRAM_ADDR + (i*8), 0);
  }

  return errCnt;

} // cep_srot::InitKeyIndexRAM
// ------------------------------------------------------------------------------------------------------------



// ------------------------------------------------------------------------------------------------------------
// This method will load the specified Key Index and Key into the appropriate RAMs in the SRoT
// ------------------------------------------------------------------------------------------------------------
// Parameter list:
//    KeyIndex    - Selected Index into the Key Index RAM
//    CoreIndex   - Which core is this key for?
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
    LOGI("%s: Loading a key into the Key RAM\n",__FUNCTION__);
  }
  if (invertType == INVERT_ALTERNATE) {
    mInvertMask = 0x5555555555555555LLU;
  } else {
    mInvertMask = 0xFFFFFFFFFFFFFFFFLLU;
  }
  for (int i = 0; i < (HighPointer - LowPointer + 1); i++) {
    // not invert all the bits, just the odd bits, IIR will give the same results if flip all bits
    cep_writeNcapture(SROT_KEYRAM_ADDR + LowPointer*8 + (i*8), mInvertKey ? (Key[i] ^ mInvertMask) : Key[i]);
  }
  
  // Load a valid key index
  if (GetVerbose(2)) {
    LOGI("%s: Loading a valid key index\n",__FUNCTION__);
  }
  cep_writeNcapture(SROT_KEYINDEXRAM_ADDR + KeyIndex*8, key_index_pack(
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
    LOGI("%s: Switching the LLKI from DEBUG to OPERATIONAL mode\n",__FUNCTION__);
  }
  cep_writeNcapture(SROT_CTRLSTS_ADDR, SROT_CTRLSTS_MODEBIT0_MASK | SROT_CTRLSTS_MODEBIT1_MASK);

  return errCnt;

} // cep_srot::SetOperationalMode
// ------------------------------------------------------------------------------------------------------------



// ------------------------------------------------------------------------------------------------------------
// The following method will send a Load Key Request to the SRoT with the specified key index
// ------------------------------------------------------------------------------------------------------------
int cep_srot::EnableLLKI (uint8_t KeyIndex)
{

  int       errCnt = 0;
  uint8_t   status = 0;

  //
  if (GetVerbose(2)) {
    LOGI("%s: Issuing a Load Key Request to the SRoT\n",__FUNCTION__);
  }

  // Write a word to the LLKI C2 Send FIFO
  cep_writeNcapture(SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                    LLKI_MID_C2LOADKEYREQ,  // Message ID
                    0x00,                   // Status (unused)
                    0x0001,                 // Message Length
                    KeyIndex,               // Key Index
                    0xDEADBE));             // rsvd1
  
  // Poll the response waiting bit.. acknowledging that he timeout needs to accomodate the LONGEST key
  cep_readNspin(SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, maxTO);
  
  // Read and check the response
  status = llkic2_extract_status(cep_readNcapture(SROT_LLKIC2_SENDRECV_ADDR));
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

  int       errCnt = 0;
  uint8_t   status = 0;

  if (GetVerbose(2)) {
    LOGI("%s: Issuing a Clear Key Request to the SRoT\n",__FUNCTION__);
    LOGI("\n");
  }
  // Write a word to the LLKI C2 Send FIFO
  cep_writeNcapture(SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                    LLKI_MID_C2CLEARKEYREQ, // Message ID
                    0x00,                   // Status (unused)
                    0x0001,                 // Message Length
                    KeyIndex,               // Key Index
                    0xDEADBE));             // rsvd1
  
  // Poll the response waiting bit
  cep_readNspin(SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, maxTO);
  
  // Read and check the response
  status = llkic2_extract_status(cep_readNcapture(SROT_LLKIC2_SENDRECV_ADDR));

  CHECK_RESPONSE(status, LLKI_STATUS_GOOD, GetVerbose());

  // Return the error count
  return errCnt;

}
// ------------------------------------------------------------------------------------------------------------



// ------------------------------------------------------------------------------------------------------------
// Public Method for setting up the LLKI
//
// Every call to this method will begin by initializing the SRoT's KeyIndexRAM, which will effectively "erase"
// all existing SRoT keys
// ------------------------------------------------------------------------------------------------------------
int cep_srot::LLKI_Setup(int cpuId) {

  int errCnt = 0;
    
  // pick the first LSB in cpuActiveMask as master
  int iAMmaster = 0;
  for (int i = 0; i < 4; i++) {
    if (((1 << i) & GetCpuActiveMask())) { // found the first LSB=1 bit
      if (i == cpuId) {
        iAMmaster = 1;
      }
      break;
    }
  }

  // Only one core can perform LLKI operations
  if (iAMmaster) {
     
    if (GetVerbose()) {
      LOGI("%s: cpu#%d is the master. cpuActiveMask=0x%x\n",__FUNCTION__, cpuId, GetCpuActiveMask());
    }
  
    // Initialize the LLKI Key Index RAM to all zeroes (thus effectively deleting any existing keys)
    errCnt += InitKeyIndexRAM();

    if (errCnt) return errCnt;

    //
    // Loop through all the LLKI cores and load keys if the following conditions are met:
    //  1) The core is enabled as define in cep_core_info (CEP.h)
    //  2) The current CoreMask has selected the current core
    //
    // As a single index is being used to access both the cep_core_info and KEY_DATA arrays,
    // the cores that use keys must be lower justified in cep_core_info array
    for (int coreIndex = 0; coreIndex < CEP_LLKI_CORES; coreIndex ++) {
      cep_core_info_t core = cep_core_info[coreIndex];
      cep_key_info_t  key  = KEY_DATA[coreIndex];

      core_type_t coreType = core.type;
      
      if (IS_ON(coreIndex) && core.enabled) {

        // Load the appropriate key
        errCnt += LoadLLKIKey(
                    coreIndex,        // KeyIndex (address in the KeyIndexRAM)
                    coreIndex,        // CoreIndex (defines the core for which this key is destined for.  Corresponds
                                      // to the core indicies defined in DevKitConfigs.scala)
                    key.lowPointer,
                    key.highPointer,
                    key.keyData,
                    key.invertType);
        if (errCnt) {
          cep_writeNcapture(mStatusIndex, cep_scratch4_reg, CEP_OK2RUN_SIGNATURE);
          return errCnt;
        }
        errCnt += DisableLLKI(coreIndex);
        if (errCnt) {
          cep_writeNcapture(mStatusIndex, cep_scratch4_reg, CEP_OK2RUN_SIGNATURE);
          return errCnt;
        }
        errCnt += EnableLLKI(coreIndex);
        if (errCnt) {
          cep_writeNcapture(mStatusIndex, cep_scratch4_reg, CEP_OK2RUN_SIGNATURE);
          return errCnt;
        }

      } // if (IS_ON(keyIndex) && core.enabled)

    } // for (int coreIndex = 0; coreIndex < CEP_LLKI_CORES; coreIndex ++) 
        
    // Write to the core0 status register indicating that the LLKI operations are complete
    cep_writeNcapture(mStatusIndex, cep_scratch4_reg, CEP_OK2RUN_SIGNATURE);
      
  } else {  // else if (iAMMaster)

    if (GetVerbose()) {
      LOGI("%s: cpu#%d will be the slave. mask=0x%x\n",__FUNCTION__,cpuId,GetCpuActiveMask());
    }
    errCnt += cep_readNspin(mStatusIndex, cep_scratch4_reg, CEP_OK2RUN_SIGNATURE, 0xFFFFFFFF, maxTO); 
    
  } // if (iAMmaster) {

  if (GetVerbose()) {
    LOGI("%s: Done ...cpu=%d !!!\n",__FUNCTION__,cpuId);
  }  
  
  return errCnt;
} // cep_srot::LLKI_Setup
// ------------------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------------------
// Public Method to perform error testing on the SRoT / LLKI_PP blocks
// ------------------------------------------------------------------------------------------------------------
int cep_srot::LLKI_ErrorTest(int cpuId) {

  int       errCnt    = 0;
  uint8_t   status    = 0;
  int       iAMmaster = 0;

  for (int i = 0 ; i < 4 ; i++) {
    if (((1 << i) & GetCpuActiveMask())) { // found the first LSB=1 bit
      if (i == cpuId) {
        iAMmaster = 1;
      }
      break;
    }
  }

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
    cep_key_info_t key0 = KEY_DATA[0];
    cep_key_info_t key1 = KEY_DATA[1];
    
    errCnt += LoadLLKIKey(0 , 0, key0.lowPointer, key0.highPointer, key0.keyData, key0.invertType);

    // Bad core index
    errCnt += LoadLLKIKey(1 , 0, key1.lowPointer,  key1.highPointer, key1.keyData, key1.invertType); 

    // Disable LLKI to clear it.. to make it independent of the states of the chip, tony d.
    errCnt += DisableLLKI (0);


    // Load a Key Index with a bad pointer pair (low pointer > high pointer)
    cep_writeNcapture(SROT_KEYINDEXRAM_ADDR + 2*8, key_index_pack(
                      2,                // low pointer
                      1,                // high pointer
                      0,                // core index
                      0x1));            // Valid

    // Load a Key Index with a bad pointer pair (pointer exceeds key ram size)
    cep_writeNcapture(SROT_KEYINDEXRAM_ADDR + 3*8, key_index_pack(
                      0,                // low pointer
                      SROT_KEYRAM_SIZE, // high pointer
                      0,                // core index
                      0x1));            // Valid

    // Load a Key Index with a bad core index
    cep_writeNcapture(SROT_KEYINDEXRAM_ADDR + 4*8, key_index_pack(
                      0,                // low pointer
                      1,                // high pointer
                      31,               // core index
                      0x1));            // Valid

    // --------------------------------------------------------------------------------------------------------


    // Perform a normal key load for the AES core
    errCnt += EnableLLKI (0);

    // --------------------------------------------------------------------------------------------------------
    // Peform a redundant key load load for the AES Core
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x0001,                 // Message Length
                      0,                      // Key Index
                      0xDEADBE));             // rsvd1
  
    // Poll the response waiting bit
    cep_readNspin(SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, maxTO);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_KL_KEY_OVERWRITE, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // Clear the AES Core key
    errCnt += DisableLLKI (0);


    // --------------------------------------------------------------------------------------------------------
    // Load a bad length key into the AES Core
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x0001,                 // Message Length
                      1,                      // Key Index
                      0xDEADBE));             // rsvd1
  
    // Poll the response waiting bit
    cep_readNspin(SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, maxTO);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_KL_BAD_KEY_LEN, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Send a BAD message ID
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      0xFF,                   // Bad Message ID
                      0x00,                   // Status (unused)
                      0x0001,                 // Message Length
                      0,                      // Key Index
                      0xDEADBE));             // rsvd1
  
    // Poll the response waiting bit
    cep_readNspin(SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, maxTO);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_BAD_MSG_ID, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Send a BAD message Length  (all LLKI-C2 messages are length 1)
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x0002,                 // BAD Message Length
                      0,                      // Key Index
                      0xDEADBE));             // rsvd1
  
    // Poll the response waiting bit
    cep_readNspin(SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, maxTO);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_BAD_MSG_LEN, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Stimulate the LLKI_STATUS_KEY_INDX_EXCEED error
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x0001,                 // Message Length
                      32,                     // Key Index
                      0xDEADBE));             // rsvd1
  
    // Poll the response waiting bit
    cep_readNspin(SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, maxTO);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_LLKIC2_SENDRECV_ADDR));
  CHECK_RESPONSE(status, LLKI_STATUS_KEY_INDEX_EXCEED, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Stimulate the LLKI_STATUS_KEY_INDEX_INVALID error
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x0001,                 // Message Length
                      31,                     // Key Index
                      0xDEADBE));             // rsvd1
  
    // Poll the response waiting bit
    cep_readNspin(SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, maxTO);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_KEY_INDEX_INVALID, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Stimulate the LLKI_STATUS_BAD_POINTER_PAIR error (low > high)
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x0001,                 // Message Length
                      2,                      // Key Index
                      0xDEADBE));             // rsvd1
  
    // Poll the response waiting bit
    cep_readNspin(SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, maxTO);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_BAD_POINTER_PAIR, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Stimulate the LLKI_STATUS_BAD_POINTER_PAIR error (high pointer > Key RAM size)
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x0001,                 // Message Length
                      3,                      // Key Index
                      0xDEADBE));             // rsvd1
  
    // Poll the response waiting bit
    cep_readNspin(SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, maxTO);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_LLKIC2_SENDRECV_ADDR));
    CHECK_RESPONSE(status, LLKI_STATUS_BAD_POINTER_PAIR, GetVerbose());
    // --------------------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------------------
    // Stimulate the LLKI_STATUS_BAD_CORE_INDEX error
    // --------------------------------------------------------------------------------------------------------
    cep_writeNcapture(SROT_LLKIC2_SENDRECV_ADDR, llkic2_pack(  
                      LLKI_MID_C2LOADKEYREQ,  // Message ID
                      0x00,                   // Status (unused)
                      0x0001,                 // Message Length
                      4,                      // Key Index
                      0xDEADBE));             // rsvd1
  
    // Poll the response waiting bit
    cep_readNspin(SROT_CTRLSTS_ADDR, SROT_CTRLSTS_RESP_WAITING_MASK, SROT_CTRLSTS_RESP_WAITING_MASK, maxTO);

    // Compare expected and received responses
    status = llkic2_extract_status(cep_readNcapture(SROT_LLKIC2_SENDRECV_ADDR));
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
