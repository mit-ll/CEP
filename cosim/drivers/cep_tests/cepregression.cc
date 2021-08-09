//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      cepregression.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    Regression test program for the CEP
// Notes:          Buildroot does not seem to like having special
//                 characters in the name.
//************************************************************************

//
// These are for real HW
//
#ifndef SIM_ENV_ONLY
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>

#ifndef BARE_MODE
#include <syslog.h>
#include <sys/mman.h>
#endif

//
// ==========================
// For simulation ONLY
// ===========================
#else
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "simdiag_global.h"
#include "portable_io.h"
#include "simPio.h"

// turn this on to sync with HW output else message might be mangle and not sync due to different processes printing
//
//#define printf LOGI
#endif

#include "cepregression.h"


// override printf
#ifdef LINUX_MODE
#include "simdiag_global.h"
#include "cep_io.h"
#define printf(format, ...) do { \
  if (VERBOSE1()) { THR_LOGI(format, ##__VA_ARGS__); }	\
  } while (0)
#endif


//
// syscalls.c has printf
//
#ifdef BARE_MODE
#undef printf
#include "encoding.h"
#define MAP_FAILED	((void *) -1)
//int munmap (void *__addr, size_t __len) { return 1; }
//long sysconf(int name);
extern int printf(const char *format, ...);
#endif
//
//
//
#include "dft_input.h"
#include "fir_input.h"
#include "idft_input.h"
#include "iir_input.h"
#include "CEP.h"


//************************************************************************
// BEGIN: Ip Core Structure 
//************************************************************************
typedef struct ipCoreData {
    char name[MAX_CONFIG_STRING_K];
    size_t address;
    unsigned char *mem;
    int fd;
    size_t pagesize;
    bool enabled;
} IpCore_st;
//************************************************************************
// END: Ip Core Structure 
//************************************************************************

//************************************************************************
// BEGIN: Global Variables
//************************************************************************
IpCore_st ipCores[CEP_TOTAL_CORES];
//************************************************************************
// END: Global Variables
//************************************************************************

//************************************************************************
// BEGIN: CEP MMAP Utility Functions
//************************************************************************
// cep_read - Read from a CEP address
uint32_t cep_read32(int device, uint32_t pAddress) {
#ifdef BARE_MODE
  return *(volatile uint32_t*)(ipCores[device].address + pAddress);
#elif SIM_ENV_ONLY
  uint32_t d32;
  DUT_READ32_32( ipCores[device].address + pAddress, d32);
  return d32;
#elif LINUX_MODE
  return lnx_cep_read32(ipCores[device].address + pAddress);
#else  
    return *(uint32_t*)(&(ipCores[device].mem)[pAddress]);
#endif
}
// 64-bits
uint64_t cep_read(int device, uint32_t pAddress) {
#ifdef BARE_MODE
  return *(volatile uint64_t*)(ipCores[device].address + pAddress);
#elif SIM_ENV_ONLY
  uint64_t d64;
  DUT_READ32_64( ipCores[device].address + pAddress, d64);
  return d64;
#elif LINUX_MODE
  return lnx_cep_read(ipCores[device].address + pAddress);
#else  
    return *(uint64_t*)(&(ipCores[device].mem)[pAddress]);
#endif
}

uint64_t get_physical_adr(int device, uint32_t pAddress) {
  return (ipCores[device].address + pAddress);
}

// cep_write - Write to a CEP address
//
void cep_write32(int device, uint32_t pAddress, uint32_t pData) {
#ifdef BARE_MODE
  *(volatile uint32_t*)(ipCores[device].address + pAddress) = pData;  
#elif SIM_ENV_ONLY
  DUT_WRITE32_32( ipCores[device].address + pAddress, pData);
#elif LINUX_MODE
  return lnx_cep_write32(ipCores[device].address + pAddress, pData);  
#else
  memcpy(ipCores[device].mem + pAddress, &pData, sizeof(pData));
#endif  
}

//64-bit
void cep_write(int device, uint32_t pAddress, uint64_t pData) {
#ifdef BARE_MODE
  *(volatile uint64_t*)(ipCores[device].address + pAddress) = pData;  
#elif SIM_ENV_ONLY
  DUT_WRITE32_64( ipCores[device].address + pAddress, pData);
#elif LINUX_MODE
  return lnx_cep_write(ipCores[device].address + pAddress, pData);  
#else
  memcpy(ipCores[device].mem + pAddress, &pData, sizeof(pData));
#endif  
}

// ===========
// AES Test
// ===========
void init_aes(void) {
#if 1 // def BARE_MODE
  ipCores[AES_BASE_K].address = cep_core_info[AES_BASE_K].base_address;
  ipCores[AES_BASE_K].enabled = true;  
#endif  
}

// ===========
// DES3 Test
// ===========
void init_des3(void) {
#if 1 // def BARE_MODE
  ipCores[DES3_BASE_K].address = cep_core_info[DES3_BASE_K].base_address;
  ipCores[DES3_BASE_K].enabled = true;
#endif
  
}

// ===========
// DFT Test
// ===========
void init_dft(void) {
#if 1 // def BARE_MODE
  ipCores[DFT_BASE_K].address = cep_core_info[DFT_BASE_K].base_address;
  ipCores[DFT_BASE_K].enabled = true;
#endif  
}

// ===========
// IDFT Test
// ===========
void init_idft(void) {
#if 1 // def BARE_MODE
  ipCores[IDFT_BASE_K].address = cep_core_info[IDFT_BASE_K].base_address;
  ipCores[IDFT_BASE_K].enabled = true;
#endif  
}

// ===========
// FIR Test
// ===========
void init_fir(void)
{
#if 1 // def BARE_MODE
  ipCores[FIR_BASE_K].address = cep_core_info[FIR_BASE_K].base_address;
  ipCores[FIR_BASE_K].enabled = true;
#endif  
}

// ===========
// IIR Test
// ===========
void init_iir(void) {
#if 1 // def BARE_MODE
  ipCores[IIR_BASE_K].address = cep_core_info[IIR_BASE_K].base_address;
  ipCores[IIR_BASE_K].enabled = true;
#endif  
}

// ===========
// GPS Test
// ===========
void init_gps(void)  {
#if 1 // def BARE_MODE
  ipCores[GPS_BASE_K].address = cep_core_info[GPS_BASE_K].base_address;
  ipCores[GPS_BASE_K].enabled = true;
#endif  
}

// ===========
// MD5 Test
// ===========
void init_md5(void) {
#if 1 // def BARE_MODE
  ipCores[MD5_BASE_K].address = cep_core_info[MD5_BASE_K].base_address;
  ipCores[MD5_BASE_K].enabled = true;
#endif
}

// ===========
// RSA Test
// ===========
void init_rsa(void) {
#if 1 // def BARE_MODE
  ipCores[RSA_BASE_K].address = cep_core_info[RSA_BASE_K].base_address;
  ipCores[RSA_BASE_K].enabled = true;
#endif  
}

// ===========
// SHA256 Test
// ===========
void init_sha256(void) {
#if 1 // def BARE_MODE
  ipCores[SHA256_BASE_K].address = cep_core_info[SHA256_BASE_K].base_address;
  ipCores[SHA256_BASE_K].enabled = true;
#endif
}

// ===========
// SRoT Test
// ===========
void init_srot(void) {
#if 1 // def BARE_MODE
  ipCores[SROT_BASE_K].address = cep_core_info[SROT_BASE_K].base_address;
  ipCores[SROT_BASE_K].enabled = true;  
#endif  
}

void init_cepregs(void) {
#if 1 // def BARE_MODE
  ipCores[CEP_VERSION_REG_K].address = cep_core_info[CEP_VERSION_REG_K].base_address;
  ipCores[CEP_VERSION_REG_K].enabled = true;  
#endif  
}


//************************************************************************
// BEGIN: Config File Functions
//************************************************************************

int __initConfigDone = 0;
// Initializes the ipCore_st structure for all cores
void initConfig()
{

#ifndef BARE_MODE
    size_t pagesize = sysconf(_SC_PAGE_SIZE); // Assuming every core uses 4k

#else
    size_t pagesize = 1 << 14;
#endif

    for (int i = 0; i < CEP_TOTAL_CORES; i++) {
        ipCores[i].address          = cep_core_info[i].base_address;
        strncpy(ipCores[i].name, cep_core_info[i].name, strlen(cep_core_info[i].name));
        ipCores[i].fd               = -1;
        ipCores[i].mem              = (unsigned char *)MAP_FAILED;
        ipCores[i].pagesize         = pagesize;
        ipCores[i].enabled          = cep_core_info[i].enabled;		
    }

    __initConfigDone = 1;    

}

//************************************************************************
// END: Config File Functions
//************************************************************************




