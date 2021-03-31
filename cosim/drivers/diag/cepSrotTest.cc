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
  cep_write(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, message);

  // Poll the response waiting bit
  //do {;} while (!(cep_read(SROT_BASE_K, SROT_CTRLSTS_ADDR) & SROT_CTRLSTS_RESP_WAITING_MASK));
  while (timeOut > 0) {
    if (cep_read(SROT_BASE_K, SROT_CTRLSTS_ADDR) & SROT_CTRLSTS_RESP_WAITING_MASK) break;
    timeOut--;
    if (timeOut == 0) return 0xff; // bad!!
  }
  
  // Read the response
  response_message = cep_read(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR);

  // Return the status
  return((uint8_t)llkic2_extract_status(response_message));
} // end send_srot_message


// Helper function for parsing LLKIC2 message statu
int parse_message_status(uint8_t message_status, int verbose) {

  // Parse the status field to see if any error occured
  switch (message_status) {
    case LLKI_STATUS_GOOD                 :
      if (verbose) {
        LOGI("\n");
        LOGI("parse_message_status: Good message received!\n");
        LOGI("\n");
      }
      break;
    case LLKI_STATUS_KEY_PRESENT            :
      LOGI("\n");
      LOGI("parse_message_status: LLKI-KL Key Present\n");
      LOGI("\n");
      break;
    case LLKI_STATUS_KEY_NOT_PRESENT        :
      LOGI("\n");
      LOGI("parse_message_status: LLKI-KL Key Not Present\n");
      LOGI("\n");
      return 0;
      break;
    case LLKI_STATUS_BAD_MSG_ID             :
      LOGE("\n");
      LOGE("parse_message_status: Bad message ID\n");
      LOGE("\n");
      return 1;
      break;
    case LLKI_STATUS_BAD_MSG_LEN            :
      LOGE("\n");
      LOGE("parse_message_status: Bad message length\n");
      LOGE("\n");
      return 1;
      break;
    case LLKI_STATUS_KEY_INDEX_EXCEED   :
      LOGE("\n");
      LOGE("parse_message_status: Key Index exceeds allowable bounds\n");
      LOGE("\n");
      return 1;
      break;
    case LLKI_STATUS_KEY_INDEX_INVALID  :
      LOGE("\n");
      LOGE("parse_message_status: Referenced Key Index is not valid\n");
      LOGE("\n");
      return 1;
      break;
    case LLKI_STATUS_BAD_POINTER_PAIR       :
      LOGE("\n");
      LOGE("parse_message_status: Referenced Key Index pointer pair is not valid\n");
      LOGE("\n");
      return 1;
      break;
    case LLKI_STATUS_BAD_CORE_INDEX         :
      LOGE("\n");
      LOGE("parse_message_status: Referenced Key Index specifies an out of bounds core index\n");
      LOGE("\n");
      return 1;
      break;
    case LLKI_STATUS_KL_REQ_BAD_MSG_ID     :
      LOGE("\n");
      LOGE("parse_message_status: LLKI-KL Request Bad Message ID\n");
      LOGE("\n");
      return 1;
      break;
    case LLKI_STATUS_KL_REQ_BAD_MSG_LEN     :
      LOGE("\n");
      LOGE("parse_message_status: LLKI-KL Request Bad Message Length\n");
      LOGE("\n");
      return 1;
      break;
    case LLKI_STATUS_KL_RESP_BAD_MSG_ID     :
      LOGE("\n");
      LOGE("parse_message_status: LLKI-KL Response Bad Message ID\n");
      LOGE("\n");
      return 1;
      break;
    case LLKI_STATUS_KL_TILELINK_ERROR      :
      LOGE("\n");
      LOGE("parse_message_status: LLKI-KL Tilelink Error\n");
      LOGE("\n");
      return 1;
      break;
    case LLKI_STATUS_UNKNOWN_ERROR            :
      LOGE("\n");
      LOGE("parse_message_status: Unknown Error\n");
      LOGE("\n");
      return 1;
      break;
    default                                 :
      LOGE("\n");
      LOGE("parse_message_status: Unknown message error\n");
      LOGE("\n");
      return 1;
      break;
  } // end switch (my_message)

  // Status is good
  return 0;

} // end parse_message_status

