//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      shIpc.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include <stdio.h>
#include <unistd.h>

//#define GOTO_SLEEP usleep(10)
//#define GOTO_SLEEP usleep(2)
#define GOTO_SLEEP 


#ifdef DLL_SIM
#include "veriuser.h"
#include "vpi_user.h"
#include "acc_user.h"
#else
#include "stdio.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdarg.h>
#endif
//extern "C" {

#include <string.h>

//#include "share.h"

#include "shMem.h"
#include "shIpc.h"
#include "v2c_cmds.h"

//	   }

#ifndef _SOLARIS
#include <stdlib.h>
#endif

//
// Global stuff
//
MY_MUTEX_TYPE  globalRanDomMutex = MY_DEF_MUTEX; // PTHREAD_MUTEX_INITIALIZER;

// default contructor
shIpc::shIpc() {
//  InitMe(0,0);
}

// client/initiator constructor
shIpc::shIpc(int slotId, int cpuId) {
  InitMe(slotId, cpuId);
}

//
// Init Support functions
//
void shIpc::InitMe(int slotId, int cpuId) {
  SetId(slotId,cpuId);
  SetVerbose(0);
  SetPostVerbose(0);
  SetCycleCount(0xFFFFFFFF);
  mCmdValid = 0;
  mCmdStatus = SHIPC_STATUS_SUCCESS;
  mErrCnt = 0;
  mCmd = 0;
  mStr[0] = '\0';
  mPostStr[0] = '\0';
  mAdr = 0;
  for (int i=0;i<8;i++) { mPar[i] = 0; }
  InitSemaphore();
  mThreadDone = 0;
  // remote access
  mRemoteFlag = 0;
  mRemoteReq = 0;
  mRemoteCmdValid = 0;
  mRemoteOffset = 0;
  // MailBox
  SetMailFlag(0);
  SetMailSize(0);
  SetFromAdr(0);
  SetToAdr(0);
  SetMailType(0);
  SetMailStatus(SHIPC_STATUS_INACTIVE);
  mForceExit = 0;
  mStartFlag = 0;
}
void shIpc::InitSemaphore() {
  //print("shIpc::initSemaphore\n");
  int count  =  1;
  MY_SEMA_INIT(&mSema, count);
}

//
// ======================================================
// Used by "C/C++" side
// ======================================================
//
//#define USE_SEMAPHORE
void shIpc::WaitTilCmdDone() {
#ifdef USE_SEMAPHORE
  MY_SEMA_WAIT(&mSema);
#else
  while (IsCmdValid()) GOTO_SLEEP;
#endif
}

void shIpc::WaitTilStart() {
#ifdef USE_SEMAPHORE
  MY_SEMA_WAIT(&mSema);
#else
  while (mStartFlag == 0) GOTO_SLEEP;
#endif
}

void shIpc::WaitTilRemoteCmdNFlagDone() {
#ifdef USE_SEMAPHORE
  MY_SEMA_WAIT(&mSema);
#else
  while (IsRemoteCmdValid() || GetRemoteFlag()) GOTO_SLEEP;
#endif
}

void shIpc::WaitTilRemoteReqDone() {
#ifdef USE_SEMAPHORE
  MY_SEMA_WAIT(&mSema);
#else
  while (mRemoteReq) GOTO_SLEEP;
#endif
}

//
void shIpc::SetRemoteFlag(int slotId, int cpuId) {
  mRemoteOffset = GET_OFFSET(slotId,cpuId);
  mRemoteFlag = 1; // set the flag
}

