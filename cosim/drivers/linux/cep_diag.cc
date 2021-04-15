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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <malloc.h>

#include <sys/mman.h>
//
// =======================================
// Includes
// =======================================
//
#include "cep_adrMap.h"
#include "simdiag_global.h"
#include "cep_io.h"
#include "cep_token.h"
#include "cep_script.h"
#include "cep_vars.h"
#include "cep_cmds.h"
#include "cep_run.h"
#include "cep_acc.h"
#include "cep_dbg.h"
#include "cep_diag.h"
#include "cep_exports.h"

// Macro to properly format the version register
#define MAJOR_VERSION(n) (int)((n >> 48) & 0xFF)
#define MINOR_VERSION(n) (int)((n >> 56) & 0xFF)
//
// =======================================
// Physical to virtual mapping stuffs to access CEP
// =======================================
//

//
// Global
// Physical to virtual mappings
//
//FILE *devNull = NULL;
typedef struct p2vMap {
  size_t phy_address;
  unsigned char *mem;
  int fd;
  size_t pagesize;
  size_t mask;
} p2vMap_st;

p2vMap_st _cepMap;
p2vMap_st _ddr3Map;
p2vMap_st _otherMap; // spi/UART
p2vMap_st _sysMap; // system 0 - 10000000

//
// offs can be full address as well since it will be masked off
//

#define do_read(_t,offs) do {						\
    if ((offs & cep_adr_mask) == cep_fpga_base_adr) {			\
      return *(_t *)(&(_cepMap.mem)[offs & _cepMap.mask]);		\
    } else if ((offs & ddr3_adr_mask) == ddr3_base_adr) {		\
      return *(_t*)(&(_ddr3Map.mem)[offs & _ddr3Map.mask]);		\
    } else if ((offs & other_adr_mask) == other_base_addr) {		\
      return *(_t *)(&(_otherMap.mem)[offs & _otherMap.mask]);		\
    } else {								\
      return *(_t *)(&(_sysMap.mem)[offs & _sysMap.mask]);		\
    }									\
  } while (0)

#define do_write(offs,pData) do {					\
    if ((offs & cep_adr_mask) == cep_fpga_base_adr) {			\
      memcpy(_cepMap.mem + (offs & _cepMap.mask) , &pData, sizeof(pData)); \
    } else if ((offs & ddr3_adr_mask) == ddr3_base_adr) {		\
      memcpy(_ddr3Map.mem + (offs & _ddr3Map.mask) , &pData, sizeof(pData)); \
    } else if ((offs & other_adr_mask) == other_base_addr) {		\
      memcpy(_otherMap.mem + (offs & _otherMap.mask) , &pData, sizeof(pData)); \
    } else {								\
      memcpy(_sysMap.mem + (offs & _sysMap.mask) , &pData, sizeof(pData)); \
    }									\
    } while (0)

u_int8_t  lnx_cep_read8 (u_int32_t offs) { do_read(u_int8_t,  offs); }
u_int16_t lnx_cep_read16(u_int32_t offs) { do_read(u_int16_t, offs); }
u_int32_t lnx_cep_read32(u_int32_t offs) { do_read(u_int32_t, offs); }
u_int64_t lnx_cep_read  (u_int32_t offs) { do_read(u_int64_t, offs); }

void lnx_cep_write8( u_int32_t offs,u_int8_t  pData) { do_write(offs,pData); }
void lnx_cep_write16(u_int32_t offs,u_int16_t pData) { do_write(offs,pData); }
void lnx_cep_write32(u_int32_t offs,u_int32_t pData) { do_write(offs,pData); }
void lnx_cep_write  (u_int32_t offs,u_int64_t pData) { do_write(offs,pData); }

//
#define FORCE_MAP_FIXED_REG
#define FORCE_MAP_FIXED_MEM
#define FORCE_MAP_FIXED_OTHER
#define FORCE_MAP_FIXED_SYS

#define RISCV_OTHER_VIRT_BASE  0x600000000ULL
#define RISCV_REG_VIRT_BASE    0x700000000ULL
#define RISCV_MEM_VIRT_BASE    0x800000000ULL
#define RISCV_SYS_VIRT_BASE    0xC00000000ULL
//
//
u_int64_t getCepRegVirtAdr(void)  { return (u_int64_t)_cepMap.mem; }
u_int64_t getCepMemVirtAdr(void)  { return (u_int64_t)_ddr3Map.mem; }
u_int64_t getCepMiscVirtAdr(void) { return (u_int64_t)_otherMap.mem; }
u_int64_t getCepSysVirtAdr(void)  { return (u_int64_t)_sysMap.mem; }

void cep_usleep(int x) { usleep(x); }

