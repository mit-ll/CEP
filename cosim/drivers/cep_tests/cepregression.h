//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef CEPREGRESSION_H
#define CEPREGRESSION_H

extern int __initConfigDone ;
#ifdef __cplusplus
extern "C" {
#endif

  //
  // for use with cep_diag
  //
  int cepregression_test(int mask);
  int run_ceptest(int mask);  
  void initConfig();
  void init_md5(void) ;
  void init_sha256(void) ;
  void init_iir(void) ;
  void init_fir(void) ;      
  void init_des3(void) ;
  void init_gps(void) ;
  void init_aes(void) ;
  void init_dft(void) ;
  void init_idft(void) ;  
  void init_rsa(void) ;
  
  int cep_AES_test(void);
  int cep_DES3_test(void);
  int cep_DFT_test(void);
  int cep_IDFT_test(void);
  int cep_FIR_test(void);
  int cep_IIR_test(void) ;
  int cep_GPS_test(void) ;
  int cep_MD5_test(void);
  int cep_RSA_test(void);
  int cep_SHA256_test(void);

  uint64_t get_physical_adr(int device, uint32_t pAddress);
  void cep_write(int device, uint32_t pAddress, uint64_t pData);
  uint64_t cep_read(int device, uint32_t pAddress);
  
#ifdef __cplusplus
}
#endif
  
#endif
