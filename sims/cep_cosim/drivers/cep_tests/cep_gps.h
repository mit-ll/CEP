//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
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

#define X1A_POLYNOMIAL  0b110010100000
#define X1B_POLYNOMIAL  0b111110010011
#define X2A_POLYNOMIAL  0b111111011101
#define X2B_POLYNOMIAL  0b100110001110
#define X1A_PERIOD      4092
#define X1B_PERIOD      4093
#define X2A_PERIOD      4092
#define X2B_PERIOD      4093

#define CHIP_RATE ((uint64_t)10230000)

//
// CEP's GPS 
//
class cep_gps : public cep_aes { 
  // public stuffs
public: //
  // constructors
  //
  cep_gps(int coreIndex, int seed, int verbose);  
  cep_gps(int coreIndex, int seed, int staticPCodeInit, int verbose);
  ~cep_gps() {}; 

  int  GetSvNum ( ) { return mSvNum; }
  void SetSvNum (int svNum);
  void SetPcodeSpeed (uint16_t xn_cnt_speed, uint32_t z_cnt_speed);
  void SetPcodeXnInit (uint16_t x1a_initial, uint16_t x1b_initial, uint16_t x2a_initial, uint16_t x2b_initial);
  void LoadKey(void);  
  void Start(void);
  void BusReset(int assert);
  void BusReset(void);
  int  waitTilDone(int maxTO);
  

  void ResetCA_Code(void);
  void GenCA_Code(int svNum);
  int  ReadNCheckCA_Code(void);
  void GenP_Code(void);
  int  ReadNCheckP_Code(void);
  void ReadL_Code(void);  
  int RunSingle(void);
  //
  int RunGpsTest(int maxLoop);

protected:
  //
  int   mStaticPCodeInit;
  int   mSvNum;

  //X1, X2 Shift Registers:
  typedef struct LFSR {
    unsigned x: 12; // 12 bits
  } LFSR;

  int       xor_bits(unsigned x);
  uint8_t   x1a_shift(LFSR* x);
  uint8_t   x1b_shift(LFSR* x); 
  uint8_t   x2a_shift(LFSR* x);
  uint8_t   x2b_shift(LFSR* x);
  void      x_prep(void);
  uint8_t   x1_lookup(uint8_t * x_buf, uint64_t index);
  uint8_t   x2_lookup(uint8_t * x_buf, uint64_t index);
  uint8_t   x2_lookup_last(uint8_t * x_buf, uint64_t index);
  uint8_t   pcode_lookup(uint8_t * x_buf, uint64_t index, uint8_t prn);

  uint8_t   mExpCaCode[2];
  uint8_t   mActCaCode[2];
  uint16_t  m_xn_cnt_speed;
  uint32_t  m_z_cnt_speed;
  uint16_t  m_x1a_initial;
  uint16_t  m_x1b_initial;
  uint16_t  m_x2a_initial;
  uint16_t  m_x2b_initial;
  uint8_t   m_x_buf[4096];
  uint64_t  m_pcode_index;

  // CA Code SW implementation is a mirror of the hardware
  uint8_t   g1[11]; // [0] not used to match HW
  uint8_t   g2[11];
};

//
#endif
