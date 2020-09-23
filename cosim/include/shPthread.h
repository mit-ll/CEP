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

#ifndef __shPthread_H
#define __shPthread_H

#include <pthread.h>
#include "v2c_cmds.h"
#include "shMem.h"

extern pthread_t globalThread2SlotIdMap[];
extern pthread_t globalThread2JoinMap[];
extern int globalThreadStatus[];
extern int globalThread2SlotIdMap_initDone;

//
// Struct for passing parameter
//
typedef struct {
  int slotId;
  int cpuId;
  int verbose;
  Int32U seed;
  int restart;
} pthread_parm_t;

class shPthread {
  // public stuffs
public:
  // constructors
  shPthread() { mErrCnt=0; mSystemThreadOffset = -1; mReStartFlag = -1; };
  ~shPthread() {};
  //
  void SetActiveMask(long unsigned int mask) ;
  //int ForkAThread(int slotId, int cpuId, char *soFile, char *apiFunc2Run);
  //int ForkAThread(int slotId, int cpuId, void * (*start_routine)(void *), int detachIt=1);
  int ForkAThread(int slotId, int cpuId, int verbose, Int32U seed, void * (*start_routine)(void *), int detachIt=1);
  int ForkAThread(pthread_parm_t *tParm, void * (*start_routine)(void *), int detachIt=1);
  
  int AddSysThread(int slotId, int cpuId) ;
  void JoinAllNonDetachables();
  int AllThreadDone();
  int ProcessRemoteReqs();
  void InitTables();
  int GetErrorCount() { return mErrCnt; }
  int GetSystemThreadOffset() { return mSystemThreadOffset; }
  void Shutdown();

  void SetReStartFlag(int value) { mReStartFlag = value; }
  int GetReStartFlag() { return mReStartFlag; }

private:
  // unique key
  int mErrCnt;
  int mSystemThreadOffset;
  int mReStartFlag;
};

#endif
