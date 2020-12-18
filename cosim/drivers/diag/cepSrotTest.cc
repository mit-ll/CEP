//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
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

  // Write a word to the LLKI C2 Send FIFO
  cep_write(SROT_BASE_K, SROT_LLKIC2_SENDRECV_ADDR, message);

  // Poll the response waiting bit
  do {;} while (!(cep_read(SROT_BASE_K, SROT_CTRLSTS_ADDR) & SROT_CTRLSTS_RESP_WAITING_MASK));
  
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

int cepSrotTest_runTest(int cpuId, int seed, int verbose) {

  uint64_t  response_message;
  uint8_t   response_status;

  // Initialize the SRoT Key Index RAM
  LOGI("\n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("cepSrotTest_runTest: Initializing the Key Index RAM...                      \n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("\n");
  for (int i = 0; i < SROT_KEYINDEXRAM_SIZE; i++) {
    cep_write(SROT_BASE_K, SROT_KEYINDEXRAM_ADDR + (i*8), 0x0000000000000000);
  }

  LOGI("\n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("cepSrotTest_runTest: Loading a key into the Key RAM...                      \n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("\n");
  cep_write(SROT_BASE_K, SROT_KEYRAM_ADDR + 0, AES_MOCK_TSS_KEY_0);
  cep_write(SROT_BASE_K, SROT_KEYRAM_ADDR + 8, AES_MOCK_TSS_KEY_1);

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
  }

  // Set the desired message values
  LOGI("\n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("cepSrotTest_runTest: Issuing a Key Status Request to the SRoT and waiting   \n");
  LOGI("cepSrotTest_runTest: for the response...                                    \n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("\n");
  // Write a word to the LLKI C2 Send FIFO
  response_status = send_srot_message(verbose, llkic2_pack( LLKI_MID_C2KEYSTATUSREQ,  // Message ID
                                                            0x00,                     // Status (unused)
                                                            0x01,                     // Message Length
                                                            0x00,                     // Key Index
                                                            0xDEADBEEF));             // Reserved Field
  switch (response_status) {
    case LLKI_STATUS_KEY_PRESENT :
      LOGI("\n");
      LOGI("----------------------------------------------------------------------------\n");
      LOGI("cepSrotTest_runTest: Key is present (as expected)                           \n");
      LOGI("----------------------------------------------------------------------------\n");
      LOGI("\n");
      break;
    case LLKI_STATUS_KEY_NOT_PRESENT :
      LOGE("\n");
      LOGE("----------------------------------------------------------------------------\n");
      LOGE("cepSrotTest_runTest: Key is NOT present (not expected)                      \n");
      LOGE("----------------------------------------------------------------------------\n");
      LOGE("\n");
      return 1;
    default:
      parse_message_status(response_status, verbose);
      return 1;
  }

  // Set the desired message values
  LOGI("\n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("cepSrotTest_runTest: Issuing a Clear Key Request to the SRoT and waiting    \n");
  LOGI("cepSrotTest_runTest: for the response...                                    \n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("\n");
  response_status = send_srot_message(verbose, llkic2_pack( LLKI_MID_C2CLEARKEYREQ,  // Message ID
                                                            0x00,                     // Status (unused)
                                                            0x01,                     // Message Length
                                                            0x00,                     // Key Index
                                                            0xDEADBEEF));             // Reserved Field
  if (parse_message_status(response_status, verbose)) {
    return 1;
  }


  // Set the desired message values
  LOGI("\n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("cepSrotTest_runTest: Issuing a Key Status Request to the SRoT and waiting   \n");
  LOGI("cepSrotTest_runTest: for the response...                                    \n");
  LOGI("----------------------------------------------------------------------------\n");
  LOGI("\n");
  response_status = send_srot_message(verbose, llkic2_pack( LLKI_MID_C2KEYSTATUSREQ,  // Message ID
                                                            0x00,                     // Status (unused)
                                                            0x01,                     // Message Length
                                                            0x00,                     // Key Index
                                                            0xDEADBEEF));             // Reserved Field

  switch (response_status) {
    case LLKI_STATUS_KEY_NOT_PRESENT :
      LOGI("\n");
      LOGI("----------------------------------------------------------------------------\n");
      LOGI("cepSrotTest_runTest: Key is NOT present (as expected)                       \n");
      LOGI("----------------------------------------------------------------------------\n");
      LOGI("\n");
      break;
    case LLKI_STATUS_KEY_PRESENT :
      LOGE("\n");
      LOGE("----------------------------------------------------------------------------\n");
      LOGE("cepSrotTest_runTest: Key is present (not expected)                          \n");
      LOGE("----------------------------------------------------------------------------\n");
      LOGE("\n");
      return 1;
    default:
      parse_message_status(response_status, verbose);
      return 1;
  }


  // Complete the test (successfully)
  return 0;

}
