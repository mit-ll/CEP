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

#include "stdTypes.h"
#include "random48.h"

#include "access.h"

// global
int gActiveSlotId=0;
int gActiveLocalId=0;
//int access::mUseShIpc = 0;
int access::mVerbose = 0;

//
// Contructors/Destructor
//
access::access() {
  mSlotId = gActiveSlotId;
  mLocalId = gActiveLocalId;
//  mVerbose = 1;
  mLogLevelMask = INIT_LOG_LEVEL;
  mUseShIpc = 0;
  MaybeAThread();
}
access::access(int slotId, int localId, int verbose) {
  mSlotId  = slotId;
  mLocalId = localId;
//  mVerbose = verbose;
  mLogLevelMask = ACCESS_LOG_LEVEL;
  mUseShIpc = 0;
  MaybeAThread();
}

access::~access() {}
//
// Check if it is a thread then set the mUseShIpc and slotId/cpuId
//
void access::MaybeAThread() {
#ifdef SIM_ENV_ONLY
#ifndef DLL_SIM  
  shLog log;
  int slot, cpu;
  if (log.isAThread(&slot,&cpu)) {
    mSlotId = slot;
    mLocalId = cpu;
    SetUseShIpcFlag(1);
    ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
  } 
#endif
#endif
}

//
// ===========================
// Write Acess
// ===========================
//
int access::Atomic_Rdw64(u_int64_t address, int param, int mask, u_int64_t *data) {
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM  
#else
  if (0) { // GetVerbose()) {
    ptr->PrintVerbose("access::Write64_BURST mSlotId=%d mLocalId=%d address=%016llx dat=%016llx\n",
		      mSlotId,mLocalId,address,data);
  }
  ptr->Atomic_Rdw64(address,param,mask,data);
#endif
#else
  // Not supported for now!!
  //*reinterpret_cast<volatile Int32U *>(address) = data;
#endif
  return 0;
}

int access::Write64_BURST(u_int64_t address, int wordCnt, u_int64_t *data) {
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM  
#else
  if (0) { // GetVerbose()) {
    ptr->PrintVerbose("access::Write64_BURST mSlotId=%d mLocalId=%d address=%016llx dat=%016llx\n",
		      mSlotId,mLocalId,address,data[0]);
  }
  ptr->Write64_BURST(address,wordCnt,data);
#endif
#else
  // Not supported for now!!
  //*reinterpret_cast<volatile Int32U *>(address) = data;
#endif
  return 0;
}

int access::Read64_BURST(u_int64_t address, int wordCnt, u_int64_t *data) {
  int errCnt =0;
#ifdef SIM_ENV_ONLY
  //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
  ptr->Read64_BURST(address,wordCnt,data);
  if (0) { // GetVerbose()) {
    ptr->PrintPostVerbose("access::Read32_32 mSlotId=%d mLocalId=%d address=%08x dat=%016llx\n",mSlotId,mLocalId,address,data);
  }
#endif
  return errCnt;
}


void access::Write32_32(u_int32_t address, u_int32_t data) {
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM  
#else
  if (0) { // GetVerbose()) {
    ptr->PrintVerbose("access::Write64_64 mSlotId=%d mLocalId=%d address=%016llx dat=%016llx\n",
		      mSlotId,mLocalId,address,data);
  }
  ptr->Write32_32(address,data);
#endif
#else
  *reinterpret_cast<volatile Int32U *>(address) = data;
#endif

}

void access::Write32_16(u_int32_t address, u_int16_t data) {
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM  
#else
  if (0) { // GetVerbose()) {
    ptr->PrintVerbose("access::Write64_64 mSlotId=%d mLocalId=%d address=%016llx dat=%016llx\n",
		      mSlotId,mLocalId,address,data);
  }
  ptr->Write32_16(address,data);
#endif
#else
  *reinterpret_cast<volatile u_int16_t *>(address) = data;
#endif

}

void access::Write32_8(u_int32_t address, u_int8_t data) {
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM  
#else
  if (0) { // GetVerbose()) {
    ptr->PrintVerbose("access::Write64_64 mSlotId=%d mLocalId=%d address=%016llx dat=%016llx\n",
		      mSlotId,mLocalId,address,data);
  }
  ptr->Write32_8(address,data);
#endif
#else
  *reinterpret_cast<volatile u_int8_t *>(address) = data;
#endif

}


void access::Write32_64(u_int32_t address, u_int64_t data) {
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM  
#else
  if (0) { // GetVerbose()) {
    ptr->PrintVerbose("access::Write64_64 mSlotId=%d mLocalId=%d address=%016llx dat=%016llx\n",
		      mSlotId,mLocalId,address,data);
  }
  ptr->Write32_64(address,data);
#endif
#else
  *reinterpret_cast<volatile Int32U *>(address) = data;
#endif

}
void access::Write64_64(u_int64_t address, u_int64_t data) {
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM  
#else
  if (0) { // GetVerbose()) {
    ptr->PrintVerbose("access::Write64_64 mSlotId=%d mLocalId=%d address=%016llx dat=%016llx\n",
		      mSlotId,mLocalId,address,data);
  }
  ptr->Write64_64(address,data);
#endif
#else
  *reinterpret_cast<volatile Int64U *>(address) = data;
#endif
}

