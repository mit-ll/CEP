//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
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
  cep_rsa(int seed, int verbose);  
  ~cep_rsa(); 

  int RunRsaTest(int maxLoop, int maxBytes);
  int RunRsaTest2(int maxLoop, int maxBytes);  
  void CleamMem(int maxEntries);
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
  uint64_t compute_manually(void);
  //
  uint32_t ModularMultiplication(uint32_t A, uint32_t B, uint32_t n);
  uint32_t ModularExponentiation(uint32_t A, uint32_t B, uint32_t n); // E = A**B mod (A=message,B=exponent, n=modulus)
  //
  // montgomery
  uint32_t MonPro2(int x, uint32_t A, uint32_t B, uint32_t M);
  uint32_t MonExp_R2L(uint32_t P, uint32_t E, uint32_t M);
  uint32_t MonExp_L2R(uint32_t P, uint32_t E, uint32_t M);  
  
  int mUseManualMod;
  int mAddZeros;
  int mUseExpBits;
};

//
#endif
