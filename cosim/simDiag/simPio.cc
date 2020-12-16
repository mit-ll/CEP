//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include "v2c_cmds.h"
#include "simPio.h"
//#include "sim_adr.h"

#include "simdiag_global.h"

simPio::simPio() : access() {
  mLogLevelMask = ACCESS_LOG_LEVEL;
}

simPio::~simPio() {
}

u_long curTarget = 0;

// Global
simPio gCpu;

void sim_RunClk(int a) { access axc; axc.RunClk(a); }
u_int32_t sim_Read32_32(u_int32_t adr) { access axc; return axc.Read32_32(adr); }
u_int64_t sim_Read32_64(u_int32_t adr) { access axc; return axc.Read32_64(adr); }
u_int64_t sim_Read64_64(u_int64_t adr) { access axc; return axc.Read64_64(adr); }
void sim_Write32_32(u_int32_t adr, u_int32_t data) { access axc; axc.Write32_32(adr,data); }
void sim_Write32_64(u_int32_t adr, u_int64_t data) { access axc; axc.Write32_64(adr,data); }
void sim_Write64_64(u_int64_t adr, u_int64_t data) { access axc; axc.Write64_64(adr,data); }
void sim_Framer_RdWr(u_int32_t adr, u_int32_t wrDat, u_int32_t *rdDat) { access axc; axc.Framer_RdWr(adr,wrDat,rdDat); }
void sim_Sample_RdWr(u_int32_t adr, u_int64_t wrDat, u_int64_t *rdDat) { access axc; axc.Sample_RdWr(adr,wrDat,rdDat); }
void sim_WriteDvtFlag(int msb, int lsb, int val) { access axc; axc.WriteDvtFlag(msb,lsb,val); }  
u_int64_t  sim_ReadDvtFlag(int msb, int lsb) { access axc; return axc.ReadDvtFlag(msb,lsb); } 
int  sim_SetInActiveStatus(void) { access axc; return axc.SetInActiveStatus(); }
