//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cep_gps.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    gps test for CEP
// Notes:          
//************************************************************************

#if defined(BARE_MODE)
#else
#include <stdint.h>
#include "simPio.h"

#endif

#include <string.h>
#include "cep_gps.h"
#include "simdiag_global.h"
#include "portable_io.h"
#include "CEP.h"
#include "random48.h"

static uint16_t x_buf_size = 4096;

static uint16_t X1A_period = 4092;
static uint16_t X1B_period = 4093;
static uint16_t X2A_period = 4092;
static uint16_t X2B_period = 4093;

static uint32_t XN_cnt_speed = 1;
static uint32_t Z_cnt_speed  = 1;

static uint32_t X_a_full_count = 3750;
static uint32_t X_b_full_count = 3749;
static uint32_t X1A_loops      = ((int) ((X_a_full_count+XN_cnt_speed-1)/ XN_cnt_speed));
static uint32_t X1B_loops      = ((int) ((X_b_full_count+XN_cnt_speed-1)/ XN_cnt_speed));	

static uint64_t Z_full_period  = 403200;
static uint64_t Z_period       = ((Z_full_period + Z_cnt_speed - 1 ) / Z_cnt_speed);
static uint64_t Code_length    = (Z_period * X1A_loops * X1A_period); 
static uint64_t X1_period      = X1A_period * X1A_loops;
static uint64_t X2_period      = X1_period + 37;
static uint64_t X_b_hold       = ((X1A_period*X1A_loops) - (X1B_period*X1B_loops));

static uint64_t X2A_last_hold = ((Code_length % X2_period) % X2A_period);
static uint64_t X2B_last_hold = ((Code_length % X2_period) % X2B_period);


cep_gps::cep_gps(int coreIndex, int seed, int verbose) : cep_aes(coreIndex, seed, verbose) {
  init(coreIndex);
  mStaticPCodeInit = 0;

  // Initial values taken from gps.scala
  m_xn_cnt_speed  = 0x0001;
  m_z_cnt_speed   = 0x000000001;
  m_x1a_initial   = 0x0248;
  m_x1b_initial   = 0x0554;
  m_x2a_initial   = 0x0925;
  m_x2b_initial   = 0x0554;

}

cep_gps::cep_gps(int coreIndex, int seed, int staticPCodeInit, int verbose) : cep_aes(coreIndex, seed, verbose) {
  init(coreIndex);
  mStaticPCodeInit = staticPCodeInit;

  // Initial values taken from gps.scala
  m_xn_cnt_speed  = 0x0001;
  m_z_cnt_speed   = 0x000000001;
  m_x1a_initial   = 0x0248;
  m_x1b_initial   = 0x0554;
  m_x2a_initial   = 0x0925;
  m_x2b_initial   = 0x0554;
}

void cep_gps::SetSvNum (int svNum) {

  x_prep();

  if (svNum > MAX_SAT) {
    mSvNum = MIN_SAT;
  } else {
    mSvNum = svNum;
  }
  // Load SvNum
  cep_writeNcapture(GPS_SV_NUM, GetSvNum());
  //
}

//Default speed for normal operation is (1,1), higher speeds exist for validation purposes only.
void cep_gps::SetPcodeSpeed (uint16_t xn_cnt_speed, uint32_t z_cnt_speed) {
  m_xn_cnt_speed  &= xn_cnt_speed & 0xfff;
  m_z_cnt_speed   &= z_cnt_speed & 0x7ffff;
  uint64_t pcode_speed = m_xn_cnt_speed | (m_z_cnt_speed << 12);
  cep_writeNcapture(GPS_PCODE_SPEED, pcode_speed);
}

