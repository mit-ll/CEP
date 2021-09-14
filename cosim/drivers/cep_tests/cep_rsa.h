//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cep_rsa.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    rsa test for CEP
// Notes:          
//************************************************************************

#ifndef cep_rsa_H
#define cep_rsa_H
// ---------------------------
#include "stdint.h"
#include "cep_crypto.h"

//
// Control bit mappings
//
#define EXPONENT_MEM_API_CS  0
#define EXPONENT_MEM_API_WR  1
#define EXPONENT_MEM_API_RST 2

#define MODULUS_MEM_API_CS   0
#define MODULUS_MEM_API_WR   1
#define MODULUS_MEM_API_RST  2

#define MESSAGE_MEM_API_CS   0
#define MESSAGE_MEM_API_WR   1
#define MESSAGE_MEM_API_RST  2

#define RESULT_MEM_API_CS    0
#define RESULT_MEM_API_RST   1

//
// CEP's RSA 
//
class cep_rsa : public cep_crypto { 
  // public stuffs
public: //
  // constructors
  //
  cep_rsa(int coreIndex, int seed, int verbose);  
  ~cep_rsa(); 

  int RunRsaTest(int maxLoop, int maxBytes);
  int RunRsaMemTest(int memMask, int sizeInBytes);
protected:
  //
  int GetAddZeros(void) { return mAddZeros; }
  void SetAddZeros(int on) { mAddZeros = on; }

  int GetUseExpBits(void) { return mUseExpBits; }
  void SetUseExpBits(int on) { mUseExpBits = on; }  
  int CountExpBits(uint8_t *exp, int expBytes);
  
  void LoadMessage(uint8_t *msg, int msgBytes, int wrEn) ;
  void LoadModulus(uint8_t *modulus, int modulusBytes, int wrEn);
  void LoadExponent(uint8_t *exponent, int exponentBytes, int wrEn);
  void ReadResult(uint8_t *result, int resultBytes) ;
  void Start(void);
  int  waitTilDone(int maxTO);
  int generate_key(void);
  uint64_t Byte2Word(uint8_t *buf);
  void Word2Byte(uint64_t word, uint8_t *buf);
  int compute_results(void);

  int mAddZeros;
  int mUseExpBits;
};

//
#endif
