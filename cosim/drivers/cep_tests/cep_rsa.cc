//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cep_rsa.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    rsa test for CEP
// Notes:          
//************************************************************************
#if defined(BARE_MODE)
#else
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>

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

cep_rsa::cep_rsa(int seed, int verbose) {
  init();  
  SetSeed(seed);
  SetVerbose(verbose);
  mUseManualMod = 0;
  SetAddZeros(1); // 32bit become 32+32, etc..
  SetUseExpBits(0);
  SetExpSize(4); // fix at 32-bits
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
  if (GetVerbose()) { LOGI("%s wCnt=%d\n",__FUNCTION__,wCnt); }
  // 
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_CTRL , 1<<MESSAGE_MEM_API_RST);
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_CTRL , 0x0000000000000000);
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
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_DATA , word);
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_CTRL , (1 << MESSAGE_MEM_API_CS) | (wrEn<<MESSAGE_MEM_API_WR));
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_CTRL , 0x0000000000000000);
    }
  } else {
    // readback for checking
    for (int i=0; i<wCnt;i++) {
      // read from current
      word = cep_readNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_DATA); // bit[63:32] are read data
      Word2Byte(word>>32,&(msg[j]));
      j += 4;
      // issue for next
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_CTRL , (1 << MESSAGE_MEM_API_CS));
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MESSAGE_CTRL , 0x0000000000000000);
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
  if (GetVerbose()) { LOGI("%s wCnt=%d\n",__FUNCTION__,wCnt); }  
  // 
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_CTRL , 1<<MODULUS_MEM_API_RST);
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_CTRL , 0x0000000000000000);
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
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_DATA , word);
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_CTRL , (1 << MODULUS_MEM_API_CS) | (wrEn<<MODULUS_MEM_API_WR));
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_CTRL , 0x0000000000000000);
    }
    cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_LENGTH  , wCnt);
  } else {
    for (int i=0; i<wCnt;i++) {    
      // read from current
      word = cep_readNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_DATA); // bit[63:32] are read data
      Word2Byte(word>>32,&(modulus[j]));
      j += 4;
      // issue for next
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_CTRL , (1 << MODULUS_MEM_API_CS));
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_CTRL , 0x0000000000000000);
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
  if (GetVerbose()) { LOGI("%s wCnt=%d\n",__FUNCTION__,wCnt); }    
  // 
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_CTRL , 1<<EXPONENT_MEM_API_RST);
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_CTRL , 0x0000000000000000);
  //
  // add 32-bits in front to extend the size!!
  //
  if (wrEn) {
    for (int i=0; i<wCnt;i++) {
      word = Byte2Word(&(exponent[j]));        
      j += 4;
      //
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_DATA , word);
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_CTRL , (1 << EXPONENT_MEM_API_CS) | (wrEn<<EXPONENT_MEM_API_WR));
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_CTRL , 0x0000000000000000);
    }
    int bitCnt = exponentBytes*8;
    if (GetUseExpBits()) {
      bitCnt = CountExpBits(exponent,exponentBytes);
    }
    cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_LENGTH  , bitCnt); // wCnt*32);    
  }
  else {
    for (int i=0; i<wCnt;i++) {    
      // read from current
      word = cep_readNcapture(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_DATA); // bit[63:32] are read data
      Word2Byte(word>>32,&(exponent[j]));
      j += 4;
      // next
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_CTRL , (1 << EXPONENT_MEM_API_CS));
      cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_CTRL , 0x0000000000000000);
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
  if (GetVerbose()) { LOGI("%s: wCnt=%d\n",__FUNCTION__,wCnt); }  
  // 
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_RESULT_CTRL , 1<<RESULT_MEM_API_RST);
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_RESULT_CTRL , 0x0000000000000000);
  //
  if (GetAddZeros()) wCnt++; 
  for (int i=0; i<wCnt;i++) {
    cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_RESULT_CTRL , (1 << RESULT_MEM_API_CS));
    cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_RESULT_CTRL , 0);
    word = cep_readNcapture(RSA_ADDR_BASE_K, RSA_ADDR_RESULT_DATA);
    if (!GetAddZeros() || (GetAddZeros() && (i > 0))) {
      Word2Byte(word,&(result[j]));
      //
      j += 4;
    }
  }
}

