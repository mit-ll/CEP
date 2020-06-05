//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
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
  int openssl_aes192_ecb_encryption
    (
     uint8_t *input,           // input text packet
     uint8_t *output,           // output cipher packet
     int padding_enable,     // 1 = ON, 0 = OFF
     int length, int*outlen,
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
