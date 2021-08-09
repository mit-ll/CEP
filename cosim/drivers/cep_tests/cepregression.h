//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
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
  void init_srot(void);
  void init_cepregs(void);

  uint64_t get_physical_adr(int device, uint32_t pAddress);
  // 32-bits
  void cep_write32(int device, uint32_t pAddress, uint32_t pData);
  uint32_t cep_read32(int device, uint32_t pAddress);
  // 64-bits
  void cep_write(int device, uint32_t pAddress, uint64_t pData);
  uint64_t cep_read(int device, uint32_t pAddress);
  
#ifdef __cplusplus
}
#endif
  
#endif
