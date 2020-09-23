//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: MIT
//
// File Name:      
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
  simPio(); // 
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
  u_int64_t sim_Read32_64(u_int32_t adr);
  u_int64_t sim_Read64_64(u_int64_t adr);  
  void sim_Write32_64(u_int32_t adr, u_int64_t data);
  void sim_Write64_64(u_int64_t adr, u_int64_t data);  
  void sim_WriteDvtFlag(int msb, int lsb, int val) ;
  u_int64_t  sim_ReadDvtFlag(int msb, int lsb) ;  
  void sim_Framer_RdWr(u_int32_t adr, u_int32_t wrDat, u_int32_t *rdDat);
  void sim_Sample_RdWr(u_int32_t adr, u_int64_t wrDat, u_int64_t *rdDat);
  int  sim_SetInActiveStatus(void);
}

#endif
