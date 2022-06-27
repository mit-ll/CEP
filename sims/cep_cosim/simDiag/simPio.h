//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      simPio.h
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef simPio_H
#define simPio_H

#include "access.h"
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>

class simPio : public access {
  public:
    simPio(); 
    ~simPio();

  protected:
  
  private:
    int mLogLevelMask;
};


//
// global sim Driver
//
extern simPio gCpu;
extern "C" {
  void sim_RunClk(int a);  
  // BFM Only
  int  sim_Atomic_Rdw64(u_int64_t adr, int param, int mask, u_int64_t *data);
  int  sim_Write64_BURST(u_int64_t adr, int wordCnt, u_int64_t *data);
  int  sim_Read64_BURST(u_int64_t adr, int wordCnt, u_int64_t *data);
  //
  u_int64_t sim_Read32_64(u_int32_t adr);
  u_int32_t sim_Read32_32(u_int32_t adr);
  u_int16_t sim_Read32_16(u_int32_t adr);
  u_int8_t sim_Read32_8(u_int32_t adr);
  u_int64_t sim_Read64_64(u_int64_t adr);  
  //
  void sim_Write32_64(u_int32_t adr, u_int64_t data);
  void sim_Write32_32(u_int32_t adr, u_int32_t data);
  void sim_Write32_16(u_int32_t adr, u_int16_t data);
  void sim_Write32_8(u_int32_t adr, u_int8_t data);
  void sim_Write64_64(u_int64_t adr, u_int64_t data);  
  void sim_WriteDvtFlag(int msb, int lsb, int val) ;
  u_int64_t  sim_ReadDvtFlag(int msb, int lsb) ;  
  int  sim_SetInActiveStatus(void);
}

#endif
