//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cep_rsa.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    rsa test for CEP
// Notes:          
//************************************************************************
#if defined(BARE_MODE)
#else

#include <cryptopp/cryptlib.h>
#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>
#include <cryptopp/rsa.h>
#include "simPio.h"

#endif

#include <string.h>
#include "cep_rsa.h"
#include "simdiag_global.h"
#include "portable_io.h"
#include "CEP.h"
#include "cepregression.h"
#include "random48.h"
//
//

// Aliasing
#define EXPSTR  mKEY
#define MSGSTR  mSwPt
#define MODSTR  mHwPt
#define ERESSTR mSwCp
#define ARESSTR mHwCp
//
#define SetExpSize  SetKeySize
#define GetExpSize  GetKeySize

#define SetModSize  SetBlockSize
#define GetModSize  GetBlockSize

cep_rsa::cep_rsa(int coreIndex, int seed, int verbose) {
  init(coreIndex);
  SetSeed(seed);
  SetVerbose(verbose);
  SetAddZeros(1);       // 32bit become 32+32, etc..
  SetUseExpBits(0);
  SetExpSize(4);        // fix at 32-bits
}
//
cep_rsa::~cep_rsa()  {
}

uint64_t cep_rsa::Byte2Word(uint8_t *buf) {
  uint64_t word;
#ifdef BIG_ENDIAN
  word = (((uint64_t)buf[0] << 24) |
	  ((uint64_t)buf[1] << 16) |
	  ((uint64_t)buf[2] <<  8) |
	  ((uint64_t)buf[3] <<  0) );
#else
  word = (((uint64_t)buf[0] <<  0) |
	  ((uint64_t)buf[1] <<  8) |
	  ((uint64_t)buf[2] << 16) |
	  ((uint64_t)buf[3] << 24) );  
#endif
  return word;
}

void cep_rsa::Word2Byte(uint64_t word, uint8_t *buf) {
  buf[0] = (word >> 24) & 0xff;
  buf[1] = (word >> 16) & 0xff;
  buf[2] = (word >>  8) & 0xff;
  buf[3] = (word >>  0) & 0xff;  
}

//
// Message
//
void cep_rsa::LoadMessage(uint8_t *msg, int msgBytes, int wrEn) {
  uint64_t word;
  int wCnt = msgBytes/4 + ((msgBytes & 0x3) ? 1 : 0);
  int j=0;
  if (GetVerbose(2)) { LOGI("%s wCnt=%d\n",__FUNCTION__,wCnt); }
  // 
  cep_writeNcapture(RSA_ADDR_MESSAGE_CTRL , 1<<MESSAGE_MEM_API_RST);
  cep_writeNcapture(RSA_ADDR_MESSAGE_CTRL , 0x0000000000000000);
  //
  if (wrEn) {
    if (GetAddZeros()) wCnt++;
    //
    for (int i=0; i<wCnt;i++) {
      if (GetAddZeros() && (i==0)) {
	word = 0;
      } else {
	word = Byte2Word(&(msg[j]));
	j += 4;
      }
      //
      cep_writeNcapture(RSA_ADDR_MESSAGE_DATA , word);
      cep_writeNcapture(RSA_ADDR_MESSAGE_CTRL , (1 << MESSAGE_MEM_API_CS) | (wrEn<<MESSAGE_MEM_API_WR));
      cep_writeNcapture(RSA_ADDR_MESSAGE_CTRL , 0x0000000000000000);
    }
  } else {
    // readback for checking
    for (int i=0; i<wCnt;i++) {
      // read from current
      word = cep_readNcapture(RSA_ADDR_MESSAGE_DATA); // bit[63:32] are read data
      Word2Byte(word>>32,&(msg[j]));
      j += 4;
      // issue for next
      cep_writeNcapture(RSA_ADDR_MESSAGE_CTRL , (1 << MESSAGE_MEM_API_CS));
      cep_writeNcapture(RSA_ADDR_MESSAGE_CTRL , 0x0000000000000000);
    }
  }
}

