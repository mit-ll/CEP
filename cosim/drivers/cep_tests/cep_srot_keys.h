//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cep_srot.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    SRoT class for CEP
// Notes:          
//************************************************************************

#ifndef cep_srot_keys_H
#define cep_srot_keys_H

#include "stdint.h"

#define INVERT_ALL_BITS  1
#define INVERT_ALTERNATE 2

// Include all core unlock keys here.
// Mock TSS keys are defined in llki_pkg.sv, and duplicated here.
const uint64_t  AES_MOCK_TSS_KEY[]     = {
  0xAE53456789ABCDEF,
  0xFEDCBA9876543210
};

const uint64_t  DES3_MOCK_TSS_KEY[]    = {
  0xDE53456789ABCDEF
};

const uint64_t  SHA256_MOCK_TSS_KEY[]  = {
  0x54A3456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210
};

const uint64_t  MD5_MOCK_TSS_KEY[]     = {
  0x3D53456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210
};

const uint64_t  RSA_MOCK_TSS_KEY[]     = {
  0x45A3456789ABCDEF
};

const uint64_t  IIR_MOCK_TSS_KEY[]     = {
  0x1143456789ABCDEF
};

const uint64_t  FIR_MOCK_TSS_KEY[]     = {
  0xF143456789ABCDEF
};

const uint64_t  DFT_MOCK_TSS_KEY[]     = {
  0xDF73456789ABCDEF
};

const uint64_t  IDFT_MOCK_TSS_KEY[]    = {
  0x1DF7456789ABCDEF
};

const uint64_t  GPS_MOCK_TSS_KEY[]     = {
  0x6953456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF,
  0xFEDCBA9876543210,
  0x0123456789ABCDEF
};

//Key info struct for unlocking LLKI cores.
typedef struct cep_key_info_t {
    const char      *name;              //
    const uint64_t  *keyData;           // Actual Key Bits
    const uint64_t  lowPointer;         // Lower bound of where key will be loaded into KeyRAM
    const uint64_t  highPointer;        // Upper bound of where key will be loaded into KeyRAM
    const int       invertType;         // 
} cep_key_info_t;

// Number of LLKI cores for MacroMix to cover. 
// Don't include SROT or CEP Version "cores", which must be the last ones in this list.
#define CEP_LLKI_CORES 16
    
//
// Array of keys and key lengths (number of 64 bit words)
// index in array == core index
// The order defined here **MUST** correspond to those
// defined in DevKitConfigs.scala
//
// KeyLength = High Pointer - Low Pointer + 1
//
const cep_key_info_t KEY_DATA[CEP_LLKI_CORES] = {
  {"AES",      AES_MOCK_TSS_KEY,    0 ,  1, INVERT_ALL_BITS},
  {"MD5",      MD5_MOCK_TSS_KEY,    2 ,  9, INVERT_ALL_BITS},
  {"SHA256.0", SHA256_MOCK_TSS_KEY, 10, 17, INVERT_ALL_BITS},
  {"SHA256.1", SHA256_MOCK_TSS_KEY, 18, 25, INVERT_ALL_BITS},
  {"SHA256.2", SHA256_MOCK_TSS_KEY, 26, 33, INVERT_ALL_BITS},
  {"SHA256.3", SHA256_MOCK_TSS_KEY, 34, 41, INVERT_ALL_BITS},
  {"RSA",      RSA_MOCK_TSS_KEY,    42, 42, INVERT_ALL_BITS},
  {"DES3",     DES3_MOCK_TSS_KEY,   43, 43, INVERT_ALL_BITS},
  {"DFT",      DFT_MOCK_TSS_KEY,    44, 44, INVERT_ALL_BITS},
  {"IDFT",     IDFT_MOCK_TSS_KEY,   45, 45, INVERT_ALL_BITS},
  {"FIR",      FIR_MOCK_TSS_KEY,    46, 46, INVERT_ALTERNATE},
  {"IIR",      IIR_MOCK_TSS_KEY,    47, 47, INVERT_ALTERNATE},
  {"GPS.0",    GPS_MOCK_TSS_KEY,    48, 52, INVERT_ALL_BITS},
  {"GPS.1",    GPS_MOCK_TSS_KEY,    53, 57, INVERT_ALL_BITS},
  {"GPS.2",    GPS_MOCK_TSS_KEY,    58, 62, INVERT_ALL_BITS},
  {"GPS.3",    GPS_MOCK_TSS_KEY,    63, 67, INVERT_ALL_BITS}
};

#endif
