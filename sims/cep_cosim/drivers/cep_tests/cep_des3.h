//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cep_des3.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    des3 test for CEP
// Notes:          
//************************************************************************

#ifndef cep_des3_H
#define cep_des3_H
// ---------------------------
#include "stdint.h"
#include "cep_crypto.h"
//
// CEP's DES3 
//
class cep_des3 : public cep_crypto { 
  // public stuffs
public: //
  // constructors
  //
  cep_des3(int coreIndex, int seed, int verbose);  
  ~cep_des3(); 

  void SetDecrypt(int decrypt) { mDecrypt = decrypt; }
  int GetDecrypt() { return mDecrypt; }

  //
  int RunDes3Test(int maxLoop);

protected:
  //
  // Routine only supports a single DES block
  //
  int prepare_des3_key_N_text
    (
     uint8_t *input,        // input text
     uint8_t *output,       // output text
     int verbose);
  //
  void LoadKey(void);
  void LoadInText(void);
  void Start(void);
  void SetMode(void);
  int waitTilDone(int maxTO);
  void ReadOutText(void);
  uint8_t MakeOddByte(uint8_t bin);
  void AdjustParity();
  //
  int mDecrypt; // 1=decrypt, 0=encrypt
  //
  uint64_t mHwKEY[3];   // pack into 56-bit with no parity
  
};

//
#endif
