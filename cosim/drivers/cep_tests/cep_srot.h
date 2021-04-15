//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cep_srot.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    SRoT class for CEP
// Notes:          
//************************************************************************

#ifndef cep_srot_H
#define cep_srot_H
// ---------------------------
#include "stdint.h"
#include "cep_crypto.h"

#define INVERT_ALL_BITS  1
#define INVERT_ALTERNATE 2

#define MAX_LLKI_KEY_SIZE   256   // Maximize size of an LLKI Key in units
                                  // of 64-bit words

// ----------------------------------------------------------------------------
// LLKI related constants, structures, and function prototypes
//
// Note: Surrogate Root of Trust address constants may be found in CEP.h
// ----------------------------------------------------------------------------

// LLKI Message ID constants
const uint8_t   LLKI_MID_C2LOADKEYREQ             = 0x00;
const uint8_t   LLKI_MID_C2CLEARKEYREQ            = 0x01;
const uint8_t   LLKI_MID_C2KEYSTATUSREQ           = 0x02;
const uint8_t   LLKI_MID_C2LOADKEYACK             = 0x03;
const uint8_t   LLKI_MID_C2CLEARKEYACK            = 0x04;
const uint8_t   LLKI_MID_C2KEYSTATUSRESP          = 0x05;
const uint8_t   LLKI_MID_C2ERRORRESP              = 0x06;

// LLKI Status Constants
const uint8_t   LLKI_STATUS_GOOD                  = 0x00;
const uint8_t   LLKI_STATUS_KEY_PRESENT           = 0x01;
const uint8_t   LLKI_STATUS_KEY_NOT_PRESENT       = 0x02;

const uint8_t   LLKI_STATUS_BAD_MSG_ID            = 0x20;
const uint8_t   LLKI_STATUS_BAD_MSG_LEN           = 0x21;
const uint8_t   LLKI_STATUS_KEY_INDEX_EXCEED      = 0x22; // Specified key index exceeds size
                                                            // of key index ram
const uint8_t   LLKI_STATUS_KEY_INDEX_INVALID     = 0x23;
const uint8_t   LLKI_STATUS_BAD_POINTER_PAIR      = 0x24;
const uint8_t   LLKI_STATUS_BAD_CORE_INDEX        = 0x25;
const uint8_t   LLKI_STATUS_KL_REQ_BAD_MSG_ID     = 0x26;
const uint8_t   LLKI_STATUS_KL_REQ_BAD_MSG_LEN    = 0x27;
const uint8_t   LLKI_STATUS_KL_RESP_BAD_MSG_ID    = 0x28;
const uint8_t   LLKI_STATUS_KL_TILELINK_ERROR     = 0x29;
const uint8_t   LLKI_STATUS_KL_LOSS_OF_SYNC       = 0x30;
const uint8_t   LLKI_STATUS_KL_BAD_KEY_LEN        = 0x31;
const uint8_t   LLKI_STATUS_KL_KEY_OVERWRITE      = 0x32;

const uint8_t   LLKI_STATUS_UNKNOWN_ERROR         = 0xFF;

// LLKI Core Index Constants
const uint8_t   LLKI_AES_CORE_INDEX               = 0x00;
const uint8_t   LLKI_INVALID_CORE_INDEX           = 0xFF;
const uint8_t   LLKI_KEYINDEX_VALID               = 0x80;

// Packing macro for creating an LLKIC2 message to send to the SRoT 
#define llkic2_pack(msg_id, status, msg_len, key_index, rsvd1) \
  (((uint64_t)msg_id << 0)            & 0x00000000000000FF) | \
  (((uint64_t)status << 8)            & 0x000000000000FF00) | \
  (((uint64_t)msg_len << 16)          & 0x0000000000FF0000) | \
  (((uint64_t)key_index << 24)        & 0x00000000FF000000) | \
  (((uint64_t)rsvd1 << 32)            & 0xFFFFFFFF00000000) 

// Packing macro for creating a Key Index entry
#define key_index_pack(low_pointer, high_pointer, core_index, valid) \
  (((uint64_t)low_pointer << 0)       & 0x000000000000FFFF) | \
  (((uint64_t)high_pointer << 16)     & 0x00000000FFFF0000) | \
  (((uint64_t)core_index << 32)       & 0x000000FF00000000) | \
  (((uint64_t)valid << 63)            & 0x8000000000000000) 

// Helper macros for response message processing
#define llkic2_extract_status(message) \
  (uint8_t)(((uint64_t)message >> 8)  & 0x00000000000000FF)

// Helper macro for comparing the response to an expected value
#define CHECK_RESPONSE(act, exp, verbose) \
  do { \
    if (exp != act) {           \
      LOGE("ERROR: response_status = 0x%x != %s (0x%x)\n",act,#exp,exp); \
      errCnt++; \
    } else if (verbose) { \
      LOGI("OK: response_status = 0x%x == %s\n",act,#exp); \
    } \
  } while (0)

// Mock TSS Keys
const uint64_t  AES_MOCK_TSS_KEY[]      = {
  0x0123456789ABCDEF,
  0xFEDCBA9876543210
};

const uint64_t  DES3_MOCK_TSS_KEY[]     = {
  0x0123456789ABCDEF
};

const uint64_t  SHA256_MOCK_TSS_KEY[]     = {
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210
};

const uint64_t  MD5_MOCK_TSS_KEY[]     = {
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210
};

const uint64_t  RSA_MOCK_TSS_KEY[]     = {
  0x0123456789ABCDEF
};

const uint64_t  IIR_MOCK_TSS_KEY[]     = {
  0x0123456789ABCDEF
};

const uint64_t  FIR_MOCK_TSS_KEY[]     = {
  0x0123456789ABCDEF
};

const uint64_t  DFT_MOCK_TSS_KEY[]     = {
  0x0123456789ABCDEF
};

const uint64_t  IDFT_MOCK_TSS_KEY[]     = {
  0x0123456789ABCDEF
};

const uint64_t  GPS_MOCK_TSS_KEY[]     = {
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF
};

//
// CEP's SRoT
//
class cep_srot : public cep_crypto { 
  
  //
  // Public methods
  //
  public: 
    cep_srot(int verbose);  
    ~cep_srot() {}; 

    //
    int SetOperationalMode (void);
    int EnableLLKI (uint8_t KeyIndex);
    int DisableLLKI (uint8_t KeyIndex);
    //
    int LLKI_Setup(int cpuId);
    int LLKI_ErrorTest(int cpuId);

    void SetCpuActiveMask(int mask) {  mCpuActiveMask = mask; }
    int  GetCpuActiveMask(void) { return mCpuActiveMask; }
    // for negative testing
    void LLKI_invertKey(int enable) { mInvertKey = enable; }
  //
  // Private methods
  //
  protected:

    // ------------------------------------------------------
    // Move them to protected
    // ------------------------------------------------------
    int InitKeyIndexRAM (void);
    int LoadLLKIKey (uint8_t KeyIndex, uint8_t CoreIndex, uint16_t LowPointer, uint16_t HighPointer, const uint64_t *Key, int invertType);
    int LLKIInitComplete (int cpuId);
    int LLKIClearComplete (int cpuId);

    int mCpuActiveMask;
    int maxTO;  
    //
    int mInvertKey;
    uint64_t mInvertMask;
};

#endif