//
// Max size key test: to make sure  no hang (not much to test :-( )
//
#define CHECK_RESPONSE(exp,act) do {       \
    if (exp != act) {           \
      LOGE("ERROR: response_status=0x%x != %s (0x%x)\n",act,#exp,exp); \
      errCnt++;             \
    } else if (verbose) {         \
      LOGI("OK: response_status=0x%x == %s\n",act,#exp);    \
    }               \
  } while (0)

//
// Macro to selective assign MOCK_TSS_KEY and its size based on the selected core
//
#define SET_KEY_PARAM(k)          \
  case k ## _BASE_K :           \
  kPtr = (uint64_t *)k ## _MOCK_TSS_KEY;      \
  kSize = sizeof(k ## _MOCK_TSS_KEY)/sizeof(uint64_t);    \
  maxKeySize = 128;           \
  break               \
  
int cepSrotTest_maxKeyTest(int cpuId, int verbose) {
  int errCnt=0;

#ifndef BARE_MODE
  uint64_t  response_message;
  uint8_t   response_status;
  int core_index=0;
  int key_index=0;
  uint64_t *kPtr;
  int kSize;
  int maxKeySize = 128; // enough
  int expStatus ;

  //
  // fill the key ram with all 1s
  //
  LOGI("%s : Initializing Key RAM to all Ones\n",__FUNCTION__);

<<<<<<< HEAD
  for (int i = 0; i < MAX_LLKI_KEY_SIZE; i++) {
    cep_write(SROT_BASE_K, SROT_KEYRAM_ADDR + (i*8), (uint64_t)-1);
=======
  LOGI("\n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("cepSrotTest_runTest: Loading a key into the Key RAM...                      \n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("\n");
  cep_write(SROT_BASE_K, SROT_KEYRAM_ADDR + 0, AES_MOCK_TSS_KEY[0]);
  cep_write(SROT_BASE_K, SROT_KEYRAM_ADDR + 8, AES_MOCK_TSS_KEY[1]);

  LOGI("\n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("cepSrotTest_runTest: Loading a valid key index...                           \n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("\n");
  cep_write(SROT_BASE_K, SROT_KEYINDEXRAM_ADDR + 0, key_index_pack( 0x0000,                 // low pointer
                                                                    0x0001,                 // high pointer
                                                                    LLKI_AES_CORE_INDEX,    // core index
                                                                    0x1));                  // Valid

  LOGI("\n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("cepSrotTest_runTest: Switching the LLKI from DEBUG to OPERATIONAL mode...   \n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("\n");
  cep_write(SROT_BASE_K, SROT_CTRLSTS_ADDR, SROT_CTRLSTS_MODEBIT0_MASK | SROT_CTRLSTS_MODEBIT1_MASK);

  // Set the desired message values
  LOGI("\n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("cepSrotTest_runTest: Issuing a Load Key Request to the SRoT and waiting     \n");
  LOGI("cepSrotTest_runTest: for the response...                                    \n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("\n");
  response_status = send_srot_message(verbose, llkic2_pack( LLKI_MID_C2LOADKEYREQ,    // Message ID
                                                            0x00,                     // Status (unused)
                                                            0x01,                     // Message Length
                                                            0x00,                     // Key Index
                                                            0xDEADBEEF));             // Reserved Field

  // Parse the message status
  if (parse_message_status(response_status, verbose)) {
    return 1;
>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0
  }

  //
  // Goes thru all the cores!!!
  //
  for (core_index = AES_BASE_K; core_index <= GPS_BASE_K; core_index++) {

    // For now, the key_index will equal the core_index (though this is not required)
    key_index = core_index;
    
    // Set the Key Paramaters
    switch (key_index) {
      SET_KEY_PARAM(AES);
      SET_KEY_PARAM(MD5);
      SET_KEY_PARAM(SHA256);
      SET_KEY_PARAM(RSA);
      SET_KEY_PARAM(DES3);
      SET_KEY_PARAM(DFT);
      SET_KEY_PARAM(IDFT);
      SET_KEY_PARAM(FIR);
      SET_KEY_PARAM(IIR);
      SET_KEY_PARAM(GPS);
    }

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
            cep_write(SROT_BASE_K, SROT_KEYRAM_ADDR + (i*8), (uint64_t) - 1);
          }
          break;
        case 1:
          // -----------------------
          // Inverted Key
          // -----------------------
          for (int i = 0; i < kSize; i++) {
            cep_write(SROT_BASE_K, SROT_KEYRAM_ADDR + (i*8), ~kPtr[i]);
          }
          break;
        case 2:
          // -----------------------
          //  BAD LEN : max len Key with the front end = expected key
          // -----------------------
          for (int i = 0; i < kSize; i++) {
            cep_write(SROT_BASE_K, SROT_KEYRAM_ADDR + (i*8), kPtr[i]);
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
      cep_write(SROT_BASE_K, SROT_KEYINDEXRAM_ADDR + key_index * 8, key_index_pack( 0x0000,   // low pointer
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
          CHECK_RESPONSE(LLKI_STATUS_GOOD, response_status);
          break;      
        case 2 :
          CHECK_RESPONSE(LLKI_STATUS_KL_BAD_KEY_LEN, response_status);
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
          CHECK_RESPONSE(LLKI_STATUS_KEY_PRESENT, response_status);       
          break;      
        case 2 :
          CHECK_RESPONSE(LLKI_STATUS_KEY_NOT_PRESENT, response_status);
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
      CHECK_RESPONSE(LLKI_STATUS_GOOD, response_status);
      if (errCnt) return 1;
      
      // Issue a Key Status Request to the select core
      LOGI("%s : Issuing a Key Status Request\n",__FUNCTION__);
      response_status = send_srot_message(verbose, llkic2_pack( LLKI_MID_C2KEYSTATUSREQ,  // Message ID
                0x00,                     // Status (unused)
                0x01,                     // Message Length
                key_index,                // Key Index
                0xDEADBEEF));             // Reserved Field

      // Check expected response
      CHECK_RESPONSE(LLKI_STATUS_KEY_NOT_PRESENT, response_status);       
      if (errCnt) return 1;

    } // for (int l = 0; l < 4; l++)
  } // for (core_index=AES_BASE_K;core_index<=GPS_BASE_K;core_index++)
#endif

  //
  // Complete the test (successfully)
  return 0;

}
