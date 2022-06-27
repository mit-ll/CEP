//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      shMem.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

//
// extern "C" must be used if you want verilog to link it OK .. No name mangle, please
//
#ifdef SIM_ENV_ONLY
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "stdio.h"

#include "portable_io.h"
#include <string.h>

#include "v2c_cmds.h"
#include "share.h"
#endif
#include <stdlib.h>

#include "shMem.h"

// default contructor
shMem::shMem() { SetReStartFlag(0); mActiveMask=-1;}

// client/initiator constructor
void shMem::shMemInit( int asClient, key_t seedOrKey_key ) {
#ifdef SIM_ENV_ONLY
  
  // Verilog side of shared memory
  if (asClient) {
    
    mKey = seedOrKey_key; 
    
    // if mKey == -1, get from the file
    if ((int)mKey == -1) {
      mKey = (key_t)GetTheKey(C2VKeyFile);
    }
    
    // open the share memory segment as client
    if ((shmid = shmget(mKey,(size_t) (MAX_SHIPC * sizeof(shIpc)),0)) == -1) {
      printf("Client:Shared memory segment was not created for me to attach --- exiting\n");
      exit(2);
    } 
    ClientFlag = 1; // as client    
  }
  // C Side of shared memory  
  else { 
    mKey = ftok(".",(int)seedOrKey_key); 
    // open the share memory segment with create mode 
    if ((shmid = shmget(mKey,(size_t) (MAX_SHIPC * sizeof(shIpc)),IPC_CREAT|IPC_EXCL|0666)) == -1) {
      printf("Initiator:Shared memory segment already exist --- issue a remove...exiting\n");
      // try to detach and retry??
      shmctl(shmid,IPC_RMID,0);
      shmdt(0);
      printf("shmget mKey = %d errono = %s (%d)\n",mKey,
        (errno == EEXIST)        ? "EEXIST" :
        (errno == EINVAL)        ? "EINVAL" :
        (errno == EIDRM)         ? "EIDRM"  :
        (errno == ENOSPC)        ? "ENOSPC" :
        (errno == ENOENT)        ? "ENOENT" :
        (errno == EACCES)        ? "EACCES" :
        (errno == ENOMEM)        ? "ENOMEM" :
        "UNKNOWN",errno);
      if ((shmid = shmget(mKey,(size_t) (MAX_SHIPC * sizeof(shIpc)),0)) == -1) {
        printf("Initiator: Failed second retry as initiator\n");
        printf("shmget mKey = %d errono = %s (%d)\n",mKey,
         (errno == EEXIST)        ? "EEXIST" :
         (errno == EINVAL)        ? "EINVAL" :
         (errno == EIDRM)         ? "EIDRM" :
         (errno == ENOSPC)        ? "ENOSPC" :
         (errno == ENOENT)        ? "ENOENT" :
         (errno == EACCES)        ? "EACCES" :
         (errno == ENOMEM)        ? "ENOMEM" :
         "UNKNOWN",errno);
        exit(2);
      }
      
    }

    ClientFlag = 0; // as initiator
    SetTheKey((int)mKey,C2VKeyFile);
  }

  // NOW ATTACH IT
  segptr = (shIpc *) shmat(shmid, (void *)  0, 0);
  if ((unsigned long)segptr == -1) {
    printf("%s: %s: Failed to attach Shared memory segment with mKey = %x shmid = %x\n", __FUNCTION__, (asClient) ? "Client" : "Initiator", mKey, shmid);
    exit(2);
  }

  // if initiator send mKey to client
  if (!ClientFlag) {

    printf("%s: master: shMem->InitMe\n", __FUNCTION__);
    for (int s=0;s<MAX_SHIPC;s++) {
      (segptr+s)->InitMe(GET_SLOTID(s),GET_CPUID(s));
    }
    printf("%s: master: shMem->InitMe Done\n", __FUNCTION__);
  } else {
    LOGI("%s: Client: shMem->InitMe Done\n",__FUNCTION__);
  }

#endif
}

//
// Desctructor = remove the share segment
//
// Can't have ANY "log" here (except printf)!!!!!!!!
shMem::~shMem () {
  //printf("~shMem:Dettaching Shared memory segment -- you are history\n");
  // dettach first
  shmctl(shmid,IPC_RMID,0);
  shMemDestroy();
}
void shMem::shMemDestroy() {
  //shmctl(shmid,IPC_RMID,0);
  shmdt(segptr);
}
//
// Methods
//
key_t shMem::getKey() { return mKey; }

shIpc *shMem::getPtr() { return segptr; }

void shMem::setFlag(int isClient) { ClientFlag = isClient; }

int shMem::getFlag() { return ClientFlag; }

shIpc *shMem::getIpcPtr(int slotId, int cpuId) {
  return (getPtr() + getOffset(slotId,cpuId));
}
shIpc *shMem::getIpcPtr(int offset) {
  return (getPtr() + offset);
}
int shMem::getOffset(int slotId, int cpuId) {
  return (GET_OFFSET(slotId,cpuId)); // (slotId & 0xf) << 4) | (cpuId & 0xf);
} 
void shMem::getSlotCpuId(int offset,int *slotId, int *cpuId) {
  *slotId = GET_SLOTID(offset); // (offset >> 4) & 0xf;
  *cpuId = GET_CPUID(offset); // offset & 0xf;
} 
int shMem::GetTheKey(const char *keyFile) {
  int key = -1;
#ifdef SIM_ENV_ONLY
  //
  // wait until the file is there
  //
  //LOGI("%s Checking for keyFile=%s\n",__FUNCTION__,keyFile);  
  struct stat sts;
  while ((stat(keyFile, &sts) == -1) && (errno == ENOENT)) {
    sleep(1);
  }
  FILE *fp;
  fp = fopen(keyFile,"r");
  fscanf(fp, "mKey = %08x mActiveMask=%016llx", &key, &mActiveMask);
  fclose(fp);
  LOGI("%s mKey = 0x%08x, mActiveMask = 0x%016llx from %s\n", __FUNCTION__, key, mActiveMask, keyFile);
#endif
  return key;
}

void shMem::SetTheKey(int key, const char *keyFile) {
#ifdef SIM_ENV_ONLY
  // create the file and put the key there
  FILE *fp;
  LOGI("%s key = 0x%08x, mActiveMask = %016llx to %s\n",__FUNCTION__, key, getActiveMask(), keyFile);
  fp = fopen(keyFile,"w");
  fprintf(fp,"mKey = %08x mActiveMask=%016llx", key, getActiveMask());
  fclose(fp);
#endif
}


shMem GlobalShMemory;