/* Defaults:
x1a = 0b001001001000
x1b = 0b010101010100
x2a = 0b100100100101
x2b = 0b010101010100
*/
void cep_gps::SetPcodeXnInit (uint16_t x1a_initial, uint16_t x1b_initial, uint16_t x2a_initial, uint16_t x2b_initial) {
  m_x1a_initial = x1a_initial & 0xfff;
  m_x1b_initial = x1b_initial & 0xfff;
  m_x2a_initial = x2a_initial & 0xfff;
  m_x2b_initial = x2b_initial & 0xfff;
  // Update values for fast loop
  X1A_period = 10;
  X1B_period = 9;
  X2A_period = 10;
  X2B_period = 9;

  XN_cnt_speed = 163;
  Z_cnt_speed  = 174763;

  X1A_loops      = ((int) ((X_a_full_count+XN_cnt_speed-1)/ XN_cnt_speed));
  X1B_loops      = ((int) ((X_b_full_count+XN_cnt_speed-1)/ XN_cnt_speed));
  X1_period      = X1A_period * X1A_loops;
  X2_period      = X1_period + 37;
  
  X_b_hold       = ((X1A_period*X1A_loops) - (X1B_period*X1B_loops));
  Z_period       = ((Z_full_period + Z_cnt_speed - 1 ) / Z_cnt_speed);
  Code_length    = (Z_period * X1A_loops * X1A_period);

  X2A_last_hold = ((Code_length % X2_period) % X2A_period);
  X2B_last_hold = ((Code_length % X2_period) % X2B_period);

  uint64_t pcode_xinitial = 0;
  pcode_xinitial |= (uint64_t)m_x1a_initial << 0;
  pcode_xinitial |= (uint64_t)m_x1b_initial << 12;
  pcode_xinitial |= (uint64_t)m_x2a_initial << 24;
  pcode_xinitial |= (uint64_t)m_x2b_initial << 36;
  cep_writeNcapture(GPS_PCODE_XINI, pcode_xinitial);
  uint64_t pcode_speed = XN_cnt_speed | (Z_cnt_speed << 12);
  cep_writeNcapture(GPS_PCODE_SPEED, pcode_speed); 
}


// Key[2,1,0] (64-bits each) = mKEY[0....23]
void cep_gps::LoadKey(void) {
  uint64_t word;
#ifdef BIG_ENDIAN
  for(int i = 0; i < mKeySize/8; i++) { //  8-bytes/word
    word = 0;
    for (int j=0;j<8;j++) {
      word = (word << 8) | (uint64_t)mKEY[i*8 + j];      
    }
    cep_writeNcapture(GPS_KEY_BASE + (i * BYTES_PER_WORD), word);
  }
#else
  for(int i = 0; i < mKeySize/8; i++) { //  8-bytes/word
    word = 0;
    for (int j=0;j<8;j++) {
      word = (word << 8) | (uint64_t)mKEY[i*8 + j];      
    }
    cep_writeNcapture(GPS_KEY_BASE + (((AES_KEY_WORDS - 1) - i) * BYTES_PER_WORD), word);
  }
#endif
}


void cep_gps::Start(void) {
  //

  cep_writeNcapture(GPS_GEN_NEXT, 0x1);
  cep_writeNcapture(GPS_GEN_NEXT, 0x0);

}

void cep_gps::BusReset(int assert) {
  cep_writeNcapture(GPS_RESET, 0x01);
}

void cep_gps::BusReset(void) {
  // need to extend at least for slow clock can catch      
  for (int i=0;i<5;i++) {
    cep_writeNcapture(GPS_RESET, 0x01);
  }
  cep_writeNcapture(GPS_RESET, 0x00);
  //
}

int cep_gps::waitTilDone(int maxTO) {
  if (GetVerbose(2)) {  LOGI("%s\n",__FUNCTION__); }
  return (cep_readNspin(GPS_GEN_DONE, 2, 2, maxTO)); 
}

