//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      shMem.h
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef SHMEM_H
#define SHMEM_H
// ---------------------------

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shIpc.h"

// Key files
#define C2VKeyFile "C2VKeyFile.KEY"

// 1K class object
#define MAX_SEGSIZE (1 << 10)

// 16 slots with 16 CPU per slot
#define MAX_SLOTS         16
#define MAX_CPU_PER_SLOT  8
#define MAX_SHIPC         (MAX_SLOTS * MAX_CPU_PER_SLOT)
#define GET_SLOTID(offset) (offset/MAX_CPU_PER_SLOT)
#define GET_CPUID(offset)  (offset%MAX_CPU_PER_SLOT)
#define GET_OFFSET(slotId,cpuId) ((slotId*MAX_CPU_PER_SLOT)+cpuId)
//
// Share Memory Segment between 2 process
//
class shMem {
  // public stuffs
 public:
  // constructors
  shMem();
  ~shMem(); // destroy when done -> return the share segment
  // methods
  void  shMemInit( int asClient, key_t seedOrKey);      // this is for initiator
  void shMemDestroy() ;
  
  shIpc *getPtr();
  key_t getKey();
  long unsigned int getActiveMask() { return mActiveMask; }
  void setActiveMask(long unsigned int mask) { mActiveMask = mask; }
  
  void setFlag(int isClient);
  int getFlag();
  shIpc *getIpcPtr(int slotId, int cpuId);
  shIpc *getIpcPtr(int offset);
  int getOffset(int slotId, int cpuId) ;
  void getSlotCpuId(int offset,int *slotId, int *cpuId);

  int GetTheKey(const char *keyFile);
  void SetTheKey(int key, const char *keyFile);

  void SetReStartFlag(int value) { mReStartFlag = value; }
  int GetReStartFlag() { return mReStartFlag; }
private:
  // unique key
  key_t mKey;
  // active thread mask if specify
  long unsigned int mActiveMask;
  // shmid
  int shmid;
  // segment pointer
  shIpc *segptr;
  // flag
  int ClientFlag;
  int mReStartFlag;
};

extern shMem GlobalShMemory;
//
#endif
