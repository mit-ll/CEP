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

#ifndef SHIPC_H
#define SHIPC_H
// ---------------------------

#ifdef _SOLARIS
#include <thread.h>
#include <synch.h>
#else
#include <pthread.h>
#include <semaphore.h>
#endif

#include "stdTypes.h"

#include "v2c_cmds.h"

//
// Linux/Solaris
//
#ifdef _SOLARIS
// Semaphore stuff
typedef sema_t MY_SEMA_TYPE;
#define MY_SEMA_INIT(s,c)       sema_init(s,c,NULL,NULL)
#define MY_SEMA_WAIT            sema_wait
#define MY_SEMA_POST            sema_post

// Mutex stuff
typedef mutex_t MY_MUTEX_TYPE;
extern  MY_MUTEX_TYPE  globalRanDomMutex;
#define MY_DEF_MUTEX            DEFAULTMUTEX
#define MY_MUTEX_LOCK(m)        mutex_lock(m)
#define MY_MUTEX_UNLOCK(m)      mutex_unlock(m)

// thread
typedef thread_t MY_THREAD_TYPE;
#define MY_THREAD_ID()         thr_self()
#define MY_THREAD_CREATE(aFunc,mySemaId,tid) thr_create(NULL, NULL, aFunc, (void *)(intptr_t)mySemaId, THR_BOUND, tid)
#else
// -------------- LINUX or PTHREAD
// semaphore
typedef sem_t MY_SEMA_TYPE;
#define MY_SEMA_INIT(s,c)       sem_init(s,0,c)
#define MY_SEMA_WAIT            sem_wait
#define MY_SEMA_POST            sem_post

// Mutex stuff
typedef pthread_mutex_t MY_MUTEX_TYPE;
extern  MY_MUTEX_TYPE  globalRanDomMutex;
#define MY_DEF_MUTEX            PTHREAD_MUTEX_INITIALIZER
#define MY_MUTEX_LOCK(m)        pthread_mutex_lock(m)
#define MY_MUTEX_UNLOCK(m)      pthread_mutex_unlock(m)

// thread
typedef pthread_t MY_THREAD_TYPE;
#define MY_THREAD_ID()         pthread_self()
#define MY_THREAD_CREATE(aFunc,mySemaId,tid) { \
  pthread_attr_t attr; \
  pthread_attr_init(&attr); \
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); \
  pthread_create( tid,&attr, aFunc, (void *)(intptr_t)mySemaId);	\
}
#endif



#define PACKET_MAIL_TYPE 0x1

//
// Share Memory Segment between 2 process
//
class shIpc {
  // public stuffs
public:
  // constructors
  shIpc();
  shIpc(int slotId, int cpuId);
  ~shIpc(); 
  //
  // inline methods
  //
  void SetVerbose(int verbose) { mVerbose = verbose; }
  int GetVerbose() { return mVerbose; }
  // to speed up
  void SetPostVerbose(int verbose) { mPostVerbose = verbose; }
  int GetPostVerbose() { return mPostVerbose; }
  //
  void SetId(int slotId, int cpuId) { mSlotId = slotId; mCpuId = cpuId; }
  int IsCmdValid() { return mCmdValid; }
  void SetError(int value=1) { mErrCnt += value; }
  int GetError() { return mErrCnt; }
  void SetThreadDone() { mThreadDone = 1; SetInActiveStatus(); }
  int GetThreadDone() { return mThreadDone; }
  // master 
  void WaitTilCmdDone();
  void WaitTilRemoteCmdNFlagDone();
  void WaitTilRemoteReqDone();
  void InitMe(int slotId, int cpuId);
  void InitSemaphore();
  void SendCmdNwait(u_int32_t cmd);
  int CheckCmdOK();
  void Atomic_Rdw64(u_int64_t adr, int param, int mask, u_int64_t *dat);
  void Write64_BURST(u_int64_t adr, int wordCnt, u_int64_t *dat) ;
  void Write256(u_int32_t adr, u_int32_t *dat) ;
  void Write64(u_int32_t adr, u_int64_t dat) ;
  void Write8(u_int32_t adr, u_int8_t dat) ;
  void PciCfgWrite(int inst, u_int32_t adr, u_int32_t dat) ;
  void WriteStatus(u_int32_t dat) ;
  void WriteMailBox(u_int32_t cmd, u_int32_t dat);
  void Read64_BURST(u_int64_t adr, int wordCnt, u_int64_t *dat) ;
  void Read256(u_int32_t adr, u_int32_t *dat) ;
  u_int64_t Read64(u_int32_t adr) ;
  u_int8_t Read8(u_int32_t adr) ;
  u_int32_t PciCfgRead(int inst, u_int32_t adr) ;
  u_int32_t ReadStatus() ;
  u_int32_t ReadErrorCount() ;
  void Print(const char *fmt, ...) ;
  void Print(char *str) ;
  void PrintVerbose(const char *fmt, ...);
  void PrintPostVerbose(const char *fmt, ...);
//  void Print(char *prefix,char *str);
  void WriteDvtFlag(int msb, int lsb, int value);
  u_int64_t  ReadDvtFlag(int msb, int lsb);
  void RunClk(int numClk);
  u_int32_t GetSimClk();
  void Shutdown();
  void SimCheckPoint(char *saveFile);
  // slave
  void AckCmdNdone(int cmdStatus) ;
  void AckRemoteCmdNdone(int cmdStatus) ;
  void SetAliveStatus() { mCmdStatus = SHIPC_STATUS_ALIVE; mStartFlag=1;}
  int GetAliveStatus() { return mCmdStatus; }
  int IsAliveStatus() { return (mCmdStatus == SHIPC_STATUS_ALIVE); }