int cep_gps::xor_bits(unsigned x) {
    return __builtin_parity(x);
}
uint8_t cep_gps::x1a_shift(LFSR* x) {
    uint8_t r = ((*x).x >> 11) & 1;
    uint8_t lsb = xor_bits((*x).x & X1A_POLYNOMIAL);
    (*x).x = ((*x).x<<1)+lsb;
    return r;
}
uint8_t cep_gps::x1b_shift(LFSR* x) {
    uint8_t r = ((*x).x >> 11) & 1;
    uint8_t lsb = xor_bits((*x).x & X1B_POLYNOMIAL);
    (*x).x = ((*x).x<<1)+lsb;
    return r;
}

uint8_t cep_gps::x2a_shift(LFSR* x) {
    uint8_t r = ((*x).x >> 11) & 1;
    uint8_t lsb = xor_bits((*x).x & X2A_POLYNOMIAL);
    (*x).x = ((*x).x<<1)+lsb;
    return r;
}

uint8_t cep_gps::x2b_shift(LFSR* x) {
    uint8_t r = ((*x).x >> 11) & 1;
    uint8_t lsb = xor_bits((*x).x & X2B_POLYNOMIAL);
    (*x).x = ((*x).x<<1)+lsb;
    return r;
}

void cep_gps::x_prep(void) { 
    // x_buf holds pre-calculated x[12][ab] values, 1 of each per byte.
    LFSR x1a_reg = {.x = m_x1a_initial};
    LFSR x1b_reg = {.x = m_x1b_initial};
    LFSR x2a_reg = {.x = m_x2a_initial};
    LFSR x2b_reg = {.x = m_x2b_initial};
    for (int i = 0; i < x_buf_size; i++) {
        uint8_t bits;
        bits  = x1a_shift(&x1a_reg) << 0;
        bits |= x1b_shift(&x1b_reg) << 1;
        bits |= x2a_shift(&x2a_reg) << 2;
        bits |= x2b_shift(&x2b_reg) << 3;
        m_x_buf[i] = bits;
    }

    // initial the PCode index
    m_pcode_index = 0;

}

uint8_t cep_gps::x1_lookup(uint8_t * x_buf, uint64_t index) {
    index = index % X1_period;
    uint8_t x1a = (x_buf[index % X1A_period]>>0) &1;

    if ( index >= X1_period - X_b_hold ){      
	index = X1B_period -1; //Hold logic for x1b
    }
    uint8_t x1b = (x_buf[index % X1B_period]>>1) &1;

    return x1a ^ x1b;
}

uint8_t cep_gps::x2_lookup(uint8_t * x_buf, uint64_t index) {
    uint64_t index_x2a = index % X2_period;
    uint64_t index_x2b = index_x2a;
    uint64_t index_a   = index % X1_period;
    uint64_t index_b   = index_a;

    if (index_x2a >= X1_period){
      index_x2a = X2A_period - 1; //Hold logic for x2a
    }
    uint8_t x2a = (x_buf[index_x2a % X2A_period]>>2) &1;

    if ( index_x2b >= X1_period - X_b_hold ){
      index_x2b = X2B_period - 1; //Hold logic for x2b
    }
    uint8_t x2b = (x_buf[index_x2b % X2B_period]>>3) &1;

    return x2a ^ x2b;
}

uint8_t cep_gps::x2_lookup_last(uint8_t * x_buf, uint64_t index) {
    uint64_t index_x2a = index % X2_period;
    uint64_t index_x2b = index_x2a;
    uint64_t index_a   = index % X1_period;
    uint64_t index_b   = index_a;
    
    if (index >= (Code_length - X2A_last_hold)){
	index_x2a = X2A_period - 1 ; //Hold logic for x2a
    }
    uint8_t x2a = (x_buf[index_x2a % X2A_period]>>2) &1;

    if ( index >= Code_length - X2B_last_hold){ 
       index_x2b = X2B_period - 1; //Hold logic for x2b
    }
    uint8_t x2b = (x_buf[index_x2b % X2B_period]>>3) &1;

    return x2a ^ x2b;
}