void cep_rsa::Start(void) {
  if (GetVerbose()) { LOGI("%s\n",__FUNCTION__); }  
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_CTRL            , 0x0000000000000002);
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_CTRL            , 0x0000000000000000);
}

int cep_rsa::waitTilDone(int maxTO) {
  if (GetVerbose()) { LOGI("%s: to=%d\n",__FUNCTION__,maxTO); }
#if 1
  return cep_readNspin(RSA_ADDR_BASE_K, RSA_ADDR_STATUS, 1, maxTO);
#else
  while (maxTO > 0) {
    if (cep_readNcapture(RSA_ADDR_BASE_K, RSA_ADDR_STATUS)== 0x0000000000000001) break;
    maxTO--;
    DUT_RUNCLK(2000);
  };
  return (maxTO <= 0) ? 1 : 0;
#endif
}

// Using Bignum to compute c = m^e mod n;
int cep_rsa::compute_results(void) {
  #if defined(BARE_MODE)
#else
  BIGNUM *expNum = BN_new();
  BIGNUM *modNum = BN_new();
  BIGNUM *msgNum = BN_new();
  BIGNUM *resNum = BN_new();
  BN_CTX *bnCtx  = BN_CTX_new();
  if (GetVerbose()) {
    LOGI("%s\n",__FUNCTION__);
  }
  if ((expNum == NULL) || (modNum == NULL) || (msgNum == NULL) || (resNum == NULL)) { LOGE("%s: can't make eNum\n",__FUNCTION__); return 1; }
  // convert to BN
  BN_CTX_init(bnCtx);
  (void) BN_bin2bn(EXPSTR, GetExpSize(), expNum);
  (void) BN_bin2bn(MODSTR, GetModSize(), modNum);
  (void) BN_bin2bn(MSGSTR, GetMsgSize(), msgNum);    
  // compute result
  mErrCnt += (BN_mod_exp(resNum,msgNum,expNum,modNum, bnCtx) == 0) ? 1 : 0;
  //convert back to char
  (void)BN_bn2bin(resNum, ERESSTR);
  BN_CTX_free(bnCtx);
  BN_free(msgNum);
  BN_free(expNum);
  BN_free(modNum);
  BN_free(resNum);
#endif
  return mErrCnt;
}

int cep_rsa::expIsOne(void) {
  int isOne = (EXPSTR[GetExpSize()-1] == 1) ? 1 : 0;
  for (int i=0;i<GetExpSize()-1;i++) {
    if (EXPSTR[i] != 0) {
      isOne = 0;
      break;
    }
  }
  return isOne;
}

int cep_rsa::generate_key(void) {
#if defined(BARE_MODE)
#else  
  // Get the exponent
  BIGNUM *eNum;
  BIGNUM *modNum;
  RSA *rsa;
  //
  if (GetVerbose()) {
    LOGI("%s\n",__FUNCTION__);
  }
  rsa    = RSA_new();
  eNum   = BN_new();
  modNum = BN_new();  
  if ((rsa == NULL) || (eNum == NULL) || (modNum == NULL)) {
    LOGE("%s: can't make eNum\n",__FUNCTION__);
    return 1;
  }
  (void) BN_bin2bn(EXPSTR, GetExpSize(), eNum);
  //  (void) BN_bin2bn(MODSTR, GetMsgSize(), modNum);
  mErrCnt += (RSA_generate_key_ex(rsa, (GetModSize()*8), eNum, NULL) == 0) ? 1 : 0;
  if (mErrCnt) {
    LOGI("%s: Problem with RSA_generate_key_ex. (%s) \n",__FUNCTION__,
	 ERR_error_string(ERR_get_error(),NULL));
    mErrCnt=0; // for now, ignore it
  }
  //
  if (mUseManualMod == 0) {
    (void) BN_bn2bin(rsa->n, MODSTR); // Update MOD from 
  } else {
    (void) BN_bin2bn(MODSTR,GetMsgSize(),rsa->n); // force to use manual mod
  }
  //
  // Ned to adjust MSGSTR to make sure it is smaller then Modulus
  //
  if ((MSGSTR[0] > 0) && (MSGSTR[0] >= MODSTR[0])) {
    MSGSTR[0] = MODSTR[0]-1;
  }
  //
  //
  mErrCnt += (RSA_public_encrypt(GetMsgSize(),MSGSTR, ERESSTR, rsa, RSA_NO_PADDING) == -1) ? 1 : 0; // no
  if (mErrCnt) {
    LOGE("%s: Problem with RSA_public_encrypt. (%s) \n",__FUNCTION__,
	 ERR_error_string(ERR_get_error(),NULL));
    mErrCnt=0; // for now, ignore it
  }
  //
  #ifdef SIM_ENV_ONLY
  //LOGI("eNum=0x%08x modNum=0x%08x\n",rsa->e->d[0],rsa->n->d[0]);
  FILE *fp;
  fp = fopen("BIO_OUT","a");  
  RSA_print_fp(fp,rsa,0);
  fclose(fp);
  #endif
  //
  (void)BN_bn2bin(rsa->n, MODSTR);  
  //
  if ((RSA_check_key(rsa) != 1) && !expIsOne()) {
    mErrCnt++;
    LOGE("%s ERROR Key is not valid\n",__FUNCTION__);
  }
  RSA_free(rsa);
  BN_free(eNum);
  BN_free(modNum);
  //
#endif
  return mErrCnt;
}