void access::RunClk(int numClk) {
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM
#else
  //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
  if (GetVerbose()) {
    ptr->PrintVerbose("access::RunClk mSlotId=%d mLocalId=%d numClk=%08x\n",mSlotId,mLocalId,numClk);
  }
  ptr->RunClk(numClk);
#endif
#else
  for (volatile int i=0;i<numClk;i++) ;
#endif

}
//
// ===========================
// Read Acess
// ===========================
//
unsigned access::GetSimClk() {
  unsigned dat; // ,datH;
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM
#else
  //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
  dat = (unsigned) ptr->GetSimClk();
  if (GetVerbose()) {
    ptr->PrintPostVerbose("access::GetSimClk mSlotId=%d mLocalId=%d dat=%08x\n",mSlotId,mLocalId,dat);
  }
#endif
#else
  dat = 0; // *reinterpret_cast<volatile unsigned *>(address);
  if (GetVerbose()) {
      LOGI("access::GetSimClk dat=%08x\n",dat);
  }
#endif
  return dat;
}

u_int8_t access::Read32_8(u_int32_t address) {
  u_int8_t data=0;
#ifdef SIM_ENV_ONLY
  //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
  data =  ptr->Read32_8(address);
  if (0) { // GetVerbose()) {
    ptr->PrintPostVerbose("access::Read32_8 mSlotId=%d mLocalId=%d address=%08x dat=%016llx\n",mSlotId,mLocalId,address,data);
  }
#endif
  return data;
}

u_int16_t access::Read32_16(u_int32_t address) {
  u_int16_t data=0;
#ifdef SIM_ENV_ONLY
  //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
  data =  ptr->Read32_16(address);
  if (0) { // GetVerbose()) {
    ptr->PrintPostVerbose("access::Read32_16 mSlotId=%d mLocalId=%d address=%08x dat=%016llx\n",mSlotId,mLocalId,address,data);
  }
#endif
  return data;
}

u_int32_t access::Read32_32(u_int32_t address) {
  u_int32_t data=0;
#ifdef SIM_ENV_ONLY
  //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
  data =  ptr->Read32_32(address);
  if (0) { // GetVerbose()) {
    ptr->PrintPostVerbose("access::Read32_32 mSlotId=%d mLocalId=%d address=%08x dat=%016llx\n",mSlotId,mLocalId,address,data);
  }
#endif
  return data;
}

u_int64_t access::Read32_64(u_int32_t address) {
  u_int64_t data=0;
#ifdef SIM_ENV_ONLY
  //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
  data =  ptr->Read32_64(address);
  if (0) { // GetVerbose()) {
    ptr->PrintPostVerbose("access::Read32_64 mSlotId=%d mLocalId=%d address=%08x dat=%016llx\n",mSlotId,mLocalId,address,data);
  }
#endif
  return data;
}
u_int64_t access::Read64_64(u_int64_t address) {
  u_int64_t data=0;
#ifdef SIM_ENV_ONLY
  //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
  data =  ptr->Read64_64(address);
  if (0) { // GetVerbose()) {
    ptr->PrintPostVerbose("access::Read64_64 mSlotId=%d mLocalId=%d address=%08x dat=%016llx\n",mSlotId,mLocalId,address,data);
  }
#endif
  return data;
}

int access::Framer_RdWr(u_int32_t address, u_int32_t wrDat, u_int32_t *rdDat) {
  int errCnt=0;
#ifdef SIM_ENV_ONLY
  //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
    ptr->Framer_RdWr(address,wrDat,rdDat);
    if (0) { // GetVerbose()) {
      ptr->PrintPostVerbose("access::Framer_RdWr mSlotId=%d mLocalId=%d address=%08x dat=%08x/%08x\n",mSlotId,mLocalId,address,wrDat,*rdDat);
    }
#endif
    return errCnt;
}

int access::Sample_RdWr(u_int32_t address, u_int64_t wrDat, u_int64_t *rdDat) {
  int errCnt=0;
#ifdef SIM_ENV_ONLY
  //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
    ptr->Sample_RdWr(address,wrDat,rdDat);
    if (0) { // GetVerbose()) {
      ptr->PrintPostVerbose("access::Framer_RdWr mSlotId=%d mLocalId=%d address=%08x dat=%08x/%08x\n",mSlotId,mLocalId,address,wrDat,*rdDat);
    }
#endif
    return errCnt;
}

//
// ===========================
// Write Modify
// ===========================
//