uint8_t cep_gps::pcode_lookup(uint8_t * x_buf, uint64_t index, uint8_t prn) {
    uint8_t day = (prn-1)/37;
    index += CHIP_RATE*86400*day;
    uint64_t index_x2 = (Code_length + index - prn) % Code_length; //pre add CODE_LENGTH, otherwise problems with index-prn being negative
    index %= Code_length;

    uint8_t p_x1 = x1_lookup(x_buf, index);
    uint8_t p_x2 = x2_lookup(x_buf, index_x2);
    if ((index_x2>= Code_length - X1A_period) ) { 
        p_x2 = x2_lookup_last(x_buf, index_x2); 
    }

    return p_x1 ^ p_x2;
}

// Generate an expected 128-bit P code value
void cep_gps::GenP_Code(void) {

   if (m_pcode_index == x_buf_size){
     m_pcode_index = 0;
  }

  for (int i = 0; i < 128; i+=4) {
    uint8_t code = 0;
    code |= pcode_lookup(m_x_buf, m_pcode_index++, mSvNum) << 3;
    code |= pcode_lookup(m_x_buf, m_pcode_index++, mSvNum) << 2;
    code |= pcode_lookup(m_x_buf, m_pcode_index++, mSvNum) << 1;
    code |= pcode_lookup(m_x_buf, m_pcode_index++, mSvNum) << 0;

    // Lower nibble
    if (i % 8 == 0)
      mSwPt[i/8] = (code & 0x0F) << 4;
    // Upper nibble
    else
      mSwPt[i/8] |= (code & 0x0F);
  }

} // void cep_gps::GenP_Code(void)

// 16 bytes = 2 words
// read P-code and use it as plainText for AES
int cep_gps::ReadNCheckP_Code(void) {
  
  int       errCnt    = 0;
  uint64_t  actPCode  = 0;

  // Read the P Code output from the Hardware
#ifdef BIG_ENDIAN
  for(int i = 0; i < mBlockSize/8; i++) { //  8-bytes/word
    actPCode = cep_readNcapture(GPS_P_BASE + i*8);
    for (int j=0;j<8;j++) {
      mHwPt[i*8 +j]= (actPCode >> (8*(7-j)) ) & 0xff;
    }
  }  
#else
  for(int i = 0; i < mBlockSize/8; i++) { //  8-bytes/word
    actPCode = cep_readNcapture(GPS_P_BASE + (1-i)*8);
    // As of 04/12/20: each 32 bits are swapped within 64-bit register (see gps.scala)
    for (int j=0;j<8;j++) {
      switch (j) {
      case 0 : mHwPt[i*8 +j]= (word >> (8*(3)) ) & 0xff; break;
      case 1 : mHwPt[i*8 +j]= (word >> (8*(2)) ) & 0xff; break;
      case 2 : mHwPt[i*8 +j]= (word >> (8*(1)) ) & 0xff; break;
      case 3 : mHwPt[i*8 +j]= (word >> (8*(0)) ) & 0xff; break;
      case 4 : mHwPt[i*8 +j]= (word >> (8*(7)) ) & 0xff; break;
      case 5 : mHwPt[i*8 +j]= (word >> (8*(6)) ) & 0xff; break;
      case 6 : mHwPt[i*8 +j]= (word >> (8*(5)) ) & 0xff; break;
      case 7 : mHwPt[i*8 +j]= (word >> (8*(4)) ) & 0xff; break;   
      }
    }
  }
#endif

  for (int i = 0; i < 16; i ++ ) {
    if (mHwPt[i] != mSwPt[i])
      errCnt ++;
  }

  if (errCnt)
    LOGE("%s: P Code error detected\n",__FUNCTION__);

  return errCnt;

}