//
// Modulus
//
void cep_rsa::LoadModulus(uint8_t *modulus, int modulusBytes, int wrEn) {
  uint64_t word;
  int wCnt = modulusBytes/4 + ((modulusBytes & 0x3) ? 1 : 0);
  int j=0;
  if (GetVerbose(2)) { LOGI("%s wCnt=%d\n",__FUNCTION__,wCnt); }  
  // 
  cep_writeNcapture(RSA_ADDR_MODULUS_CTRL , 1<<MODULUS_MEM_API_RST);
  cep_writeNcapture(RSA_ADDR_MODULUS_CTRL , 0x0000000000000000);
  //
  if (wrEn) {
    if (GetAddZeros()) wCnt++;  
    for (int i=0; i<wCnt;i++) {
      if (GetAddZeros() && (i==0)) {
	word = 0;
      } else {    
	word = Byte2Word(&(modulus[j]));    
	j += 4;
      }
      cep_writeNcapture(RSA_ADDR_MODULUS_DATA , word);
      cep_writeNcapture(RSA_ADDR_MODULUS_CTRL , (1 << MODULUS_MEM_API_CS) | (wrEn<<MODULUS_MEM_API_WR));
      cep_writeNcapture(RSA_ADDR_MODULUS_CTRL , 0x0000000000000000);
    }
    cep_writeNcapture(RSA_ADDR_MODULUS_LENGTH  , wCnt);
  } else {
    for (int i=0; i<wCnt;i++) {    
      // read from current
      word = cep_readNcapture(RSA_ADDR_MODULUS_DATA); // bit[63:32] are read data
      Word2Byte(word>>32,&(modulus[j]));
      j += 4;
      // issue for next
      cep_writeNcapture(RSA_ADDR_MODULUS_CTRL , (1 << MODULUS_MEM_API_CS));
      cep_writeNcapture(RSA_ADDR_MODULUS_CTRL , 0x0000000000000000);
    }
  }
}

//
// Exponent
//
void cep_rsa::LoadExponent(uint8_t *exponent, int exponentBytes, int wrEn) {
  uint64_t word;
  int wCnt = exponentBytes/4 + ((exponentBytes & 0x3) ? 1 : 0);
  int j=0;
  if (GetVerbose(2)) { LOGI("%s wCnt=%d\n",__FUNCTION__,wCnt); }    
  // 
  cep_writeNcapture(RSA_ADDR_EXPONENT_CTRL , 1<<EXPONENT_MEM_API_RST);
  cep_writeNcapture(RSA_ADDR_EXPONENT_CTRL , 0x0000000000000000);
  //
  // add 32-bits in front to extend the size!!
  //
  if (wrEn) {
    for (int i=0; i<wCnt;i++) {
      word = Byte2Word(&(exponent[j]));        
      j += 4;
      //
      cep_writeNcapture(RSA_ADDR_EXPONENT_DATA , word);
      cep_writeNcapture(RSA_ADDR_EXPONENT_CTRL , (1 << EXPONENT_MEM_API_CS) | (wrEn<<EXPONENT_MEM_API_WR));
      cep_writeNcapture(RSA_ADDR_EXPONENT_CTRL , 0x0000000000000000);
    }
    int bitCnt = exponentBytes*8;
    if (GetUseExpBits()) {
      bitCnt = CountExpBits(exponent,exponentBytes);
    }
    cep_writeNcapture(RSA_ADDR_EXPONENT_LENGTH  , bitCnt); // wCnt*32);    
  }
  else {
    for (int i=0; i<wCnt;i++) {    
      // read from current
      word = cep_readNcapture(RSA_ADDR_EXPONENT_DATA); // bit[63:32] are read data
      Word2Byte(word>>32,&(exponent[j]));
      j += 4;
      // next
      cep_writeNcapture(RSA_ADDR_EXPONENT_CTRL , (1 << EXPONENT_MEM_API_CS));
      cep_writeNcapture(RSA_ADDR_EXPONENT_CTRL , 0x0000000000000000);
    }
  }
}