//
// ===========================
// Compares
// ===========================
//
int access::ReadNCompare(unsigned address, unsigned expDat, unsigned mask) {
  unsigned rdDat=0; //, datH;
  int notOK = (rdDat ^ expDat) & mask;
  if (notOK) {
    LOGE("ERROR: access::ReadNCompare mismatch=%08x expDat=%08x actDat=%08x mask=%08x\n",address,expDat,rdDat,mask);
  }
  else if (GetVerbose()) {
    LOGI("access::ReadNCompare OK address=%08x expDat=actDat=%08x mask=%08x\n",address,rdDat,mask);
  }
  return notOK;
}


//
// ===========================
// Run Program
// ===========================
//
int access::EnableShIpc(int onOff) {

  return 0;
}

void access::WriteDvtFlag(int msb, int lsb, int value) {
  if (0) { // GetVerbose()) {
    LOGI("access::WriteDvtFlag %d:%d=%08x\n",msb,lsb,value);
  }
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM
  // no support
#else
  if (0) { // !GetUseShIpcFlag()) {
    // no support yet!!!
  } else {
    //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
    //if (GetVerbose()) {
    //  ptr->PrintVerbose("access::WriteDvtFlag mSlotId=%d mLocalId=%d %d:%d=%08x\n",mSlotId,mLocalId,msb,lsb,value);
    //}
    ptr->WriteDvtFlag(msb,lsb,value);
  }
#endif
#else
  //if (GetVerbose()) {
  //  LOGI("access::WriteDvtFlag mSlotId=%d mLocalId=%d %d:%d=%08x\n",mSlotId,mLocalId,msb,lsb,value);
  //}
#endif
}

u_int64_t access::ReadDvtFlag(int msb, int lsb) {
  u_int64_t value = -1;
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM
  // no support
#else
  if (0) { // !GetUseShIpcFlag()) {
    // no support yet!!!
  } else {
    //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
    value = ptr->ReadDvtFlag(msb,lsb);
    //if (GetVerbose()) {
    //ptr->PrintVerbose("access::ReadDvtFlag mSlotId=%d mLocalId=%d %d:%d=%08x\n",mSlotId,mLocalId,msb,lsb,value);
    //}
  }
#endif
#else
  //if (GetVerbose()) {
  //  LOGI("access::ReadDvtFlag mSlotId=%d mLocalId=%d %d:%d=%08x\n",mSlotId,mLocalId,msb,lsb,value);
  //}
#endif
  if (0) { // GetVerbose()) {
    //LOGI("access::ReadDvtFlag %d:%d=%08x\n",msb,lsb,value);
  }
  return value;

}

int  access::SetInActiveStatus(void) {

#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM
  // no support
#else
  if (0) { // !GetUseShIpcFlag()) {
    // no support yet!!!
  } else {
    //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
    //if (GetVerbose()) {
    //  ptr->PrintVerbose("access::WriteDvtFlag mSlotId=%d mLocalId=%d %d:%d=%08x\n",mSlotId,mLocalId,msb,lsb,value);
    //}
    ptr->SetInActiveStatus();
  }
#endif
#else
  //if (GetVerbose()) {
  //  LOGI("access::WriteDvtFlag mSlotId=%d mLocalId=%d %d:%d=%08x\n",mSlotId,mLocalId,msb,lsb,value);
  //}
#endif
  //
  return 0;
}

int access::ReadStatus() {
  int value = -1;
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM
  // no support
#else
  if (0) { // !GetUseShIpcFlag()) {
    // no support yet!!!
  } else {
    //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
    value = ptr->ReadStatus();
    //if (GetVerbose()) {
    //  ptr->PrintVerbose("access::ReadStatus mSlotId=%d mLocalId=%d %08x\n",mSlotId,mLocalId,value);
    //}
  }
#endif
#else
  //if (GetVerbose()) {
  //  LOGI("access::ReadStatus mSlotId=%d mLocalId=%d %08x\n",mSlotId,mLocalId,value);
  //}
#endif
  if (GetVerbose()) {
    LOGI("access::ReadStatus mSlotId=%d mLocalId=%d %08x\n",mSlotId,mLocalId,value);
  }
  return value;

}

int access::ReadErrorCount() {
  int value = -1;
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM
  // no support
#else
  if (0) { // !GetUseShIpcFlag()) {
    // no support yet!!!
  } else {
    //shIpc *ptr = GlobalShMemory.getIpcPtr(GetSlotId(),GetLocalId());
    value = ptr->ReadErrorCount();
    if (GetVerbose()) {
      ptr->PrintVerbose("access::ReadErrorCount mSlotId=%d mLocalId=%d %08x\n",mSlotId,mLocalId,value);
    }
  }
#endif
#else
  if (GetVerbose()) {
    LOGI("access::ReadErrorCount mSlotId=%d mLocalId=%d %08x\n",mSlotId,mLocalId,value);
  }
#endif
  return value;

}

void access::Print(const char *fmt, ...) {
#ifdef SIM_ENV_ONLY
#ifdef DLL_SIM
#else
  char tmp[1024];
  va_list  arglist;
  va_start(arglist,fmt);
  vsprintf(tmp,fmt, arglist);
  va_end(arglist);
  ptr->Print(tmp);
#endif
#endif
}