// clean memories
void cep_rsa::CleamMem(int maxEntries) {
  if (GetVerbose()) { LOGI("%s maxEntries=%d\n",__FUNCTION__,maxEntries); }    
  SetModSize(maxEntries*4);   // Exponent
  SetMsgSize(maxEntries*4);     // modulus & message
  LoadMessage(mHwPt, GetMsgSize(),1);   
  LoadModulus(mHwCp, GetModSize(),1);   
  LoadExponent(mKEY, GetExpSize(),1);   
}

int cep_rsa::RunRsaTest(int maxLoop, int maxBytes) {
  //  int outLen=mBlockSize;
  // kind of empty the file
#ifdef SIM_ENV_ONLY  
  FILE *fp;  
  fp = fopen("BIO_OUT","w");  
  fclose(fp);
#endif
  //
  SetAddZeros(1);
  SetUseExpBits(1);
  //
  if (GetVerbose()) { LOGI("%s: maxLop=%d\n",__FUNCTION__,maxLoop); }  
  //
  //
  int bCnt = 0; //  min is 4 and multiple of 4
  for (int i=0;i<maxLoop;i++) { //  
    //
    // Takes 10+ hours just to run 128-bit encrytion
    //
    bCnt += 4;
    if (bCnt > maxBytes) {
      bCnt = 4;
    }
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
    for (int z=0;z<GetExpSize();z++) { EXPSTR[z] = 0; }
    //RandomGen(EXPSTR, GetExpSize());
    EXPSTR[GetExpSize()-4] = 0x00;    
    EXPSTR[GetExpSize()-3] = 0x01;
    EXPSTR[GetExpSize()-2] = 0x00;
    EXPSTR[GetExpSize()-1] = 0x01;
    // fixed exp
   //
#if 0
    mUseManualMod = 0;          // let openssl pick a valid modulus    
    // Message
    Word2Byte(0x0F1182EA+(i*4),MSGSTR); // 
    // Exponent
    Word2Byte(0x10001 + (i*4),EXPSTR);
    // modulus
    Word2Byte(0xb60eee11, MODSTR); // this is the valid modulus picked from openssl
#endif
    //1
    mErrCnt += generate_key(); //
    if (mErrCnt) break;
    // print
    if (mErrCnt || GetVerbose()) {
      PrintMe("Exponent" , EXPSTR, GetExpSize());
      PrintMe("Modulus " , MODSTR, GetModSize());
      PrintMe("Message " , MSGSTR, GetMsgSize());
      PrintMe("ExpCp   "  ,ERESSTR, GetMsgSize());      
    }
#if 1
    LoadExponent(EXPSTR, GetExpSize(),1);    
    LoadModulus(MODSTR, GetModSize(),1);
    LoadMessage(MSGSTR, GetMsgSize(),1);
    Start();
    if (mErrCnt) break;    
    mErrCnt += waitTilDone(((GetMsgSize()/4)+1) * 20000); // about 16K+ cycle per 32-bits (64-bit actual)
    if (!mErrCnt) {
      ReadResult(ARESSTR,GetMsgSize());
    }
    //
    // Check results against openssl
    //
    mErrCnt += CheckCipherText();    
#endif
    //
    // Print
    //
    if (mErrCnt || GetVerbose()) {
      PrintMe("Exponent" , EXPSTR, GetExpSize());
      PrintMe("Modulus " , MODSTR, GetModSize());
      PrintMe("Message " , MSGSTR, GetMsgSize());
      PrintMe("ExpCp   "  ,ERESSTR, GetMsgSize());
      compute_results();
      PrintMe("BNCp    "  ,ERESSTR, GetMsgSize());
      // openSSL
      PrintMe("ActCp   "  ,ARESSTR, GetMsgSize());      
      //Word2Byte(compute_manually(), ERESSTR);
      //PrintMe("Manual  "  ,ERESSTR, GetMsgSize());      
    }
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
// To test memory
//
int cep_rsa::RunRsaMemTest(int memMask, int sizeInBytes) {
  //
  SetAddZeros(0); // 32bit become 32+32, etc..
  //
  // Just for coverage
  //
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_LENGTH  , -1);
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_EXPONENT_LENGTH  ,  0);
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_LENGTH   , -1);
  cep_writeNcapture(RSA_ADDR_BASE_K, RSA_ADDR_MODULUS_LENGTH   ,  0);  
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
    if (mErrCnt) {
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
    if (mErrCnt) {
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
    if (mErrCnt) {
      LOGI("%s: Testing Message Memory bytes=%d error=%d\n",__FUNCTION__,sizeInBytes,mErrCnt);
      PrintMe("expMem "  ,mSwPt, GetBlockSize());
      PrintMe("actMem "  ,mHwPt, GetBlockSize());    
    }
  }
  return mErrCnt;
}
//
// Big message/small exponent
//

