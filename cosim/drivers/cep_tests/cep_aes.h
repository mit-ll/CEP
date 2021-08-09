//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cep_aes.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    aes test for CEP
// Notes:          
//************************************************************************

#ifndef cep_aes_H
#define cep_aes_H
// ---------------------------
#include "stdint.h"
#include "cep_crypto.h"
//
// CEP's AES 
//
class cep_aes : public cep_crypto { 
  // public stuffs
public: //
  // constructors
  //
  cep_aes(int seed, int verbose);  
  ~cep_aes() {}; 

  int RunAes192Test(int maxLoop);

 protected:
  //
  int cryptopp_aes192_ecb_encryption
    (
     uint8_t *input,           // input text packet
     uint8_t *output,          // output cipher packet
     int verbose);
  //
  void LoadKey(void);
  void LoadPlaintext(void);
  void StartEncrypt(void);
  int waitTilDone(int maxTO);
  void ReadCiphertext(void);
  //
  
};

//
#endif
