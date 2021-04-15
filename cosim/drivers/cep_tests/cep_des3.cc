//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cep_des3.cc/h
// Program:        Common Evaluation Platform (CEP)
// Description:    des3 test for CEP
// Notes:          
//************************************************************************
#if defined(BARE_MODE)
#else
//#include <openssl/rand.h>
//#include <openssl/ecdsa.h>
//#include <openssl/obj_mac.h>
//#include <openssl/err.h>
//#include <openssl/pem.h>
#include <openssl/evp.h>
//#include <openssl/hmac.h>

#include "simPio.h"

#endif

#include <string.h>
#include "cep_des3.h"
#include "simdiag_global.h"
#include "portable_io.h"
#include "CEP.h"
#include "cepregression.h"
#include "random48.h"
//
//

cep_des3::cep_des3(int seed, int verbose) {
  init();  
  SetKeySize(24); // 56*3 = 21 bytes
  SetBlockSize(64/8); // 8 bytes
  SetSeed(seed);
  SetVerbose(verbose);
}
//
cep_des3::~cep_des3()  {
}

//
// via openssl
//
int cep_des3::prepare_des3_key_N_text
(
 uint8_t *input,           // input text
 uint8_t *output,           // output packet
 int padding_enable,
 int length,
 int *outlen,
 int verbose)
{
  //
#if defined(BARE_MODE)
#else
  int tmplen;
  EVP_CIPHER_CTX ctx;
  EVP_CIPHER_CTX_init(&ctx);
  //
  if (GetDecrypt()) {
    EVP_DecryptInit_ex(&ctx, EVP_des_ede3_ecb(), NULL, &(mKEY[0]), NULL);
    if(!EVP_DecryptUpdate(&ctx, output, outlen, input, length)) {
      return 1;
    }      
  } else {
    EVP_EncryptInit_ex(&ctx, EVP_des_ede3_ecb(), NULL, &(mKEY[0]), NULL);
    EVP_CIPHER_CTX_set_padding(&ctx, padding_enable); // OFF
    //    EVP_CIPHER_CTX_set_key_length(&ctx, GetKeySize());     
    //
    if(!EVP_EncryptUpdate(&ctx, output, outlen, input, length)) {
      return 1;
    }
    
    /* Buffer passed to EVP_EncryptFinal() must be after data just
     * encrypted to avoid overwriting it.
     */
    if(!EVP_EncryptFinal_ex(&ctx, output + *outlen, &tmplen)) {
      /* Error */
      return 1;
    }
    *outlen += tmplen;
    //
  }
  //
  if (verbose) {
    printf("%s: INFO inLen=%d outlen=%d kL=%d bL=%d\n",__FUNCTION__,
	   length,*outlen,    
	   EVP_CIPHER_CTX_key_length(&ctx),
	   EVP_CIPHER_CTX_block_size(&ctx)
	 );


  }
  EVP_CIPHER_CTX_cleanup(&ctx);
  
#endif
  //
  return mErrCnt;
}

// Key[2,1,0] (64-bits) = mKEY[0....23]
void cep_des3::LoadKey(void) {
  //uint64_t word;
  
  for(int i = 0; i < 3; i++) { //  7-bytes/word for des3
    //    cep_writeNcapture(DES3_BASE_K, DES3_KEY_BASE + (((DES3_KEY_WORDS - 1) - i) * BYTES_PER_WORD),word);
    cep_writeNcapture(DES3_BASE_K, DES3_KEY_BASE + (i * BYTES_PER_WORD),mHwKEY[i]);
  }
}

// State[1:0] (64-bits/unit) = mHwPt[0....23]
void cep_des3::LoadInText(void) {
  uint64_t word;
  for(int i = 0; i < GetBlockSize()/8; i++) { //  8-bytes/word
    word = 0;
    for (int j=0;j<8;j++) {
      word = (word << 8) | (uint64_t)mHwPt[i*8 + j];      
    }
#ifdef BIG_ENDIAN
    cep_writeNcapture(DES3_BASE_K, DES3_IN_BASE + (i * BYTES_PER_WORD), word);    
#else
    cep_writeNcapture(DES3_BASE_K, DES3_IN_BASE + (((DES3_BLOCK_WORDS - 1) - i) * BYTES_PER_WORD), word);
#endif
  }
}

void cep_des3::SetMode(void) {
  cep_writeNcapture(DES3_BASE_K, DES3_DECRYPT, GetDecrypt());
}

void cep_des3::Start(void) {
  cep_writeNcapture(DES3_BASE_K, DES3_START, 0x1);
  cep_writeNcapture(DES3_BASE_K, DES3_START, 0x0);
}

int cep_des3::waitTilDone(int maxTO) {
#if 1
    if (GetVerbose(2)) {  LOGI("%s\n",__FUNCTION__); }    
  return cep_readNspin(DES3_BASE_K, DES3_DONE, 1, maxTO);
#else
  while (maxTO > 0) {
    if (cep_readNcapture(DES3_BASE_K, DES3_DONE)) break;
    maxTO--;
  };
  return (maxTO <= 0) ? 1 : 0;
#endif
}