void shIpc::SendCmdNwait(u_int32_t cmd) {
//printf("%s: Cmd=0x%x\n",__FUNCTION__,cmd);
  // set the flag and wait
  mCmdStatus = SHIPC_STATUS_PENDING ;
  mCmd = cmd;
  if (!mRemoteFlag) {
    mCmdValid = 1;
    WaitTilCmdDone();
  } else {
    mCmdValid       = 0;
    mRemoteFlag     = 1; // keep the flag set for DEAMON to process
    mRemoteCmdValid = 1; // now set the request!!
    WaitTilRemoteCmdNFlagDone();
  }
  //
  // if I get back here, check to make sure command is done sucessfully
  //
  (void) CheckCmdOK();
  //
  //
  //
#if 0
  if (mForceExit) {
    pthread_exit(NULL); // get out
  }
#endif		  
}
int shIpc::CheckCmdOK() {
  if (( mCmdStatus != SHIPC_STATUS_SUCCESS ) &&
      ( mCmdStatus != SHIPC_STATUS_WARNING ) &&
      ( mCmdValid != 0))
  {
    SetError();		      
  }
  return GetError();
}
//
// Write Functions
//
void shIpc::Atomic_Rdw64(u_int64_t adr, int param, int mask, u_int64_t *dat) {
  // Set data stuff
  mAdr   = adr;
  mAdrHi = param;
  mPar[0] = *dat;
  mPar[1] = mask;
  // set cmd
  SendCmdNwait(SHIPC_ATOMIC_RDW64);
  *dat = mPar[0]; 
}

void shIpc::Write64_BURST(u_int64_t adr, int wordCnt, u_int64_t *dat) {
  // Set data stuff
  mAdr   = adr;
  mAdrHi = wordCnt;
  for (int i=0;i<wordCnt;i++) {
    mPar[i] = dat[i];
  }
  // set cmd
  SendCmdNwait(SHIPC_WRITE64_BURST);
}

void shIpc::Write256(u_int32_t adr, u_int32_t *dat) {
  // Set data stuff
  mAdr = adr;
  for (int i=0;i<8;i++) {
    mPar[i] = dat[i];
  }
  // set cmd
  SendCmdNwait(SHIPC_WRITE256);
}
void shIpc::Write64(u_int32_t adr, u_int64_t dat) {
  mAdr = adr;
  // Set data stuff
  mPar[0] = dat >> 32;
  mPar[1] = dat;
  // set cmd
  SendCmdNwait(SHIPC_WRITE64);
}

void shIpc::I2cAsicWrite32(u_int32_t ctrl,u_int32_t adr, u_int32_t dat) {
  mAdr = adr;
  // Set data stuff
  mPar[0] = dat;
  mPar[1] = ctrl;
  // set cmd
  SendCmdNwait(SHIPC_I2CASIC_WRITE);
}

void shIpc::PciCfgWrite(int inst, u_int32_t adr, u_int32_t dat) {
  mAdrHi = inst;
  mAdr = adr;
  // Set data stuff
  mPar[0] = dat;
  // set cmd
  SendCmdNwait(SHIPC_PCICFGWRITE);
}


void shIpc::WriteStatus(u_int32_t dat) {
  // Set data stuff
  mPar[0] = dat;
  mPar[1] = 0;
  // set cmd
  SendCmdNwait(SHIPC_WRITE_STATUS);
}
void shIpc::WriteMailBox(u_int32_t cmd, u_int32_t dat) {
  // Set data stuff
  mPar[0] = cmd;
  mPar[1] = dat;
  // set cmd
  SendCmdNwait(SHIPC_WRITE_STATUS);
}

void shIpc::WriteDvtFlag(int msb, int lsb, int value) {
  // Set data stuff
  mPar[0] = msb;
  mPar[1] = lsb;
  mPar[2] = value;

  // printf("%s: mPar[0]=0x%016lx, mPar[1]=0x%016lx, mPar[2]=0x%016lx\n",__FUNCTION__,mPar[0], mPar[1], mPar[2]);

  // set cmd
  SendCmdNwait(SHIPC_WRITE_DVT_FLAG);
}
u_int64_t shIpc::ReadDvtFlag(int msb, int lsb) {
  // Set data stuff
  mPar[0] = msb;
  mPar[1] = lsb;
  // set cmd
  SendCmdNwait(SHIPC_READ_DVT_FLAG);
  //printf("%s: mPar[0]=0x%016lx\n",__FUNCTION__,mPar[0]);
  return  mPar[0];
}

//
// Read Functions
//
void shIpc::Read64_BURST(u_int64_t adr, int wordCnt, u_int64_t *dat) {
  mAdr = adr;
  mAdrHi = wordCnt;
  // set cmd
  SendCmdNwait(SHIPC_READ64_BURST);
  // Load data stuff
  for (int i=0;i<wordCnt;i++) {
    dat[i] = mPar[i];
  }
}


