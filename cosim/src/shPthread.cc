//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <elf.h>
#include <stdint.h>
#include "shPthread.h"

#ifdef SIM_ENV_ONLY
#include "v2c_cmds.h"
#include "v2c_sys.h"
#endif

//
// Global Stuffs
//
pthread_t globalThread2SlotIdMap[MAX_SHIPC];
pthread_t globalThread2Join[MAX_SHIPC]; // use for Joining
int globalThreadStatus[MAX_SHIPC];
int globalThread2SlotIdMap_initDone = 0;

//
// Initialize shMem &  thread map table if not yet done so
//
void shPthread::InitTables() {
  if (!globalThread2SlotIdMap_initDone) {
    // init the GlobalShMemory
    GlobalShMemory.SetReStartFlag(GetReStartFlag());
    GlobalShMemory.shMemInit(0,0x0);
    for (int i=0;i<MAX_SHIPC;i++) {
      globalThread2SlotIdMap[i] = (pthread_t)(0xFFFFFFFF);
      globalThread2Join[i] = (pthread_t)(0xFFFFFFFF);
      globalThreadStatus[i] = SHIPC_STATUS_INACTIVE;
    }
    globalThread2SlotIdMap_initDone = 1;
  }
}
//
//
void shPthread::SetActiveMask(long unsigned int mask) {
  GlobalShMemory.setActiveMask(mask);
}
//

//
// Create a thread
//
int shPthread::ForkAThread(pthread_parm_t *tParm, void * (*start_routine)(void *), int detachIt) {
  int errCnt = 0;
  char StartFile[32];
#ifdef SIM_ENV_ONLY
  //
  // Initial thread map table if not yet
  printf("shPthread::ForkAThread: InitTables from : slotId=%d cpuId=%d globalThread2SlotIdMap_initDone=%d\n",
	 tParm->slotId, tParm->cpuId,globalThread2SlotIdMap_initDone);
  InitTables();
  printf("shPthread::ForkAThread: InitTables from : slotId=%d cpuId=%d globalThread2SlotIdMap_initDone=%d\n",
	 tParm->slotId, tParm->cpuId,globalThread2SlotIdMap_initDone);
  //
  // Execute the thread
  //
  pthread_t tid;
  pthread_attr_t attr;
  pthread_attr_init(&attr); /* initialize attr with default attributes */
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); /* system-wide contention */
  if (detachIt) {
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
  } else {
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
  }
  // save for later access
  int offset = GET_OFFSET(tParm->slotId,tParm->cpuId); 
  //
  int pstat;
  pstat = pthread_create( &tid,&attr, start_routine, (void *)tParm);
  if (pstat != 0) {
    printf("%s: pthread_create failed. pstat=%d\n",__FUNCTION__,pstat);
    exit(1);
  } else {
    printf("%s: pthread_create success. pstat=%d\n",__FUNCTION__,pstat);
  }
  globalThread2SlotIdMap[offset] = tid;
  if (!detachIt) { globalThread2Join[offset] = tid; }
  globalThreadStatus[offset] = SHIPC_STATUS_ACTIVE;
  printf("shPthread::ForkAThread: NEW THREAD from : slotId=%d cpuId=%d tid=%x\n",tParm->slotId,tParm->cpuId,tid);
  // wait until it alive
  shIpc *ptr = GlobalShMemory.getIpcPtr(offset);
  while (!ptr->IsAliveStatus());
  ptr->SetActiveStatus();
#endif
  return errCnt;  
}