  void WaitTilStart();
  
  void SetActiveStatus() { mCmdStatus = SHIPC_STATUS_ACTIVE; }
  void SetInActiveStatus() { mCmdStatus = SHIPC_STATUS_INACTIVE; }
  int IsActiveStatus() { return (mCmdStatus == SHIPC_STATUS_ACTIVE); }
  int IsInActiveStatus() { return (mCmdStatus == SHIPC_STATUS_INACTIVE); }

  void LoadRetData(int cmdStatus, u_int32_t p0) ;
  void LoadRetData(int cmdStatus, u_int64_t p0) ;
  void LoadRetData(int cmdStatus, u_int32_t *p) ;

  void SetCycleCount(u_int32_t value) { mCycleCount = value;}
  u_int32_t GetCycleCount() { return mCycleCount;}

  u_int32_t GetCmd()   { return mCmd; }
  u_int64_t GetAdr()   { return mAdr; }
  u_int32_t GetAdrHi() { return mAdrHi; }
  u_int64_t GetArg(int argNum) { return mPar[argNum];}
  void SetArg(int argNum, u_int64_t value) { mPar[argNum] = value;}
  u_int64_t *GetArgPtr(void) { return (u_int64_t *)mPar;}  
  char *GetStr() { return mStr; }
  char *GetPostStr() { return mPostStr; }
  // check
  int SlotCheckOK(int slotId, int cpuId);

  void I2cAsicWrite32(u_int32_t ctrl, u_int32_t adr, u_int32_t dat) ;
  u_int32_t I2cAsicRead32(u_int32_t ctrl, u_int32_t adr) ;
  //
  // for remote access
  //
  void SetRemoteFlag(int slotId, int cpuId);
  int GetRemoteFlag() { return mRemoteFlag; }
  void ClrRemoteFlag() { mRemoteFlag = 0; }

  void ClrRemoteReq() { mRemoteReq = 0; }
  void SetRemoteReq() { mRemoteReq = 1; }
  int GetRemoteReq() { return mRemoteReq; }


  void SetRemoteCmdValid() { mRemoteCmdValid = 1; }
  void ClrRemoteCmdValid() { mRemoteCmdValid = 0; }
  int IsRemoteCmdValid() { return mRemoteCmdValid ; }

  int GetRemoteOffset() { return mRemoteOffset; }
  void SetRemoteOffset(int value) { mRemoteOffset = value; }