void shIpc::Read256(u_int32_t adr, u_int32_t *dat) {
  mAdr = adr;
  // set cmd
  SendCmdNwait(SHIPC_READ256);
  // Load data stuff
  for (int i=0;i<8;i++) {
    dat[i] = mPar[i];
  }
}
u_int64_t shIpc::Read64(u_int32_t adr) {
  mAdr = adr;
  // set cmd
  SendCmdNwait(SHIPC_READ64);
  // Set data stuff
  u_int64_t dat;
  dat = ((u_int64_t) mPar[0]) << 32;
  dat |= (u_int64_t) mPar[1];
  return dat;
}

u_int32_t shIpc::I2cAsicRead32(u_int32_t ctrl,u_int32_t adr) {
  mAdr = adr;
  mPar[0] = ctrl;
  // set cmd
  SendCmdNwait(SHIPC_I2CASIC_READ);
  // Set data stuff
  return  mPar[1];
}
u_int32_t shIpc::PciCfgRead(int inst,u_int32_t adr) {
  mAdrHi = inst;
  mAdr = adr;
  // set cmd
  SendCmdNwait(SHIPC_PCICFGREAD);
  // Set data stuff
  return  mPar[0];
}
u_int32_t shIpc::ReadStatus() {
  SendCmdNwait(SHIPC_READ_STATUS);
  // Set data stuff
  return  mPar[0];
}
u_int32_t shIpc::ReadErrorCount() {
  SendCmdNwait(SHIPC_READ_ERROR_CNT);
  // Set data stuff
  return  mPar[0];
}

u_int32_t shIpc::GetSimClk() {
  // set cmd
  SendCmdNwait(SHIPC_GET_SIM_TIME);
  // Set data stuff
  return ((u_int32_t) mPar[0]);
}

//
// Print Functions
//
void shIpc::PrintVerbose(const char *fmt, ...) {
#ifndef DLL_SIM
  va_list  arglist;
  va_start(arglist,fmt);
  vsprintf(mStr,fmt, arglist);
  va_end(arglist);
  SetVerbose(1);
#endif
}
void shIpc::PrintPostVerbose(const char *fmt, ...) {
#ifndef DLL_SIM
  va_list  arglist;
  va_start(arglist,fmt);
  vsprintf(mPostStr,fmt, arglist);
  va_end(arglist);
  SetPostVerbose(1);
#endif
}

void shIpc::Print(const char *fmt, ...) {
#ifndef DLL_SIM
  va_list  arglist;
  va_start(arglist,fmt);
  vsprintf(mStr,fmt, arglist);
  va_end(arglist);
  SendCmdNwait(SHIPC_PRINTF);
#endif
}
void shIpc::Print(char *str) {
#ifndef DLL_SIM
  strcpy(mStr,str);
  // set cmd
  SendCmdNwait(SHIPC_PRINTF);
#endif
}
#if 0
void shIpc::Print(char *prefix,char *str) {
  strcpy(mStr,prefix);
  strcat(mStr,str);
  // set cmd
  SendCmdNwait(SHIPC_PRINTF);
}
#endif
void shIpc::RunClk(int numClk) {
  mPar[0] = numClk;
  SendCmdNwait(SHIPC_RUNCLK);
}
void shIpc::Shutdown() {
  SendCmdNwait(SHIPC_SHUTDOWN);
}
void shIpc::SimCheckPoint(char *saveFile) {
  strcpy(mStr,saveFile);
  SendCmdNwait(SHIPC_SIMV_SAVE);
}

//
// ======================================================
// Used by Verilog side (client)
// ======================================================
//
void shIpc::AckCmdNdone(int cmdStatus) {
  // clear the flag and post
  mCmdStatus = cmdStatus;
  SetVerbose(0); // clear this flag
  mCmdValid = 0;
#ifdef USE_SEMAPHORE
//  int semValue;
//  sem_getvalue(&mSema,&semValue);
//  printf("shIpc::AckCmdNdone mSema=%x\n",semValue);
//  mSema.__sem_value++;
  MY_SEMA_POST(&mSema);
#endif
}
void shIpc::AckRemoteCmdNdone(int cmdStatus) {
  // clear the flag and post
  mCmdStatus = cmdStatus;
  SetVerbose(0); // clear this flag
  mRemoteCmdValid = 0; // 
#ifdef USE_SEMAPHORE
//  int semValue;
//  sem_getvalue(&mSema,&semValue);
//  printf("shIpc::AckCmdNdone mSema=%x\n",semValue);
//  mSema.__sem_value++;
  MY_SEMA_POST(&mSema);
#endif
}

