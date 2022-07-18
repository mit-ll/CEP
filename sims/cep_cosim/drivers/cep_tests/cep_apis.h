//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      cep_apis.h
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//--------------------------------------------------------------------------------------

#ifndef CEP_APIS_H
#define CEP_APIS_H

#include <stdint.h>
#include "cep_adrMap.h"

#ifndef PLAYBACK_CMD_H
#define PLAYBACK_CMD_H
#define WRITE__CMD  1
#define RDnCMP_CMD  2
#define RDSPIN_CMD  3

#define WRITE__CMD_SIZE  3
#define RDnCMP_CMD_SIZE  3
#define RDSPIN_CMD_SIZE  5

#endif

#ifdef __cplusplus
extern "C" {
#endif

  /*
   * Function Prototypes
   */
  void set_backdoor_select(int backdoor);
  void enable_bootrom_uart (void);
  void enable_bootrom_sdboot (void);
  void set_uart_loopback(int loopback);
  void set_spi_loopback(int loopback);
  void release_tile_reset(int cpuId);
  void release_core_reset(int cpuId);
  void dump_wave(int cycle2start, int cycle2capture, int enable);
  int is_program_loaded(int maxTimeOut);  
  int check_bare_status(int cpuId, int maxTimeOut);  
  int loadMemory(char *imageF, int fileOffset, int maxByteCnt);
  int set_status(int errCnt, int testId);
  void set_pass(void);
  void set_fail(void);
  void set_cur_status(int status);
  int check_PassFail_status(int cpuMask, int maxTimeOut);
  //
  void cep_raw_write(uint64_t pAddress, uint64_t pData);
  uint64_t cep_raw_read(uint64_t pAddress);
  int cep_playback(uint64_t *cmdSeq,
       uint64_t upperAdr, uint64_t lowerAdr,
       int totalCmds,
       int verbose);

  //
  // lock
  //
  int cep_get_lock(int myId, int lockNum,int timeOut);
  void cep_release_lock(int myId, int lockNum);  
  int cep_get_lock_status(int myId, int lockNum, int *lockMaster);
  //
  void Set_C2C_Capture(int enable);
  int Get_C2C_Capture(void);
  void writeDvt(int msb, int lsb, int bits);
  void toggleDmiReset(void);


//
// syscalls.c has printf
//
#ifdef BARE_MODE
#undef printf
#include "encoding.h"
  extern int printf(const char *format, ...);
#endif
  
// end of extern
#ifdef __cplusplus
}
#endif


// ===============
#endif