int cep_rsa::RunRsaTest2(int maxLoop, int maxBytes) {
  //  int outLen=mBlockSize;
  // kind of empty the file
#ifdef SIM_ENV_ONLY  
  FILE *fp;  
  fp = fopen("BIO_OUT","w");  
  fclose(fp);
#endif
  //
  SetAddZeros(1);
  SetUseExpBits(1);
  //
  //
  //
  int bCnt = maxBytes; //
  //
  // 32-bits, 64, 96 ...
  //
  SetExpSize(4);
  Word2Byte(0x1,EXPSTR);
  SetModSize(bCnt);   
  SetMsgSize(bCnt);   // modulus & message
  // generate random message, exp and modulus
  // NOTE: exp should be odd
  RandomGen(MSGSTR, GetMsgSize());
  //1
  mErrCnt += generate_key(); //
  // print
  if (mErrCnt || GetVerbose()) {
    PrintMe("Exponent" , EXPSTR, GetExpSize());
    PrintMe("Modulus " , MODSTR, GetModSize());
    PrintMe("Message " , MSGSTR, GetMsgSize());
    PrintMe("ExpCp   "  ,ERESSTR, GetMsgSize());      
  }
#if 1
  LoadExponent(EXPSTR, GetExpSize(),1);    
  LoadModulus(MODSTR, GetModSize(),1);
  LoadMessage(MSGSTR, GetMsgSize(),1);
  Start();
  mErrCnt += waitTilDone(((GetMsgSize()/4)+1) * 20000); // about 16K+ cycle per 32-bits (64-bit actual)
  if (!mErrCnt) {
    ReadResult(ARESSTR,GetMsgSize());
  }
  //
  // Check results against openssl
  //
  mErrCnt += CheckCipherText();    
#endif
  //
  // Print
  //
  if (mErrCnt || GetVerbose()) {
    PrintMe("Exponent" , EXPSTR, GetExpSize());
    PrintMe("Modulus " , MODSTR, GetModSize());
    PrintMe("Message " , MSGSTR, GetMsgSize());
    PrintMe("ExpCp   "  ,ERESSTR, GetMsgSize());
    compute_results();
    PrintMe("BNCp    "  ,ERESSTR, GetMsgSize());
    // openSSL
    PrintMe("ActCp   "  ,ARESSTR, GetMsgSize());      
    //Word2Byte(compute_manually(), ERESSTR);
    //PrintMe("Manual  "  ,ERESSTR, GetMsgSize());      
  }
  return mErrCnt;
}