void cep_gps::ReadL_Code(void) {
  uint64_t word;
#ifdef BIG_ENDIAN
  for(int i = 0; i < mBlockSize/8; i++) { //  8-bytes/word
    word = cep_readNcapture(GPS_L_BASE + i*8);
    for (int j=0;j<8;j++) {
      mHwCp[i*8 +j]= (word >> (8*(7-j)) ) & 0xff;
    }
  }  
#else
  for(int i = 0; i < mBlockSize/8; i++) { //  8-bytes/word
    word = cep_readNcapture(GPS_L_BASE + (1-i)*8);
    // As of 04/12/20: each 32 bits are swapped within 64-bit register (see gps.scala)
    for (int j=0;j<8;j++) {
      switch (j) {
      case 0 : mHwCp[i*8 +j]= (word >> (8*(3)) ) & 0xff; break;
      case 1 : mHwCp[i*8 +j]= (word >> (8*(2)) ) & 0xff; break;
      case 2 : mHwCp[i*8 +j]= (word >> (8*(1)) ) & 0xff; break;
      case 3 : mHwCp[i*8 +j]= (word >> (8*(0)) ) & 0xff; break;
      case 4 : mHwCp[i*8 +j]= (word >> (8*(7)) ) & 0xff; break;
      case 5 : mHwCp[i*8 +j]= (word >> (8*(6)) ) & 0xff; break;
      case 6 : mHwCp[i*8 +j]= (word >> (8*(5)) ) & 0xff; break;
      case 7 : mHwCp[i*8 +j]= (word >> (8*(4)) ) & 0xff; break;   
      }
    }
  }
#endif
}


void cep_gps::ResetCA_Code()
{
  if (GetVerbose())
    LOGI("Reset CA Code\n");
  for (int i = 0; i < 11; i++) {
    g1[i] = 1;
    g2[i] = 1;
  }
}

