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

#ifndef __ACCESS_H
#define __ACCESS_H
// ---------------------------


#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef BARE_MODE
#include "stdTypes.h"
#endif

//
// this is where the overloads are happening depend on plaform setting
//
#include "portable_io.h"
#include "simdiag_global.h"


#if defined(_SIM_HW_ENV) || defined(_SIM_SW_ENV)
#include "v2c_cmds.h"
#include "v2c_sys.h"
#endif

#ifdef SIM_ENV_ONLY

#ifdef DLL_SIM
#include "log.h"
//#include "vpp_dll.h"
#else
#include "shMem.h"
#include "shLog.h"
#endif
#endif

//
// Access  Class
// 

// define for GetPinValue
#define CPU_INT_PIN     0

extern int gActiveSlotId;
extern int gActiveLocalId;

// use this as baseclass
//
class access {
  // public stuffs
public:
  //
  // constructors
  //
  access();
  access(int slotId, int localId, int verbose=1);
  ~access(); 

  //
  // Set/Get
  //
  void SetSemaId(int value) { mSlotId = value; }
  int GetSetProgStatus(int semaId, int newStatus);

  void SetSlotId(int value) { mSlotId = value; }
  void SetLocalId(int value) { mLocalId = value; }

  int GetVerbose() { return mLogLevelMask & mVerbose; }
  void SetVerbose(int value) { mVerbose = value; }

  int GetSlotId() { return mSlotId; }
  int GetLocalId() { return mLocalId; }

  //
  // Address is Byte Address
  //
  void Write32_64(u_int32_t address, u_int64_t dat);
  void Write64_64(u_int64_t address, u_int64_t dat);  


  unsigned GetSimClk();
  void SimCheckPoint(char *saveFile);
  u_int64_t Read32_64(u_int32_t address);
  u_int64_t Read64_64(u_int64_t address);    
  //
  // to support showTime
  //
  int Framer_RdWr(u_int32_t address, u_int32_t wrDat, u_int32_t *rdDat);
  int Sample_RdWr(u_int32_t address, u_int64_t wrDat, u_int64_t *rdDat);  
  //
  int ReadNCompare(unsigned address, unsigned expDat, unsigned mask=0xFFFFFFFF);

  void RunClk(int numClk);

  void SetUseShIpcFlag(int value) { mUseShIpc = value; }
  int GetUseShIpcFlag() { return mUseShIpc; }
  int EnableShIpc(int onOff);
  void MaybeAThread();

  int  SetInActiveStatus(void);
  
  void WriteDvtFlag(int msb, int lsb, int value) ;
  u_int64_t  ReadDvtFlag(int msb, int lsb) ;
  int  ReadStatus(); // Flow control
  int  ReadErrorCount(); // Flow control
#ifdef SIM_ENV_ONLY
  shIpc *ptr;
#endif

  static int mVerbose;
  
  // print??
  void Print(const char *fmt, ...);

protected:
  // members
  int mSlotId;
  int mLocalId;
  int mUseShIpc;
  int mLogLevelMask;
  int mDettach;
};

//
#endif
