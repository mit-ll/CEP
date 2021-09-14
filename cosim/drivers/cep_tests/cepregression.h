//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
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
  void initConfig();  // Initializes the ipCores datastructures

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
