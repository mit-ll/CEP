//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

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
typedef struct p2vMap {
  size_t phy_address;
  unsigned char *mem;
  int fd;
  size_t pagesize;
  size_t mask;
} p2vMap_st;

p2vMap_st _cepMap;
p2vMap_st _ddr3Map;

//
// offs can be full address as well since it will be masked off
//
u_int64_t lnx_cep_read(u_int32_t offs) {
  return *(u_int64_t*)(&(_cepMap.mem)[offs & _cepMap.mask]);
}
void lnx_cep_write(u_int32_t offs,u_int64_t pData) {
  memcpy(_cepMap.mem + (offs & _cepMap.mask) , &pData, sizeof(pData));
}

//
// DDR3
//
u_int64_t lnx_mem_read(u_int32_t offs) {
  return *(u_int64_t*)(&(_ddr3Map.mem)[offs & _ddr3Map.mask]);
} 
void lnx_mem_write(u_int32_t offs,u_int64_t pData) {
  memcpy(_ddr3Map.mem + (offs & _ddr3Map.mask) , &pData, sizeof(pData));
}

//
// =======================================
// CEP Diagnostic dispatcher
// =======================================
//
static void Print_CepBuildDate(void) {
  printf("\n*** CEP Tag=%s CEP HW VERSION = v%d.%d was built on %s %s ***\n",
   CEP_TAG, MAJOR_VERSION(lnx_cep_read(CEP_VERSION_REG)), MINOR_VERSION(lnx_cep_read(CEP_VERSION_REG)),__DATE__,__TIME__);
  // do the read and print
  printf(" CEP FPGA Physical=0x%08x -> Virtual=0x%016lx\n",(u_int32_t)_cepMap.phy_address,(u_int64_t)_cepMap.mem);
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
  if (_cepMap.fd < 0) {
    printf("ERROR: **** Can't open /dev/mem for CEP @0x%016lx\n", _cepMap.phy_address);
    return 1;
  } else {
    _cepMap.mem = (unsigned char *)mmap(0, _cepMap.pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, _cepMap.fd, _cepMap.phy_address);
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
    _ddr3Map.mem = (unsigned char *)mmap(0, _ddr3Map.pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, _ddr3Map.fd, _ddr3Map.phy_address);
    if (_ddr3Map.mem == MAP_FAILED) {
      printf("ERROR: **** Can't map memory %zu for DDR3\n", _ddr3Map.phy_address);
      return 1;
    }
  }
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
  //
  return(gErrCnt);
}