void shIpc::LoadRetData(int cmdStatus, u_int32_t p0) {
  mPar[0] = p0;
  AckCmdNdone(cmdStatus);
}
void shIpc::LoadRetData(int cmdStatus, u_int64_t p0) {
  mPar[0] = p0 >> 32;
  mPar[1] = p0 ;
  AckCmdNdone(cmdStatus);
}
void shIpc::LoadRetData(int cmdStatus, u_int32_t *p) {
  for (int i=0;i<8;i++) {
    mPar[i] = p[i];
  }
  AckCmdNdone(cmdStatus);
}
int shIpc::SlotCheckOK(int slotId, int cpuId) {
  if ((slotId == mSlotId) && (cpuId == mCpuId)) return 1;
  else return 0;
}

void shIpc::UpLoadMailContent(Int8U *srcPtr) { 
  memcpy(mMailContent,srcPtr,GetMailSize()); 
}
void shIpc::DownLoadMailContent(Int8U *dstPtr) { 
  memcpy(dstPtr,mMailContent,GetMailSize()); 
}
//
// To support HSL1's FPC
//
void shIpc::SendPacket(int destiId, u_int32_t control, int pktSize, Int8U *pktPtr) {
  // load the mail 
  SetToAdr(destiId);
  SetMailSize(pktSize);
  memcpy(mMailContent,pktPtr,pktSize);
  // send command
  mAdr = destiId;
  mPar[0] = control;
  mPar[1] = pktSize;
  SendCmdNwait(SHIPC_SEND_PACKET);
}

u_int32_t shIpc::PacketDetected() {
  SendCmdNwait(SHIPC_READ_PACKET);
  return mPar[0];
}

void shIpc::ReadPacketContent(int pktSize, Int8U *pktPtr) {
  memcpy(pktPtr,mMailContent,pktSize);
}

void shIpc::Read12BytesFromMailbox(int wordPos, u_int32_t *data) {
  int bytePos = wordPos*12;
  // first word
  data[0] = ( ((mMailContent[bytePos+0] & 0xff) << 24) |
	      ((mMailContent[bytePos+1] & 0xff) << 16) |
	      ((mMailContent[bytePos+2] & 0xff) << 8) |
	      ((mMailContent[bytePos+3] & 0xff) << 0));
  // second word
  data[1] = ( ((mMailContent[bytePos+4] & 0xff) << 24) |
	      ((mMailContent[bytePos+5] & 0xff) << 16) |
	      ((mMailContent[bytePos+6] & 0xff) << 8) |
	      ((mMailContent[bytePos+7] & 0xff) << 0));
  // third word
  data[2] = ( ((mMailContent[bytePos+8] & 0xff) << 24) |
	      ((mMailContent[bytePos+9] & 0xff) << 16) |
	      ((mMailContent[bytePos+10] & 0xff) << 8) |
	      ((mMailContent[bytePos+11] & 0xff) << 0));
}
void shIpc::Write12Bytes2Mailbox(int wordPos, u_int32_t *data) {
  int bytePos = wordPos*12; // in unit of 12bytes
  // first word in Big-endian
  mMailContent[bytePos+0] = (data[0] >> 24) & 0xff;
  mMailContent[bytePos+1] = (data[0] >> 16) & 0xff;
  mMailContent[bytePos+2] = (data[0] >> 8) & 0xff;
  mMailContent[bytePos+3] = (data[0] >> 0) & 0xff;
  // second word in Big-endian
  mMailContent[bytePos+4] = (data[1] >> 24) & 0xff;
  mMailContent[bytePos+5] = (data[1] >> 16) & 0xff;
  mMailContent[bytePos+6] = (data[1] >> 8) & 0xff;
  mMailContent[bytePos+7] = (data[1] >> 0) & 0xff;
  // third word in Big-endian
  mMailContent[bytePos+8]  = (data[2] >> 24) & 0xff;
  mMailContent[bytePos+9]  = (data[2] >> 16) & 0xff;
  mMailContent[bytePos+10] = (data[2] >> 8) & 0xff;
  mMailContent[bytePos+11] = (data[2] >> 0) & 0xff;
}
//
// ==========================================
// 64-bit addeess space
// ==========================================
//
void shIpc::Write32_16(u_int32_t adr, u_int16_t dat) {
  mAdr = adr;
  // Set data stuff
  mPar[0] = (u_int32_t) dat;
  // set cmd
  SendCmdNwait(SHIPC_WRITE32_16);
}
void shIpc::Write32_8(u_int32_t adr, u_int8_t dat) {
  mAdr = adr;
  // Set data stuff
  mPar[0] = (u_int32_t) dat;
  // set cmd
  SendCmdNwait(SHIPC_WRITE32_8);
}

