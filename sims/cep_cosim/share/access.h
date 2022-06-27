//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      access.h
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

#ifdef SIM_ENV_ONLY
  #include "shIpc.h"
  #include "shLog.h"
  #include "shMem.h"
  #include "log.h"
#endif
//
// this is where the overloads are happening depend on plaform setting
//
//#include "portable_io.h"
//#include "simdiag_global.h"


//#ifdef SIM_ENV_ONLY
//  #include "v2c_cmds.h"
//  #include "v2c_sys.h"
//
//  #ifdef DLL_SIM
//    #include "log.h"
//  #else
//    #include "shMem.h"
//    #include "shLog.h"
//  #endif
//#endif

//
// Access  Class
// 

extern int gActiveSlotId;
extern int gActiveLocalId;

class access {
  // public stuffs
  public:
    access();
    access(int slotId, int localId, int verbose=1);
    ~access(); 

    // Set/Get
    void      SetSemaId(int value) { mSlotId = value; }
    int       GetSetProgStatus(int semaId, int newStatus);

    void      SetSlotId(int value) { mSlotId = value; }
    void      SetLocalId(int value) { mLocalId = value; }

    int       GetVerbose() { return mLogLevelMask & mVerbose; }
    void      SetVerbose(int value) { mVerbose = value; }

    int       GetSlotId() { return mSlotId; }
    int       GetLocalId() { return mLocalId; }

    int       Atomic_Rdw64(u_int64_t address, int param, int mask, u_int64_t *data);
    int       Write64_BURST(u_int64_t address, int wordCnt, u_int64_t *dat);
    int       Read64_BURST (u_int64_t address, int wordCnt, u_int64_t *dat);

    void      Write32_64(u_int32_t address, u_int64_t dat);
    void      Write64_64(u_int64_t address, u_int64_t dat);  
    void      Write32_32(u_int32_t address, u_int32_t dat);
    void      Write32_32ite32_16(u_int32_t address, u_int16_t dat);
    void      Write32_8(u_int32_t address, u_int8_t dat);
    void      Write32_16(u_int32_t, u_int16_t);

    unsigned  GetSimClk();
    void      SimCheckPoint(char *saveFile);
    u_int64_t Read32_64(u_int32_t address);
    u_int32_t Read32_32(u_int32_t address);
    u_int16_t Read32_16(u_int32_t address);
    u_int8_t  Read32_8(u_int32_t address);
    u_int64_t Read64_64(u_int64_t address);    

    int       ReadNCompare(unsigned address, unsigned expDat, unsigned mask=0xFFFFFFFF);
    void      RunClk(int numClk);

    void      SetUseShIpcFlag(int value) { mUseShIpc = value; }
    int       GetUseShIpcFlag() { return mUseShIpc; }
    int       EnableShIpc(int onOff);
    void      MaybeAThread();
    int       SetInActiveStatus(void);
  
    void      WriteDvtFlag(int msb, int lsb, int value) ;
    u_int64_t ReadDvtFlag(int msb, int lsb) ;
    int       ReadStatus(); // Flow control
    int       ReadErrorCount(); // Flow control

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
