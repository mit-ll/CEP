//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
#include <string.h>

#include "simdiag_global.h"
#include "cep_adrMap.h"
#include "cepSrotTest.h"
#include "cep_srot.h"
#include "CEP.h"
#include "cepregression.h"
#include "random48.h"

#include "cep_apis.h"
#include "portable_io.h"

#ifdef SIM_ENV_ONLY
#include "simPio.h"
#endif


#ifdef BARE_MODE

#else
#include "simPio.h"
#endif

// Helper function for sending a message to the SRoT and waiting for a response
// Response status will be returned
uint8_t send_srot_message(int verbose, uint64_t message) {
  // Temporarily hold the response message
  uint64_t   response_message;
  int timeOut = 1000; // need timeout!!!

  //
  // Send a message to the SRoT
  //
  cep_write(SROT_INDEX, SROT_LLKIC2_SENDRECV_ADDR, message);

  // Poll the response waiting bit
  //do {;} while (!(cep_read(SROT_BASE_K, SROT_CTRLSTS_ADDR) & SROT_CTRLSTS_RESP_WAITING_MASK));
  while (timeOut > 0) {
    if (cep_read(SROT_INDEX, SROT_CTRLSTS_ADDR) & SROT_CTRLSTS_RESP_WAITING_MASK) break;
    timeOut--;
    if (timeOut == 0) return 0xff; // bad!!
  }
  
  // Read the response
  response_message = cep_read(SROT_INDEX, SROT_LLKIC2_SENDRECV_ADDR);

  // Return the status
  return((uint8_t)llkic2_extract_status(response_message));
} // end send_srot_message


int cepSrotTest_maxKeyTest(int cpuId, int verbose) {
  int errCnt=0;

#ifndef BARE_MODE
  //  uint64_t  response_message;
  uint8_t   response_status;
  int core_index=0;
  int key_index=0;
  uint64_t *kPtr;
  int kSize;
  int maxKeySize = 128; // enough
  //  int expStatus ;

  //
  // fill the key ram with all 1s
  //
  LOGI("%s : Initializing Key RAM to all Ones\n",__FUNCTION__);

  for (int i = 0; i < MAX_LLKI_KEY_SIZE; i++) {
    cep_write(SROT_INDEX, SROT_KEYRAM_ADDR + (i*8), (uint64_t)-1);
  }

  //
  // Goes thru all the cores!!!
  //
  for (core_index = 0; core_index < CEP_LLKI_CORES; core_index++) {

    // For now, the key_index will equal the core_index (though this is not required)
    key_index = core_index;
    
    cep_key_info_t key = KEY_DATA[core_index];
    kPtr = (uint64_t *) key.keyData;
    kSize = key.highPointer - key.lowPointer + 1;
    maxKeySize = 128;

    // Loop through different keys for the current core
    for (int l = 0; l < 4; l++) {

      if (errCnt) return 1;
      LOGI("%s: **************** Key_index = %d loop=%d ********************\n",__FUNCTION__, key_index, l);
      int ks = kSize-1;
      
      // Load key variants
      switch (l) {
        case 0:
          // -----------------------
          // BAD!!! Keys=0xFFFF...FFF...
          // -----------------------
          for (int i = 0; i < kSize; i++) {
            cep_write(SROT_INDEX, SROT_KEYRAM_ADDR + (i*8), (uint64_t) - 1);
          }
          break;
        case 1:
          // -----------------------
          // Inverted Key
          // -----------------------
          for (int i = 0; i < kSize; i++) {
            cep_write(SROT_INDEX, SROT_KEYRAM_ADDR + (i*8), ~kPtr[i]);
          }
          break;
        case 2:
          // -----------------------
          //  BAD LEN : max len Key with the front end = expected key
          // -----------------------
          for (int i = 0; i < kSize; i++) {
            cep_write(SROT_INDEX, SROT_KEYRAM_ADDR + (i*8), kPtr[i]);
          }
          ks = maxKeySize - 1;
          break;
        case 3:
          // -----------------------
          // Good Key: already loaded when l==2
          // -----------------------
          break;
      } // switch

      // Load the Key Index (based on the currently selected core) in the Key Index RAM
      LOGI("%s : Loading the Key Index into the Key Index RAM\n",__FUNCTION__);
      cep_write(SROT_INDEX, SROT_KEYINDEXRAM_ADDR + key_index * 8, key_index_pack( 0x0000,   // low pointer
                  ks,            // high pointer
                  core_index,    // core index
                  0x1));         // Valid

      // Tell the SRoT to load a key into the core referenced by the Key Index
      LOGI("%s : Issuing a Load Key Request\n",__FUNCTION__);
      response_status = send_srot_message(verbose, llkic2_pack( LLKI_MID_C2LOADKEYREQ,    // Message ID
                0x00,                     // Status (unused)
                0x01,                     // Message Length
                key_index,                // Key Index
                0xDEADBEEF));             // Reserved Field
      if (errCnt) return 1;
      
      // Expected responses diverge based on which key we are using
      switch (l) {
        case 0 :
        case 1 :
        case 3 :  
          CHECK_RESPONSE(response_status, LLKI_STATUS_GOOD,  verbose);
          break;      
        case 2 :
          CHECK_RESPONSE(response_status, LLKI_STATUS_KL_BAD_KEY_LEN,  verbose);
      } // switch
      if (errCnt) return 1;

      // Issue a Key Status Request to the select core
      LOGI("%s : Issuing a Key Status Request\n",__FUNCTION__);
      response_status = send_srot_message(verbose, llkic2_pack( LLKI_MID_C2KEYSTATUSREQ,  // Message ID
                0x00,                     // Status (unused)
                0x01,                     // Message Length
                key_index,                // Key Index
                0xDEADBEEF));             // Reserved Field

      // Expected responses diverge based on which key we are using
      switch (l) {
        case 0 :
        case 1 :
        case 3 :  
          CHECK_RESPONSE(response_status, LLKI_STATUS_KEY_PRESENT, verbose);       
          break;      
        case 2 :
          CHECK_RESPONSE(response_status, LLKI_STATUS_KEY_NOT_PRESENT, verbose);
      } // switch
      if (errCnt) return 1;

      // Issue a Clear Key Request to the select core      
      LOGI("%s : Issuing a Clear Key Request\n",__FUNCTION__);
      response_status = send_srot_message(verbose, llkic2_pack( LLKI_MID_C2CLEARKEYREQ,  // Message ID
                0x00,                     // Status (unused)
                0x01,                     // Message Length
                key_index,                // Key Index
                0xDEADBEEF));             // Reserved Field
      
      // Check expected response
      CHECK_RESPONSE(response_status, LLKI_STATUS_GOOD, verbose);
      if (errCnt) return 1;
      
      // Issue a Key Status Request to the select core
      LOGI("%s : Issuing a Key Status Request\n",__FUNCTION__);
      response_status = send_srot_message(verbose, llkic2_pack( LLKI_MID_C2KEYSTATUSREQ,  // Message ID
                0x00,                     // Status (unused)
                0x01,                     // Message Length
                key_index,                // Key Index
                0xDEADBEEF));             // Reserved Field

      // Check expected response
      CHECK_RESPONSE(response_status, LLKI_STATUS_KEY_NOT_PRESENT, verbose);       
      if (errCnt) return 1;

    } // for (int l = 0; l < 4; l++)
  } // for (core_index = 0; core_index < CEP_LLKI_CORES; core_index++)
#endif

  //
  // Complete the test (successfully)
  return 0;

}
