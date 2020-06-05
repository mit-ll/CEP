//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      cep_md5.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    md5 test for CEP
// Notes:          
//************************************************************************

#ifndef cep_md5_H
#define cep_md5_H
// ---------------------------
#include "stdint.h"
#include "cep_crypto.h"
//
// CEP's MD5 
//
class cep_md5 : public cep_crypto { 
  // public stuffs
public: //
  // constructors
  //
  cep_md5(int seed, int verbose);  
  ~cep_md5(); 

  //
  int RunMd5Test(int maxLoop);

 protected:
  //
  int prepare_md5_key_N_text
    (
     uint8_t *input,           // input text
     uint8_t *output,           // output text
     int padding_enable,     // 1 = ON, 0 = OFF
     int length, int*outlen,
     int verbose);
  //
  void LoadInText(uint8_t *buf);
  void Clear(void);
  void Start(void);  
  int  waitTilReady(int maxTO);
  int  waitTilDone(int maxTO);
  void ReadOutText(void) ;
  int CheckHashText(uint8_t *buf);
};

//
#endif
