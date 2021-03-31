//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cep_sha256.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    sha256 test for CEP
// Notes:          
//************************************************************************

#ifndef cep_sha256_H
#define cep_sha256_H
// ---------------------------
#include "stdint.h"
#include "cep_crypto.h"
//
// CEP's SHA256 
//
class cep_sha256 : public cep_crypto { 
  // public stuffs
public: //
  // constructors
  //
  cep_sha256(int seed, int verbose);  
  ~cep_sha256(); 

  //
  int RunSha256Test(int maxLoop);

 protected:
  //
  int prepare_sha256_key_N_text
    (
     uint8_t *input,           // input text
     uint8_t *output,           // output text
     int padding_enable,     // 1 = ON, 0 = OFF
     int length, int*outlen,
     int verbose);
  //
  void LoadInText(uint8_t *buf);
  void Start(int firstTime);
  int  waitTilReady(int maxTO);
  int  waitTilDone(int maxTO);
  void ReadOutText(void) ;
  int CheckHashText(uint8_t *buf);
};

//
#endif