//
// Create a static scratchpad to  play with align to 4Mbytes
//
// 4 Mbytes
#define scratchSize (1<<22)
//u_int64_t memPad[scratchSize/8] __attribute__((aligned(scratchSize)));
u_int64_t *__scratch_ptr = NULL ; //  = memPad;
//


u_int64_t *getScratchPtr(void) { return __scratch_ptr; }
void clearScratchPtr(void) {
  bzero(__scratch_ptr,scratchSize);
}
//
// DDR3
//
u_int64_t lnx_mem_read(u_int32_t offs) { do_read(u_int64_t, offs); }

void lnx_mem_write(u_int32_t offs,u_int64_t pData) { do_write(offs,pData); }

//
// =======================================
// CEP Diagnostic dispatcher
// =======================================
//
static void Print_CepBuildDate(void) {
  printf("\n*** CEP Tag=%s CEP HW VERSION = v%x.%x was built on %s %s ***\n",
   CEP_TAG, MAJOR_VERSION(lnx_cep_read(CEP_VERSION_REG)), MINOR_VERSION(lnx_cep_read(CEP_VERSION_REG)),__DATE__,__TIME__);
  // do the read and print
  printf(" CEP FPGA Physical: cepReg/ddr3/other/sys -> Virtual=0x%016lx, 0x%016lx, 0x%016lx, 0x%016lx ScratchPad=0x%016lx\n",
	 (u_int64_t)_cepMap.mem,
	 (u_int64_t)_ddr3Map.mem,
	 (u_int64_t)_otherMap.mem,
	 (u_int64_t)_sysMap.mem,
	 (u_int64_t)__scratch_ptr);
}
//
//
static int do_cep_map() {
  //
  // CEP FPGA
  //
  _cepMap.phy_address = cep_fpga_base_adr;
  _cepMap.pagesize    = cep_fpga_base_size;
  _cepMap.mask        = _cepMap.pagesize -1;
  _cepMap.fd = open("/dev/mem",O_RDWR|O_SYNC);
//
  if (_cepMap.fd < 0) {
    printf("ERROR: **** Can't open /dev/mem for CEP @0x%016lx\n", _cepMap.phy_address);
    return 1;
  } else {
#ifdef FORCE_MAP_FIXED_REG
    _cepMap.mem = (unsigned char *)mmap((void *)RISCV_REG_VIRT_BASE, _cepMap.pagesize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, _cepMap.fd, _cepMap.phy_address);
#else
    _cepMap.mem = (unsigned char *)mmap(0, _cepMap.pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, _cepMap.fd, _cepMap.phy_address);
#endif
    if (_cepMap.mem == MAP_FAILED) {
      printf("ERROR: **** Can't map memory %zu for CEP\n", _cepMap.phy_address);
      return 1;
    }
  }
  //
  // DDR3 FPGA
  //
  _ddr3Map.phy_address = ddr3_base_adr;
  _ddr3Map.pagesize    = ddr3_base_size;
  _ddr3Map.mask        = _ddr3Map.pagesize -1;
  _ddr3Map.fd = open("/dev/mem",O_RDWR|O_SYNC);
  if (_ddr3Map.fd < 0) {
    printf("ERROR: **** Can't open /dev/mem for DDR3 @0x%016lx\n", _ddr3Map.phy_address);
    return 1;
  } else {
#ifdef FORCE_MAP_FIXED_MEM
    _ddr3Map.mem = (unsigned char *)mmap((void *)RISCV_MEM_VIRT_BASE, _ddr3Map.pagesize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, _ddr3Map.fd, _ddr3Map.phy_address);
#else
    _ddr3Map.mem = (unsigned char *)mmap(0, _ddr3Map.pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, _ddr3Map.fd, _ddr3Map.phy_address);
#endif
    if (_ddr3Map.mem == MAP_FAILED) {
      printf("ERROR: **** Can't map memory %zu for DDR3\n", _ddr3Map.phy_address);
      return 1;
    }
  }
  //
  // SPI/UART & SRoT
  //
  _otherMap.phy_address = other_base_addr;
  _otherMap.pagesize    = other_base_size;
  _otherMap.mask        = _otherMap.pagesize -1;
  _otherMap.fd = open("/dev/mem",O_RDWR|O_SYNC);
  if (_otherMap.fd < 0) {
    printf("ERROR: **** Can't open /dev/mem for OTHER @0x%016lx\n", _otherMap.phy_address);
    return 1;
  } else {
#ifdef FORCE_MAP_FIXED_OTHER
    _otherMap.mem = (unsigned char *)mmap((void *)RISCV_OTHER_VIRT_BASE, _otherMap.pagesize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, _otherMap.fd, _otherMap.phy_address);
#else
    _otherMap.mem = (unsigned char *)mmap(0, _otherMap.pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, _otherMap.fd, _otherMap.phy_address);
#endif
    if (_otherMap.mem == MAP_FAILED) {
      printf("ERROR: **** Can't map memory %zu for OTHER\n", _otherMap.phy_address);
      return 1;
    }
  }

  //
  //system 0 - 0x10000000
  //
  _sysMap.phy_address = sys_base_addr;
  _sysMap.pagesize    = sys_base_size;
  _sysMap.mask        = _sysMap.pagesize -1;
  _sysMap.fd = open("/dev/mem",O_RDWR|O_SYNC);
  if (_sysMap.fd < 0) {
    printf("ERROR: **** Can't open /dev/mem for SYS @0x%016lx\n", _sysMap.phy_address);
    return 1;
  } else {
#ifdef FORCE_MAP_FIXED_SYS
    _sysMap.mem = (unsigned char *)mmap((void *)RISCV_SYS_VIRT_BASE, _sysMap.pagesize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, _sysMap.fd, _sysMap.phy_address);
#else
    _sysMap.mem = (unsigned char *)mmap(0, _sysMap.pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, _sysMap.fd, _sysMap.phy_address);
#endif
    if (_sysMap.mem == MAP_FAILED) {
      printf("ERROR: **** Can't map memory %zu for SYS\n", _sysMap.phy_address);
      return 1;
    }
  }

  // need devNull to slow down write to other...
#if 0
  devNull = fopen("/dev/null","w");
  if (devNull == NULL) {
    printf("ERROR: Can't open /dev/null\n");
    return 1;
  }
#endif
  //
  // alloc sscrtah pointer to play with
  //
  __scratch_ptr = (u_int64_t *)memalign(scratchSize, scratchSize);
  //__scratch_ptr = (u_int64_t *)aligned_alloc(scratchSize, scratchSize);
  if (__scratch_ptr == NULL) {
    LOGE("Can't malloc scratch_mem\n");
    return 1;
  }
  bzero(__scratch_ptr,scratchSize);
  //  if (VERBOSE1()) {    LOGI("scratch_prt=0x%016lx\n",(u_int64_t)__scratch_ptr);  }
  //
  //
  Print_CepBuildDate();
  return 0;
}
//

