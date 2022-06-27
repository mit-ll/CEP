//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      vpp2shMem.h
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
#ifndef VPP2SHMEM_H
#define VPP2SHMEM_H


#define MAX_SEMAPHORE 16
extern unsigned long globalSemaPtr[]; // max number of semaphore
extern int globalCurSema;
extern int tidInitialized;
extern MY_THREAD_TYPE Gtid[MAX_SEMAPHORE];
extern char *GInstance[MAX_SEMAPHORE];
extern MY_MUTEX_TYPE  Global_reqMutex;

//
typedef struct {
  int mCmdValid;
  int mRemoteReq;
  int mActive;
  unsigned int mCmd;
  unsigned int mAdrHi; // upper address
  unsigned int mCycleCount;    
  u_int64_t mAdr;
  u_int64_t mPar[8]; // upto 16 32bits = cache size
  char mStr[256]; // upto 256 characters
  char mPostStr[256]; // upto 256 characters
} mailBox;


extern "C" {
  int shMemInit(key_t key);
  void vpp_shMemInit();
  void vpp_shMemDestroy();
  void vpp_isCActive();
  void vpp_WaitTilStart();  
  
  int shMem_Send(int packetId);
  void vpp_shMem_Send();
  // not to use pipe to wakeup client on sim side
  int vpp_shMemWakeup();

  // DPI-C
  void get_v2c_mail(const int slotId, const int cpuId, mailBox *inBox) ;
  void send_v2c_mail(const int slotId, const int cpuId, const mailBox *inBox) ;  
}


// VPP_PKGEN_H
#endif 