void cep_des3::ReadOutText(void) {
  uint64_t word;
  for(int i = 0; i < GetBlockSize()/8; i++) { //  8-bytes/word
#ifdef BIG_ENDIAN
    word = cep_readNcapture(DES3_BASE_K, DES3_CT_BASE +(i * BYTES_PER_WORD));    
#else
    word = cep_readNcapture(DES3_BASE_K, DES3_CT_BASE +(((DES3_BLOCK_WORDS - 1) - i) * BYTES_PER_WORD));
#endif
    for (int j=0;j<8;j++) {
      mHwCp[i*8 +j]= (word >> (8*(7-j)) ) & 0xff;
    }
  }
}
uint8_t cep_des3::MakeOddByte(uint8_t bin)
{
  int oCnt=0;
  uint8_t bout;
  for (int i=0;i<8;i++) {
    if (bin & (1 << i)) oCnt++;
  }
  if (oCnt & 0x1) { bout = bin; }
  else { bout = bin & 0xFE; }
  //
  return bout;
}

void cep_des3::AdjustParity() {
  // Adjust mKEY = SW Keys with odd-pariy on LSB
  // mHwKEY = 56-bits, right-justify with no parity
  uint64_t hwW;
  for (int i=0;i<3;i++) {
    hwW = 0;
    // start from LSByte=7 to 0 in each
    for (int j=0;j<8;j++) {
      mKEY[(i*8)+j] = MakeOddByte(mKEY[(i*8)+j]); // swKey
      hwW = (hwW << 7) | ((mKEY[(i*8)+j]>>1)&0x7F);
    }
    // put
    mHwKEY[i] = hwW;
    //LOGI("hwKey%d=0x%016llx\n",i,mHwKEY[i]);
  }
}

int cep_des3::RunDes3Test(int maxLoop) {
  //
  // DES3 key is 56 bits translate to 64 bits with LSB is the odd-parity per bytes..
  // SW expects the key has 64-bits with odd parityin LSB
  // HW expetts only 56-bits
  /*
  uint8_t keyx[] = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		    0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		    0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
  uint8_t pt0[]  = {0x95, 0xF8, 0xA5, 0xE5, 0xDD, 0x31, 0xD9, 0x00}; // 
  */
  
  int outLen=mBlockSize;
  //
  for (int i=0;i<maxLoop;i++) {
    SetDecrypt(i&0x1);
    for (int l=0;l<2;l++) {
      if (GetVerbose()) {
	LOGI("%s: Loop %d Decrypt=%d\n",__FUNCTION__,i,GetDecrypt());
      }
      if (l == 0) {
	//
	// cover every byte of key and plain text
	//
	if (i < 128) {
	  for (int j=0;j<GetKeySize();j++) {
	    mKEY[j] = (i&0x1) ? ((~(i+j) & 0x7F) << 1) : (((i+j) & 0x7F) << 1);
	  }
	  for (int j=0;j<GetBlockSize();j++) {
	    mHwPt[j] = (i&0x1) ? ((i+j+GetKeySize()+(GetDecrypt()*128)) & 0xFF) : (~(i+j+GetKeySize()+(GetDecrypt()*128)) & 0xFF);
	  }
	} else {
	  // just randomize
	  // random
	  RandomGen(mKEY, GetKeySize());
	  RandomGen(mHwPt, GetBlockSize());      
	}
	// adjust parity and convert to SW/HW keys
	AdjustParity();
	//
	// create expected cipherTet
	mErrCnt += prepare_des3_key_N_text
	  (mHwPt, // uint8_t *input,           // input text packet
	   mSwCp, // uint8_t *output,           // output cipher packet
	   0,   // int padding_enable,
	   mBlockSize, // int length,
	   &outLen, GetVerbose());	
      }
#if 0
      memcpy(mKEY,keyx,24);
      memcpy(mHwPt,pt0,8);
      
      PrintMe("XXK",&(mKEY[0]),mKeySize);          
      PrintMe("XXT",&(mHwPt[0]),mBlockSize);
#endif
      
      //
      // load key&plaintext to HW
      //
      SetMode();
      LoadInText();
      if (l==0) { 
	LoadKey();
      }
      Start();
      mErrCnt += waitTilDone(50);
      if (!mErrCnt) {
	ReadOutText();
	mErrCnt += CheckCipherText();
      }
      if (mErrCnt && !GetExpErr()) {
	LOGE("%s: Loop %d l=%d with %d error\n",__FUNCTION__,i,l,mErrCnt);      
      }
      //
      // Print
      //
      if ((mErrCnt && !GetExpErr()) || GetVerbose(2)) {
	PrintMe("Key",       &(mKEY[0]),mKeySize);          
	PrintMe("InText    ",&(mHwPt[0]),mBlockSize);
	PrintMe("ExpOutText",&(mSwCp[0]),mBlockSize);
	PrintMe("ActOutText",&(mHwCp[0]),mBlockSize);        
      }
      if (mErrCnt) break;
      // flip, use memcpy instead of strncpy since migth has 0 in it
      memcpy(mSwCp,mHwPt,GetBlockSize()); // swap
      memcpy(mHwPt,mHwCp,GetBlockSize());      
      SetDecrypt(GetDecrypt() ^ 0x1);
    }
    MarkSingle(i);    
    if (mErrCnt) break;    
  }
  return mErrCnt;
}

