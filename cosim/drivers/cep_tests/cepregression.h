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
  void initConfig();

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
  
  
#ifdef __cplusplus
}
#endif
  
#endif