void shIpc::Write32_32(u_int32_t adr, u_int32_t dat) {
  mAdr   = adr;
  // Set data stuff
  mPar[0] = dat;
  // set cmd
  SendCmdNwait(SHIPC_WRITE32_32);
}

void shIpc::Write64_64(u_int64_t adr, u_int64_t dat) {
  mAdr   = adr;
  // Set data stuff
  mPar[0] = dat;
  // set cmd
  SendCmdNwait(SHIPC_WRITE64_64);
}
void shIpc::Write32_64(u_int32_t adr, u_int64_t dat) {
  mAdrHi = 0;
  mAdr   = adr;
  // Set data stuff
  mPar[0] = dat >> 32;
  mPar[1] = dat;
  // set cmd
  SendCmdNwait(SHIPC_WRITE32_64);
}
 
void shIpc::Write64_32(u_int64_t adr, u_int32_t dat) {
  mAdrHi = adr >> 32;
  mAdr = adr;
  // Set data stuff
  mPar[0] = dat;
  // set cmd
  SendCmdNwait(SHIPC_WRITE64_32);
}

u_int16_t shIpc::Read32_16(u_int32_t adr) {
  mAdr = adr;
  // set cmd
  SendCmdNwait(SHIPC_READ32_16);
  // Set data stuff
  return ((u_int16_t) mPar[0]);
}
u_int8_t shIpc::Read32_8(u_int32_t adr) {
  mAdr = adr;
  // set cmd
  SendCmdNwait(SHIPC_READ32_8);
  // Set data stuff
  return ((u_int8_t) mPar[0]);
}

u_int32_t shIpc::Read32_32(u_int32_t adr) {
  mAdr = adr;
  // set cmd
  SendCmdNwait(SHIPC_READ32_32);
  // Set data stuff
  return mPar[0];
}

u_int64_t shIpc::Read64_64(u_int64_t adr) {
  mAdr = adr;
  // set cmd
  SendCmdNwait(SHIPC_READ64_64);
  // Set data stuff
  return mPar[0];
}
u_int64_t shIpc::Read32_64(u_int32_t adr) {
  mAdrHi = 0;
  mAdr = adr;
  // set cmd
  SendCmdNwait(SHIPC_READ32_64);
  // Set data stuff
  u_int64_t dat;
  dat = ((u_int64_t) mPar[0]) << 32;
  dat |= (u_int64_t) mPar[1];
  return dat;
}
 
u_int32_t shIpc::Read64_32(u_int64_t adr) {
  mAdrHi = adr >> 32;
  mAdr = adr;
  // set cmd
  SendCmdNwait(SHIPC_READ64_32);
  // Set data stuff
  return  mPar[0];
}

void shIpc::Framer_RdWr(u_int32_t adr, u_int32_t wrDat, u_int32_t *rdDat) {
  mAdr = adr;
  // Set data stuff
  mPar[0] = wrDat;
  // set cmd
  SendCmdNwait(SHIPC_FRAMER_RDWR);
  *rdDat = mPar[1];

}

void shIpc::Sample_RdWr(u_int32_t adr, u_int64_t wrDat, u_int64_t *rdDat) {
  mAdr = adr;
  // Set data stuff
  mPar[0] = wrDat >> 32;
  mPar[1] = wrDat & 0xFFFFFFFF;
  // set cmd
  SendCmdNwait(SHIPC_SAMPLE_RDWR);
  *rdDat = mPar[2];
  *rdDat = (*rdDat << 32) | mPar[3];
}