  void SetForceExit(void) { mForceExit = 1; }
  //
  //
  // ==========================================
  // 64-bit addeess space
  // ==========================================
  //
  void Write32_32(u_int32_t adr, u_int32_t dat) ;
  void Write32_16(u_int32_t adr, u_int16_t dat) ;
  void Write32_8 (u_int32_t adr, u_int8_t dat) ;
  void Write64_32(u_int64_t adr, u_int32_t dat) ;
  void Write64_64(u_int64_t adr, u_int64_t dat) ;
  void Write32_64(u_int32_t adr, u_int64_t dat) ; 

  u_int32_t Read32_32(u_int32_t adr) ; 
  u_int16_t Read32_16(u_int32_t adr) ; 
  u_int8_t Read32_8(u_int32_t adr) ; 
  u_int32_t Read64_32(u_int64_t adr) ;
  u_int64_t Read64_64(u_int64_t adr) ;
  u_int64_t Read32_64(u_int32_t adr) ;
  // showtime
  void Framer_RdWr(u_int32_t adr, u_int32_t wrDat, u_int32_t *rdDat);
  void Sample_RdWr(u_int32_t adr, u_int64_t wrDat, u_int64_t *rdDat);  
  //
  // ==========================================
  // use for MailBox
  // ==========================================
  //
  //
  // Set/Get
  //
  void SetMailFlag(int value) { mMailFlag = value; }
  int GetMailFlag() { return mMailFlag; }

  void SetMailStatus(int value) { mMailStatus = value; }
  int GetMailStatus() { return mMailStatus; }

  void SetMailType(int value) { mMailType = value; }
  int  GetMailType() { return mMailType; }

  // in byte
  void SetMailSize(int value) { mMailSize = value; }
  int GetMailSize() { return mMailSize; }
  
  void SetFromAdr(int value) { mFromAdr = value; }
  int GetFromAdr() { return mFromAdr; }

  void SetToAdr(int value) { mToAdr = value; }
  int GetToAdr() { return mToAdr; }
  //
  // quickies
  //
  void WaitTilMailReceived() { while (!GetMailFlag()) ; }
  Int8U *GetMailPtr() { return mMailContent; }
  void UpLoadMailContent(Int8U *srcPtr);
  void DownLoadMailContent(Int8U *dstPtr);
  //
  // quickies
  //
  void SendPacket(int destiId, u_int32_t control, int pktSize, Int8U *pktPtr);
  u_int32_t PacketDetected();
  void ReadPacketContent(int pktSize, Int8U *pktPtr);
  void Read12BytesFromMailbox(int wordPos, u_int32_t *data);
  void Write12Bytes2Mailbox(int wordPos, u_int32_t *data);


private:
  //
  // THIS CLASS IS USED UNDER SHARE MEMORY.. IT SEEMS CAN NOT CONTAIN ANY SPECIAL
  // TYPE SUCH AS ANOTHER CLASS!!!
  //
  //
  // Semaphore
  //
  MY_SEMA_TYPE mSema ;
  //
  // Status
  //
  volatile int mThreadDone;
  volatile int mCmdValid;
  volatile int mCmdStatus;
  volatile int mErrCnt;
  volatile int mSlotId;
  volatile int mCpuId;
  volatile int mVerbose;
  volatile int mPostVerbose;
  // Command
  volatile u_int32_t  mCmd;
  //
  // for remote access
  //
  volatile int mRemoteFlag;
  volatile int mRemoteCmdValid; // set ONLY by systemDeamon!! (synch purposes), clear by TARGET
  // use by DEAMON only
  volatile int mRemoteReq; // set by DEAMON, clear by executed TARGET
  volatile int mRemoteOffset;

  // parameters
  volatile u_int32_t mCycleCount;  
  char mStr[256]; // upto 256 characters
  char mPostStr[256]; // upto 256 characters

  //
  // share MailBox!!!
  //
  volatile int mMailFlag;
  volatile int mMailStatus;
  volatile int mMailType;
  volatile int mMailSize;
  volatile int mFromAdr;
  volatile int mToAdr;
  Int8U mMailContent[1<<14]; // 16K in bytes
  //
  // pthread related
  //
  int mForceExit;
  //
  volatile int mStartFlag;
  // for DPI as well
  volatile u_int32_t mAdrHi; // upper address
  volatile u_int64_t mAdr;
  volatile u_int64_t  mPar[32]; // upto 16 32bits = cache size
};
//
#endif
