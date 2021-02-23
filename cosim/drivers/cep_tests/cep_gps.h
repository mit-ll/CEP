//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cep_gps.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    gps test for CEP
// Notes:          
//************************************************************************
#ifndef cep_gps_H
#define cep_gps_H
// ---------------------------
#include "stdint.h"
#include "cep_aes.h"

#define MIN_SAT 1
#define MAX_SAT 37
//
// CEP's GPS 
//
class cep_gps : public cep_aes { 
  // public stuffs
public: //
  // constructors
  //
  cep_gps(int seed, int verbose);  
  ~cep_gps() {}; 

  int  GetSvNum ( ) { return mSvNum; }
  void SetSvNum (int svNum) ;  
  void Start(void);
  void BusReset(void);
  int waitTilDone(int maxTO);
  int GetCA_code(int svNum);
  int ReadNCheck_CA_Code(int mask);
  void Read_PCode(void);
  void Read_LCode(void);  
  //
  int RunGpsTest(int maxLoop);

 protected:
  //
  int mSvNum;
  int mExpCaCode;
  int mActCaCode;  
  // icode/pcode
  int g1[11]; // [0] not used to match HW
  int g2[11];
};

//
#endif
