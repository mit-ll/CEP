//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef CEP_APIS_H
#define CEP_APIS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
  /*
   * Function Prototypes
   */
  void dump_wave(int cycle2start, int cycle2capture, int enable);
  int calibrate_ddr3(int maxTimeOut);
  int is_program_loaded(int maxTimeOut);  
  int check_bare_status(int coreId,int maxTimeOut);  
  int load_mainMemory(char *imageF, uint32_t ddr3_base, int srcOffset, int destOffset, int backdoor_on, int verify);
  int clear_printf_mem(int coreId);
  int set_status(int errCnt, int testId);

//
// syscalls.c has printf
//
#ifdef BARE_MODE
#undef printf
#include "encoding.h"
  extern int printf(const char *format, ...);
  extern void set_printf(int enable);
  extern int  get_printf(void);
#endif
  
// end of extern
#ifdef __cplusplus
}
#endif


// ===============
#endif