//
// Compute C/A code (use HW code for now)
//
void cep_gps::GenCA_Code(int svNum)
{
  int CACode  = 0;
  int chip    = 0;
#if defined(BARE_MODE)
#else
  //
  if (GetVerbose(2)) {
    LOGI("G1=0x%04x G2=0x%04x\n",
   (((g1[10] & 0x1) << 9) |
    ((g1[9] & 0x1) << 8) |    
    ((g1[8] & 0x1) << 7) |    
    ((g1[7] & 0x1) << 6) |    
    ((g1[6] & 0x1) << 5) |    
    ((g1[5] & 0x1) << 4) |    
    ((g1[4] & 0x1) << 3) |    
    ((g1[3] & 0x1) << 2) |    
    ((g1[2] & 0x1) << 1) |    
    ((g1[1] & 0x1) << 0)),
   (((g2[10] & 0x1) << 9) |
    ((g2[9] & 0x1) << 8) |    
    ((g2[8] & 0x1) << 7) |    
    ((g2[7] & 0x1) << 6) |    
    ((g2[6] & 0x1) << 5) |    
    ((g2[5] & 0x1) << 4) |    
    ((g2[4] & 0x1) << 3) |    
    ((g2[3] & 0x1) << 2) |    
    ((g2[2] & 0x1) << 1) |    
    ((g2[1] & 0x1) << 0)));
  }
  for (int i=0;i<13;i++) {
    chip = 0;
    switch (svNum) {
    case 1 :
      chip = g1[10] ^ g2[2] ^ g2[6]; break;
    case 2 :
      chip = g1[10] ^ g2[3] ^ g2[7]; break;
    case 3 :
      chip = g1[10] ^ g2[4] ^ g2[8]; break;
    case 4 :
      chip = g1[10] ^ g2[5] ^ g2[9]; break;
    case 5 :
      chip = g1[10] ^ g2[1] ^ g2[9]; break;
    case 6 :
      chip = g1[10] ^ g2[2] ^ g2[10]; break;
    case 7 :
      chip = g1[10] ^ g2[1] ^ g2[8]; break;
    case 8 :
      chip = g1[10] ^ g2[2] ^ g2[9]; break;
    case 9 :
      chip = g1[10] ^ g2[3] ^ g2[10]; break;
    case 10:
      chip = g1[10] ^ g2[2] ^ g2[3]; break;
    case 11:
      chip = g1[10] ^ g2[3] ^ g2[4]; break;
    case 12:
      chip = g1[10] ^ g2[5] ^ g2[6]; break;
    case 13:
      chip = g1[10] ^ g2[6] ^ g2[7]; break;
    case 14:
      chip = g1[10] ^ g2[7] ^ g2[8]; break;
    case 15:
      chip = g1[10] ^ g2[8] ^ g2[9]; break;
    case 16:
      chip = g1[10] ^ g2[9] ^ g2[10]; break;
    case 17:
      chip = g1[10] ^ g2[1] ^ g2[4]; break;
    case 18:
      chip = g1[10] ^ g2[2] ^ g2[5]; break;
    case 19:
      chip = g1[10] ^ g2[3] ^ g2[6]; break;
    case 20:
      chip = g1[10] ^ g2[4] ^ g2[7]; break;
    case 21:
      chip = g1[10] ^ g2[5] ^ g2[8]; break;
    case 22:
      chip = g1[10] ^ g2[6] ^ g2[9]; break;
    case 23:
      chip = g1[10] ^ g2[1] ^ g2[3]; break;
    case 24:
      chip = g1[10] ^ g2[4] ^ g2[6]; break;
    case 25:
      chip = g1[10] ^ g2[5] ^ g2[7]; break;
    case 26:
      chip = g1[10] ^ g2[6] ^ g2[8]; break;
    case 27:
      chip = g1[10] ^ g2[7] ^ g2[9]; break;
    case 28:
      chip = g1[10] ^ g2[8] ^ g2[10]; break;
    case 29:
      chip = g1[10] ^ g2[1] ^ g2[6]; break;
    case 30:
      chip = g1[10] ^ g2[2] ^ g2[7]; break;
    case 31:
      chip = g1[10] ^ g2[3] ^ g2[8]; break;
    case 32:
      chip = g1[10] ^ g2[4] ^ g2[9]; break;
    case 33:
      chip = g1[10] ^ g2[5] ^ g2[10]; break;
    case 34:
      chip = g1[10] ^ g2[4] ^ g2[10]; break;
    case 35:
      chip = g1[10] ^ g2[1] ^ g2[7]; break;
    case 36:
      chip = g1[10] ^ g2[2] ^ g2[8]; break;
    case 37:
      chip = g1[10] ^ g2[4] ^ g2[10]; break;
    }
    CACode = (CACode<<1) | (chip & 0x1);
    // next [1]
    g1[0] = g1[3] ^ g1[10];
    g2[0] = g2[2] ^ g2[3] ^ g2[6] ^ g2[8] ^ g2[9] ^ g2[10];    
    for (int j=10;j>0;j--) {
      //g1[10:1] <= {g1[9:1], g1[3] ^ g1[10]};
      //g2[10:1] <= {g2[9:1], g2[2] ^ g2[3] ^ g2[6] ^ g2[8] ^ g2[9] ^ g2[10]};
      g1[j] = g1[j-1]; // shift
      g2[j] = g2[j-1];
    }
  }
#endif
  
  mExpCaCode[0] = CACode & 0xFF;
  mExpCaCode[1] = (CACode >> 16) & 0xFF;

}

int cep_gps::ReadNCheckCA_Code(void)
{
  int errCnt    = 0;
  uint64_t word = 0;

  // CA Code is only 13-bits
  word            = cep_readNcapture(GPS_CA_BASE);
  mActCaCode[0]   = word & 0xFF;
  mActCaCode[1]   = (word >> 16) & 0x1F;

  for (int i = 0; i < 2; i ++ ) {
    if (mActCaCode[i] != mExpCaCode[i])
      errCnt ++;
  }

  if (errCnt)
    LOGE("%s: CA Code error detected\n",__FUNCTION__);

  return errCnt;
}