int shPthread::ForkAThread(int slotId, int cpuId, int verbose, Int32U seed, void * (*start_routine)(void *), int detachIt) {
  int errCnt = 0;
#ifdef SIM_ENV_ONLY
  // parameter
  pthread_parm_t *tParm = new pthread_parm_t;
  tParm->slotId = slotId;
  tParm->cpuId = cpuId;
  tParm->verbose = verbose;
  tParm->seed = seed;
  tParm->restart = GetReStartFlag();
  // call
  ForkAThread(tParm,start_routine,detachIt);
  // wait until it alive then delete
  delete tParm;
#endif
  return errCnt;
}
//
// return if all done 
//
int shPthread::AllThreadDone() {
  int allDone = 1; // assume alldone
#ifdef SIM_ENV_ONLY
  for (int i=0;i<MAX_SHIPC;i++) {
    if ((globalThreadStatus[i] != SHIPC_STATUS_INACTIVE) && (i != GetSystemThreadOffset())) { // check!!
      shIpc *ptr = GlobalShMemory.getIpcPtr(i);
      if (ptr->GetThreadDone()) {
	int slotId, cpuId;
	GlobalShMemory.getSlotCpuId(i,&slotId,&cpuId);
	printf("SLOTID=%d CPUID=%d (Thread %x) is done with errorCount=%d\n",slotId,cpuId, (Int32U)globalThread2SlotIdMap[i],ptr->GetError());
	// update stat
	globalThreadStatus[i] = SHIPC_STATUS_INACTIVE;
	mErrCnt += ptr->GetError();
      } else {
	allDone = 0;
      }
    }
  }
//
#endif
  return allDone;
}
// 
int shPthread::AddSysThread(int slotId, int cpuId) {
  int errCnt = 0;
#ifdef SIM_ENV_ONLY
  //
  // Initial thread map table if not yet
  //
  InitTables();
  //
  // Register System Thread
  //
  mSystemThreadOffset = GET_OFFSET(slotId,cpuId);
  pthread_t tid =  pthread_self();
  globalThread2SlotIdMap[mSystemThreadOffset] = tid;
  globalThreadStatus[mSystemThreadOffset] = SHIPC_STATUS_ACTIVE;
  printf("shPthread::AddSysThread: NEW THREAD from : slotId=%d cpuId=%d tid=%x\n",slotId,cpuId,tid);
#endif
  return errCnt;
}
void shPthread::Shutdown() {
  shIpc *sysptr = GlobalShMemory.getIpcPtr(GetSystemThreadOffset());
  printf("Enter %s\n",__FUNCTION__);
  sysptr->Shutdown();
  //printf("Dettaching %s\n",__FUNCTION__);  
  // detach memory??
  GlobalShMemory.~shMem();
  //printf("Exit %s\n",__FUNCTION__);  
}
//
// process Only by the SYSTEM deamon thread!!!!!
//
int shPthread::ProcessRemoteReqs() {  // if any
#ifdef SIM_ENV_ONLY
  for (int i=0;i<MAX_SHIPC;i++) {
    if (globalThreadStatus[i] != SHIPC_STATUS_INACTIVE) {
      shIpc *ptr = GlobalShMemory.getIpcPtr(i);
      if (ptr->GetRemoteFlag() && ptr->IsRemoteCmdValid()) { // there is a pending remote access request
	int remoteOffset = ptr->GetRemoteOffset();
	// check if the remote is available
	shIpc *remotePtr = GlobalShMemory.getIpcPtr(remoteOffset);
	if (!remotePtr->GetRemoteReq()) { // it is avail
	  // notify the remote target & clear local flag
	  remotePtr->SetRemoteOffset(i);
	  remotePtr->SetRemoteReq();
	  // clear local flag
	  ptr->ClrRemoteFlag();
	  printf("Initiating remote request from %x ->%x\n",i,remoteOffset);
	}
      }
    }
  }
#endif
  return 0;
}
//
// Jojn any non-detachable thread
//
void shPthread::JoinAllNonDetachables() {
  for (int i=0;i<MAX_SHIPC;i++) {
    if (globalThread2Join[i] != (pthread_t)(0xFFFFFFFF)) {
      printf("Registering thread %x to join\n",globalThread2Join[i]);
      (void)pthread_join(globalThread2Join[i], NULL);
    }
  }
}
