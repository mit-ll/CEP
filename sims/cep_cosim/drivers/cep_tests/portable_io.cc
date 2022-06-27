//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      portable_io.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    
//************************************************************************

#include "CEP.h"
#include "portable_io.h"

uint64_t get_physical_addr(int coreIndex, uint32_t pAddress) {
  return (cep_core_info[coreIndex].base_address + pAddress);
}

uint32_t cep_read32(int coreIndex, uint32_t pAddress) {
#ifdef BARE_MODE
  return *(volatile uint32_t*)(cep_core_info[coreIndex].base_address + pAddress);
#elif !defined(_SIM_HW_ENV)
  uint32_t d32;
  DUT_READ32_32( cep_core_info[coreIndex].base_address + pAddress, d32);
  return d32;
#endif
}

uint64_t cep_read64(int coreIndex, uint32_t pAddress) {
#ifdef BARE_MODE
  return *(volatile uint64_t*)(cep_core_info[coreIndex].base_address + pAddress);
#elif !defined(_SIM_HW_ENV)
  uint64_t d64;
  DUT_READ32_64( cep_core_info[coreIndex].base_address + pAddress, d64);
  return d64;
#endif
}

void cep_write32(int coreIndex, uint32_t pAddress, uint32_t pData) {
#ifdef BARE_MODE
  *(volatile uint32_t*)(cep_core_info[coreIndex].base_address + pAddress) = pData;  
#elif !defined(_SIM_HW_ENV)
  DUT_WRITE32_32( cep_core_info[coreIndex].base_address + pAddress, pData);
#endif  
}

void cep_write64(int coreIndex, uint32_t pAddress, uint64_t pData) {
#ifdef BARE_MODE
  *(volatile uint64_t*)(cep_core_info[coreIndex].base_address + pAddress) = pData;  
#elif !defined(_SIM_HW_ENV)
  DUT_WRITE32_64( cep_core_info[coreIndex].base_address + pAddress, pData);
#endif  
}