//
// Result
//
void cep_rsa::ReadResult(uint8_t *result, int resultBytes) {
  uint32_t word;
  int wCnt = resultBytes/4 + ((resultBytes & 0x3) ? 1 : 0);
  int j=0;
  if (GetVerbose(2)) { LOGI("%s: wCnt=%d\n",__FUNCTION__,wCnt); }  
  // 
  cep_writeNcapture(RSA_ADDR_RESULT_CTRL , 1<<RESULT_MEM_API_RST);
  cep_writeNcapture(RSA_ADDR_RESULT_CTRL , 0x0000000000000000);
  //
  if (GetAddZeros()) wCnt++; 
  for (int i=0; i<wCnt;i++) {
    cep_writeNcapture(RSA_ADDR_RESULT_CTRL , (1 << RESULT_MEM_API_CS));
    cep_writeNcapture(RSA_ADDR_RESULT_CTRL , 0);
    word = cep_readNcapture(RSA_ADDR_RESULT_DATA);
    if (!GetAddZeros() || (GetAddZeros() && (i > 0))) {
      Word2Byte(word,&(result[j]));
      //
      j += 4;
    }
  }
}

void cep_rsa::Start(void) {
  if (GetVerbose(2)) { LOGI("%s\n",__FUNCTION__); }  
  cep_writeNcapture(RSA_ADDR_CTRL            , 0x0000000000000002);
  cep_writeNcapture(RSA_ADDR_CTRL            , 0x0000000000000000);
}

int cep_rsa::waitTilDone(int maxTO) {
  if (GetVerbose(2)) { LOGI("%s: to=%d\n",__FUNCTION__,maxTO); }
  return cep_readNspin(RSA_ADDR_STATUS, 1, maxTO);
}

// Use Crypto++ to compute c = m^e mod n;
int cep_rsa::compute_results(void) {
#if defined(BARE_MODE)
#else

  using namespace CryptoPP;
  AutoSeededRandomPool rng;

  // Declare and initialize the Modulus and Exponent Integers
  Integer e ((const byte*)EXPSTR, GetExpSize());
  Integer n ((const byte*)MODSTR, GetModSize());

  // Initialize the public key and validate as a sanity check
  RSAFunction pubKey;
  pubKey.Initialize(n, e);

  // Validate the public key
  if (!pubKey.Validate(rng, 2)) {
    mErrCnt++;
    if (!GetExpErr()) {
      LOGE("%s ERROR Public Key is not valid\n",__FUNCTION__);
    }
  } // if (!pubKey.Validate(rng, 2))

  // Report that the function was called
  if (GetVerbose(2)) {
    LOGI("%s\n",__FUNCTION__);
  }

  // Create Integer datatypes for message and result
  Integer msgNum ((const byte *)MSGSTR, GetMsgSize());
  Integer resNum;

  // Perform the encryption
  resNum = pubKey.ApplyFunction(msgNum);

  // Convert the result back to a byte array
  resNum.Encode((byte *)ERESSTR, (size_t)GetMsgSize());

#endif
  return mErrCnt;
}