int cep_gps::RunSingle() {
  int outLen = mBlockSize;
  int errCnt  = 0;
  int errCnt2 = 0;

  // Start the code generators  
  Start();

  // Wait until the GPS is done (L Code valid)  
  waitTilDone(500);

  // Read and Check the CA Code output
  GenCA_Code(GetSvNum());
  errCnt += ReadNCheckCA_Code();

  // Read the PCode ouput.
  GenP_Code();
  errCnt += ReadNCheckP_Code();

  // Read and Check the expected P-Code output
  ReadL_Code();
  errCnt2 += cryptopp_aes192_ecb_encryption (mHwPt, mSwCp, GetVerbose());
  errCnt2 += CheckCipherText();
  errCnt  += errCnt2;

  // Used to localize error detection to the L code
  if (errCnt2)
    LOGE("%s: L Code error detected\n",__FUNCTION__);

  // Print
  if ((errCnt && !GetExpErr()) || GetVerbose(2)) {
    PrintMe("Exp CA-Code  ", &(mExpCaCode[0]), 2);
    PrintMe("Act CA-Code  ", &(mActCaCode[0]), 2);
    PrintMe("Exp P-Code   ", &(mSwPt[0]), mBlockSize);
    PrintMe("Act P-code   ", &(mHwPt[0]), mBlockSize);
    PrintMe("L-Code Key   ", &(mKEY[0]), mKeySize);
    PrintMe("Exp L-Code   ", &(mSwCp[0]), mBlockSize);
    PrintMe("Act L-Code   ", &(mHwCp[0]), mBlockSize);
  }

  return errCnt;
}

int cep_gps::RunGpsTest(int maxLoop) {

  // Need to take it out of reset to support unit sim due to LLKI!!!
  mErrCnt = 0;
  BusReset();
  SetSvNum(0);   // so GPS can detect a change

  //Initialize mKEY with 0xAAAA... for first iteration
  for (int i = 0; i < 192 / 8 ; i++) {
    mKEY[i] = 0xAA;
  }
  //Check first 128 bits of all SAT numbers
  //Except sat=1, for that get 2nd 128 bits for a total of 256.
  for (int i = 1 ; i <= maxLoop; i++) {
    if (GetVerbose()) {
      LOGI("%s: Loop %d\n",__FUNCTION__,i);
    }

    LoadKey();
    ResetCA_Code();
    SetSvNum(i);

    mErrCnt += RunSingle();

    if (i==1) { //Initialize mKEY with 0x555... for second iteration (to force coverage), randomize afterward
      for (int i=0;i<192/8;i++) {
        mKEY[i] = 0x55;
      }
    } else {
      RandomGen(mKEY, GetKeySize()); //Pick a random encryption key for the next iteration of the run
    }

    MarkSingle(i);

    if (mErrCnt) break;

  } // for (int i=1 ; i <= maxLoop; i++)
  if (!mErrCnt) {
    //HW Coverage test:
    SetSvNum(0);   // so GPS can detect a change
    RandomGen(mKEY, GetKeySize());
    LoadKey();
    
    // Static mode, the initialization vectors will NOT be changed
    if (!mStaticPCodeInit) {
      SetPcodeXnInit(120, 3666, 766, 1474); //_A loops over 10 values, _B loops over 9 values.
    } // if (!mLegacyPCode)

    ResetCA_Code();
    SetSvNum(1);
    
    //Need to record a total of 3*24*10 = 720 bits total. This requires 6 loops.
    for (int i = 0; i <= 5; i++) { 
      if (GetVerbose()) {
        LOGI("%s: Coverage Loop %d\n",__FUNCTION__,i);
      }
      mErrCnt += RunSingle();

      if (mErrCnt) break;
    }
  }
  //
  return mErrCnt;
}