//
// =======================================
// Main Program
// =======================================
//
int 
main (int argc, char * const *argv)
{
  int gErrCnt = 0;
  //int flags = 0;
  int gSkipInit=0, gVerbose=0;
  //
  //int my_id=0, slot=0, targetMask=0;
  //
  gErrCnt += do_cep_map();
  if (!CEP_SOE && gErrCnt) return 1;
  //
  //
  //
  // ===================================
  // initialize variable and command lists
  // ===================================
  //
  // MUST init VAR then change it
  gErrCnt += cep_init_vars(); cep_sort_vars();
  //
  //
  //
  // Global setup
  //
  if (argc > 1) {
    sscanf(argv[1],"%d",&gSkipInit);
    SET_VAR_VALUE(skipInit,gSkipInit);
  }
  printf("gSkipInit=%d/%d\n",gSkipInit,GET_VAR_VALUE(skipInit));
  if (argc > 2) {
    sscanf(argv[2],"%d",&gVerbose);
    SET_VAR_VALUE(verbose,gVerbose);  
  }
  
  simdiag_SetVerbose(GET_VAR_VALUE(verbose));
  printf("gverbose=%d/%d\n",gVerbose,GET_VAR_VALUE(verbose));
  // 
  gErrCnt += cep_init_cmds(); cep_sort_cmds();
  //
  gErrCnt += cep_init_acc(); cep_sort_acc();
  gErrCnt += cep_init_run(); cep_sort_run();
  
  //
  //
  // ===================================
  // Overload some function pointes 
  // ===================================
  //

  //
  // ===================================
  // main loop
  // ===================================
  //
  //cep_set_done_init(1);
  simdiag_SetVerbose(GET_VAR_VALUE(verbose));
  set_stty();
  gErrCnt = 0;
  while (!GET_VAR_VALUE(quit)) {
    //
    // Get Command And execute here
    //
    cep_GetCommand("EnterCmd>", 0);
    gErrCnt += cep_cmd_process_and_execute();
    //
    // process error??
    //
    if (gErrCnt >= (int)GET_VAR_VALUE(maxErr)) { // stop script if error
      cep_script_cleanup_all(); 
      gErrCnt = 0;
    }
    //thread_yield();  
  } // until quit or "crash" :-))))
  SET_VAR_VALUE(quit, 0); // clear it to support re-entrance
  free(__scratch_ptr);
  //
  //  fclose(devNull);
  return(gErrCnt);
}
