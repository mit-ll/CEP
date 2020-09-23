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

#include <stdio.h>
//#include <iostream.h>
#include <iostream>
#include <stdarg.h>
#include <string>	       

#include "shLog.h"
#include "shMem.h"


void shLog::shMem_printf(char *str) {
  // from th thread_id
  int slotId, cpuId;
  if (isAThread(&slotId,&cpuId)) { // found
    shIpc *ptr = GlobalShMemory.getIpcPtr(slotId,cpuId);
    ptr->Print(str);
  } else {
    // ERROR
    printf("ERROR: Can't find slot/cpu ID for this thread tdi=%x <%s>\n",(Int32U)pthread_self(),str);
  }
}

// return 1 if found else 0
int shLog::GetSlotId(pthread_t tid, int *slotId, int *cpuId) {
  int found = 0;
  int i = 0;
  if (globalThread2SlotIdMap_initDone) {
    while (!found && (i <= MAX_SHIPC)) {
      if (pthread_equal(globalThread2SlotIdMap[i],tid)) { //(globalThread2SlotIdMap[i] == tid) { 
	*slotId = GET_SLOTID(i); // (i >> 4) & 0xf;
	*cpuId  = GET_CPUID(i); // i & 0xf;
	found = 1;
      } else {
	i++;
      }
    }
  }
  return found;
}
pthread_t shLog::GetThreadId() {
  return (pthread_self());
}

int shLog::isAThread(int *slotId, int *cpuId) {
  if (globalThread2SlotIdMap_initDone) {
    pthread_t tid = GetThreadId();
    int ret = GetSlotId(tid,slotId,cpuId);    
    //printf("%s: ret=%d slot=%d cpu=%d\n",__FUNCTION__,ret,*slotId, *cpuId);
    return ret;
  } else {
    //printf("%s: NOT a thread\n",__FUNCTION__,*slotId, *cpuId);    
    return 0;
  }
}