// Use Crypto++ to create a public key
int cep_rsa::generate_key(void) {
#if defined(BARE_MODE)
#else  

  using namespace CryptoPP;
  AutoSeededRandomPool rng;

  // Report that the function was called
  if (GetVerbose(2)) {
    LOGI("%s\n",__FUNCTION__);
  }

  // Initialize our Public Exponent Integer
  Integer e ((const byte*)EXPSTR, GetExpSize());

  // Generate the private key
  InvertibleRSAFunction privKey;
  privKey.Initialize(rng, GetModSize() * 8, e);

  // Validate the private key
  if (!privKey.Validate(rng, 2)) {
    mErrCnt++;
    if (!GetExpErr()) {
      LOGE("%s ERROR Private Key is not valid\n",__FUNCTION__);
    }
  } // if (!privKey.Validate(rng, 2))

  // Extract the modulus and put it back in the MODSTR array
  const Integer& n    = privKey.GetModulus();  

  // Initialize the public key and validate as a sanity check
  RSAFunction pubKey;
  pubKey.Initialize(n, e);

  // Validate the public key
  if (!pubKey.Validate(rng, 2)) {
    mErrCnt++;
    if (!GetExpErr()) {
      LOGE("%s ERROR Public Key is not valid\n",__FUNCTION__);
    }
  } // if (!pubKey.Validate(rng, 2))

  n.Encode((byte *)MODSTR, GetModSize());

#endif
  return mErrCnt;
}

//
// Run the RSA Test
//
int cep_rsa::RunRsaTest(int maxLoop, int maxBytes) {

  SetAddZeros(1);
  SetUseExpBits(1);
  
  if (GetVerbose()) { 
    LOGI("%s: maxLoop=%d\n",__FUNCTION__,maxLoop);
  } // if (GetVerbose())

  // Used to define the number of bytes for the Modulus and Message
  // Must be a multiple of 4
  int bCnt = 0;
  for (int i = 0; i < maxLoop; i++) { //  
 
    // Takes 10+ hours just to run 128-bit encrytion
    bCnt += 4;
    if (bCnt > maxBytes) {bCnt = 4;}
    
    if (GetVerbose()) {
      LOGI("%s: Loop %d. bCnt=%d\n",__FUNCTION__,i,bCnt);
    }    
 
    // 32-bits, 64, 96 ...
    SetExpSize(4);
    SetModSize(bCnt);   
    SetMsgSize(bCnt);   // modulus & message
 
    // generate random message, exp and modulus
    // NOTE: exp should be odd
    RandomGen(MSGSTR, GetMsgSize());

    // Initialize the Public Exponent
    for (int z = 0; z < GetExpSize(); z++) { 
      EXPSTR[z] = 0;
    }
    EXPSTR[3] = 0x11;

    // Generate the Public Key
    mErrCnt += generate_key(); 

    // Break if there is an error
    if (mErrCnt) break;

    // Print debug info
    if ((mErrCnt && !GetExpErr()) || GetVerbose(2)) {
      PrintMe("Exponent" , EXPSTR, GetExpSize());
      PrintMe("Modulus " , MODSTR, GetModSize());
      PrintMe("Message " , MSGSTR, GetMsgSize());
    }

    // Load the operands into the RSA core and perform encryption

    LoadExponent(EXPSTR, GetExpSize(),1);    
    LoadModulus(MODSTR, GetModSize(),1);
    LoadMessage(MSGSTR, GetMsgSize(),1);
    Start();
    if (mErrCnt) break;    
    mErrCnt += waitTilDone(((GetMsgSize()/4)+1) * 20000); // about 16K+ cycle per 32-bits (64-bit actual)
    if (!mErrCnt) {
      ReadResult(ARESSTR,GetMsgSize());
    }

    // Generate the expected results
    mErrCnt += compute_results();

    // Print debug info
    if ((mErrCnt && !GetExpErr()) || GetVerbose(2)) {
      PrintMe("Exponent" , EXPSTR,  GetExpSize());
      PrintMe("Modulus " , MODSTR,  GetModSize());
      PrintMe("Message " , MSGSTR,  GetMsgSize());
      PrintMe("ExpCp   "  ,ERESSTR, GetMsgSize());
      PrintMe("ActCp   "  ,ARESSTR, GetMsgSize());      
    }
  
    // Check results against Crypto++
    mErrCnt += CheckCipherText(); 

    MarkSingle(i);
    if (mErrCnt) break;
  }
  return mErrCnt;
}

int cep_rsa::CountExpBits(uint8_t *exp, int expBytes) {
  int bitCnt = expBytes * 8;
  for (int i=0;i<expBytes;i++) {
    for (int j=7;j>=0;j--) { // start from MSB, assuming BE
      if ((1 << j) & exp[i]) { // stop as soon as bit=1 is found
	return bitCnt;
      }
      else {
	bitCnt--;
      }
    }
  }
  return bitCnt;
}

//
// To test the RSA memory
//
int cep_rsa::RunRsaMemTest(int memMask, int sizeInBytes) {
  //
  SetAddZeros(0); // 32bit become 32+32, etc..
  //
  // Just for coverage
  //
  cep_writeNcapture(RSA_ADDR_EXPONENT_LENGTH  , -1);
  cep_writeNcapture(RSA_ADDR_EXPONENT_LENGTH  ,  0);
  cep_writeNcapture(RSA_ADDR_MODULUS_LENGTH   , -1);
  cep_writeNcapture(RSA_ADDR_MODULUS_LENGTH   ,  0);  
  //
  // Modulus memory
  //
  if (memMask & 0x1) {
    if (GetVerbose()) {
      LOGI("%s: Testing Modulus Memory bytes=%d\n",__FUNCTION__,sizeInBytes);
    }
    SetBlockSize(sizeInBytes);
    RandomGen(mSwPt, GetBlockSize());
    LoadModulus(mSwPt, GetBlockSize(), 1);
    LoadModulus(mHwPt, GetBlockSize(), 0);  
    mErrCnt += CheckPlainText();
    if (mErrCnt && !GetExpErr()) {
      LOGI("%s: Testing Modulus Memory bytes=%d error=%d\n",__FUNCTION__,sizeInBytes,mErrCnt);
      PrintMe("expMem "  ,mSwPt, GetBlockSize());
      PrintMe("actMem "  ,mHwPt, GetBlockSize());    
    }
    if (mErrCnt) return mErrCnt;
  }
  //
  // Exponent memory
  //
  if (memMask & 0x2) {  
    if (GetVerbose()) {
      LOGI("%s: Testing Exponent Memory bytes=%d\n",__FUNCTION__,sizeInBytes);
    }
    SetBlockSize(sizeInBytes);
    RandomGen(mSwPt, GetBlockSize());
    LoadExponent(mSwPt, GetBlockSize(), 1);
    LoadExponent(mHwPt, GetBlockSize(), 0);  
    mErrCnt += CheckPlainText();
    if (mErrCnt && !GetExpErr()) {
      LOGI("%s: Testing Exponent Memory bytes=%d error=%d\n",__FUNCTION__,sizeInBytes,mErrCnt);
      PrintMe("expMem "  ,mSwPt, GetBlockSize());
      PrintMe("actMem "  ,mHwPt, GetBlockSize());    
    }    
    if (mErrCnt) return mErrCnt;
  }
  //
  // message memory
  //
  if (memMask & 0x4) {  
    if (GetVerbose()) {
      LOGI("%s: Testing Message Memory bytes=%d\n",__FUNCTION__,sizeInBytes);
    }
    SetBlockSize(sizeInBytes);
    RandomGen(mSwPt, GetBlockSize());
    LoadMessage(mSwPt, GetBlockSize(), 1);
    LoadMessage(mHwPt, GetBlockSize(), 0);  
    mErrCnt += CheckPlainText();
    if (mErrCnt && !GetExpErr()) {
      LOGI("%s: Testing Message Memory bytes=%d error=%d\n",__FUNCTION__,sizeInBytes,mErrCnt);
      PrintMe("expMem "  ,mSwPt, GetBlockSize());
      PrintMe("actMem "  ,mHwPt, GetBlockSize());    
    }
  }
  return mErrCnt;
}


