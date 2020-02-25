//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

//
// =============================
// This is the file where all the overload codes should be 
// =============================
//
#ifndef PORTABLE_IO_H
#define PORTABLE_IO_H

#include <stdio.h>

// =============================
// See from simulation/verilog/HW side
// =============================
#if defined(_SIM_HW_ENV)

#define LOGD   my_io_logD
#define LOGW   my_io_logW
#define LOGI   my_io_logI
#define LOGE   my_io_logE
#define LOGF   my_io_logF

// =============================
// See from simulation/C side
// =============================
#elif defined(_SIM_SW_ENV)

#include "v2c_sys.h"

#if 1
#define LOGI   v2cLogI
#define LOGW   v2cLogI
#define LOGE   v2cLogE
#define LOGF   v2cLogF
#else
#define LOGI   printf
#define LOGW   printf
#define LOGE   printf
#define LOGF   printf
#endif

#ifdef USE_DPI
#define DUT_WRITE32_64(a,d) sim_Write64_64(a,d)
#define DUT_READ32_64(a,d)  d=sim_Read64_64(a)

#define DDR3_WRITE(a,d) sim_Write64_64(a,d)
#define DDR3_READ(a,d)  d=sim_Read64_64(a)

#else
#define DUT_WRITE32_64(a,d) sim_Write32_64(a,d)
#define DUT_READ32_64(a,d)  d=sim_Read32_64(a)

#define DDR3_WRITE(a,d) sim_Write32_64(a,d)
#define DDR3_READ(a,d)  d=sim_Read32_64(a)

#endif

#define DUT_WRITE_DVT(msb,lsb,val) sim_WriteDvtFlag(msb,lsb,val)
#define DUT_READ_DVT(msb,lsb)      sim_ReadDvtFlag(msb,lsb)

#define DUT_SetInActiveStatus      sim_SetInActiveStatus

#define DUT_RUNCLK(x)              sim_RunClk(x)

// framer
#define DUT_FRAMER_RDWR(a,wd,rd) sim_Framer_RdWr(a,wd,rd)
#define DUT_SAMPLE_RDWR(a,wd,rd) sim_Sample_RdWr(a,wd,rd)

// =============================
// See from other enviroment (lab/Linux/bareMetal/etc..)
// =============================
#elif defined(LINUX_MODE)

#include "cep_diag.h"
#include "cep_io.h"
#define LOGI   THR_LOGI
#define LOGW   THR_LOGW
#define LOGE   THR_LOGE
#define LOGF   THR_LOGF
// so core display
#define PRNT   printf

#define DUT_WRITE32_64(a,d) lnx_cep_write(a,d)
#define DUT_READ32_64(a,d)  { d = lnx_cep_read(a); }

#define DDR3_WRITE(a,d) *(volatile uint64_t *)((intptr_t)a)=d
#define DDR3_READ(a,d)  d=*(volatile uint64_t *)((intptr_t)a)

//
// =============================
// See from other enviroment (lab/Linux/bareMetal/etc..)
// =============================
#else

#define LOGI   printf
#define LOGW   printf
#define LOGE   printf
#define LOGF   printf

#ifdef BARE_MODE
#define DUT_WRITE32_64(a,d) *(volatile uint64_t *)((intptr_t)a)=d
#define DUT_READ32_64(a,d)  d=*(volatile uint64_t *)((intptr_t)a)

#define DDR3_WRITE(a,d) *(volatile uint64_t *)((intptr_t)a)=d
#define DDR3_READ(a,d)  d=*(volatile uint64_t *)((intptr_t)a)

// ONLY in bare metal MODE
#define DUT_WRITE32_32(a,d) *reinterpret_cast<volatile uint32_t *>(a)=d
#define DUT_READ32_32(a,d)  d=*reinterpret_cast<volatile uint32_t *>(a)

#else
#define DUT_WRITE32_64(a,d) *reinterpret_cast<volatile uint64_t *>(a)=d
#define DUT_READ32_64(a,d)  d=*reinterpret_cast<volatile uint64_t *>(a)

#define DDR3_WRITE(a,d) *reinterpret_cast<volatile uint64_t *>(a)=d
#define DDR3_READ(a,d)  d=*reinterpret_cast<volatile uint64_t *>(a)

#endif

#endif

//
// =============================
// common for all plaforms or derived from above
// =============================
//
#define thread_timed_yield()
#define MAYBE_YIELD(y) if ((y++ & 0x1000) == 0) thread_timed_yield()


// -----------------------------
#endif

//
// can't allow name to be mangle
//
#include <math.h>
#include "svdpi.h"
//
#include "veriuser.h"
#include "acc_user.h"
#include "malloc.h"
#include <sys/stat.h>
#include <errno.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

//#include "vpp_ipc_pli.h"

#include "share.h"
#include "v2c_cmds.h"
#include "vpp2shMem.h"
#include "shMem.h"
#include "shIpc.h"


// global stuffs
unsigned long globalSemaPtr[MAX_SEMAPHORE]; // max number of semaphore
int globalCurSema = 0;
int globalDummy = 0;

MY_THREAD_TYPE Gtid[MAX_SEMAPHORE];

int tidInitialized = 0;
char *GInstance[MAX_SEMAPHORE];
MY_MUTEX_TYPE  Global_reqMutex = MY_DEF_MUTEX;


//shMem GlobalShMemory;
//
// setup shMemory
//
int vpp_shMemWakeup() {
  //
  char StartFile[32];
  int slotId = tf_getp ( 1 );
  int cpuId = tf_getp ( 2 );
  sprintf(StartFile,"Slot0x%04x_%04x.KEY",slotId,cpuId);
  struct stat sts;
  if ((stat(StartFile, &sts) == -1) && (errno == ENOENT)) {
    tf_putp ( 3, 0 );
  } else {
    tf_putp ( 3, 1 );
  }
}

int __shMemIni_done_from_client=0;
int shMemInit(key_t key) {
  // call the constructor
  //io_printf((char *)"%s Client: Opening share memory segment for key=%x\n",tf_mipname(),key);    
  GlobalShMemory.shMemInit(1, (key_t)key); // as client 
  return(0);
}

void vpp_shMemInit() {
  // get the first parameter
  key_t key     = (key_t)tf_getp(1);
  //io_printf((char *)"%s Client: Executing vpp_shMemInit for key=%x\n",tf_mipname(), key);
  shMemInit(key);
}

// check if my C driver is ON
void vpp_isCActive() {
  long unsigned int mask=-1;
  int myId = tf_getp(1);
  int onOff = ((long unsigned int)(1<<myId) & GlobalShMemory.getActiveMask()) ? 1 : 0;
  //io_printf("%s: myId=%d mask=0x%016llx is onOff=%d\n",__FUNCTION__,myId,GlobalShMemory.getActiveMask(), onOff);
  tf_putp ( 2, onOff ); // Yes  
}

void vpp_WaitTilStart() {
  //acc_initialize();
  // get slot ID, cpuId
  int slotId = tf_getp ( 1 );
  int cpuId = tf_getp ( 2 );
  // check if there is a command
  shIpc *ptr = GlobalShMemory.getIpcPtr(slotId,cpuId);
  shIpc *localPtr = ptr;
  //
  //io_printf((char *)"%s Entering slotId=%d cpuId=%d\n",tf_mipname(),slotId,cpuId);  
  if (ptr->SlotCheckOK(slotId,cpuId)) {
    //    int __shIpc_cmdValid =  ptr->IsCmdValid();
    ptr->WaitTilStart();
  }
}

void vpp_shMemDestroy() {  
  GlobalShMemory.shMemDestroy();
}

//
// =========================================
// DPI : faster ???
// =========================================
//
// Replacement for vpp_getShIpcCmd()
void get_v2c_mail(const int slotId, const int cpuId, mailBox *inBox)  {
  shIpc *ptr = GlobalShMemory.getIpcPtr(slotId,cpuId);
  shIpc *localPtr = ptr;
  if (ptr->SlotCheckOK(slotId,cpuId)) {
    int localCmdValid = ptr->IsCmdValid();
    int __shIpc_remoteReq = ptr->GetRemoteReq();
    // process remote first => change the pointer
    if (__shIpc_remoteReq) {
      int remoteOffset = ptr->GetRemoteOffset();
      ptr = GlobalShMemory.getIpcPtr(remoteOffset);
    }
    inBox->mActive    = 1;		
    Int32U curTime = (Int32U)(tf_gettime() * pow(10,tf_gettimeprecision()+9));
    // save the time
    int __shIpc_cmdValid = localCmdValid | __shIpc_remoteReq;
    if (__shIpc_cmdValid) { // get the rest of the argument
      Int32U __shIpc_cmd = ptr->GetCmd();
      // check if this is printf command or verbose mode
      char oStr[1024];
      if (ptr->GetPostVerbose()) {
	sprintf(oStr,"%8d %s %s",ptr->GetCycleCount(),svGetNameFromScope(svGetScope()),ptr->GetPostStr()); 
	io_printf(oStr);
	ptr->SetPostVerbose(0);
      }
      // now update curTime after GetPostVerbose()
      ptr->SetCycleCount(curTime);
      if (__shIpc_cmd == SHIPC_PRINTF) {
	// just print on done!!!
	sprintf(oStr,"%8d %s %s",curTime,svGetNameFromScope(svGetScope()),ptr->GetStr());	
	io_printf(oStr);
	// done
	if (__shIpc_remoteReq) {
	  // clear remote 
	  ptr->AckRemoteCmdNdone(SHIPC_STATUS_SUCCESS);
	  // clear the remote req
	  localPtr->ClrRemoteReq();
	} else {
	  ptr->AckCmdNdone(SHIPC_STATUS_SUCCESS); 
	}
	inBox->mCmdValid  = 0; 
	inBox->mRemoteReq = 0; 
      } else {
	// ONLY LOCAL!!!!
	if (__shIpc_cmd == SHIPC_SHUTDOWN) {
	  inBox->mActive    = 0;			  
	  ptr->SetError(0);
	  ptr->SetThreadDone();
	}
	// loaded the verilog
	inBox->mCmdValid  = __shIpc_cmdValid;  
	inBox->mRemoteReq = __shIpc_remoteReq; 
	inBox->mCmd       = __shIpc_cmd;       
	inBox->mAdrHi     = ptr->GetAdrHi();   
	inBox->mAdr       = ptr->GetAdr();     
	//
	memcpy(&(inBox->mPar[0]),ptr->GetArgPtr(), 8 * sizeof(u_int64_t));	
	// print it
	if (ptr->GetVerbose()) {
	  // print the verbose string before execute the command
	  sprintf(oStr,"%8d %s %s",curTime,svGetNameFromScope(svGetScope()),ptr->GetStr());
	  io_printf(oStr);
	}
	// wait 
      }
    } else if (ptr->IsInActiveStatus() || ptr->GetThreadDone()) { // thread done?
      io_printf((char *)"%s InactiveStatus detected..Shutting down!!!\n",svGetNameFromScope(svGetScope()));
      inBox->mActive    = 0;	
      inBox->mCmdValid  = 0; 
      inBox->mRemoteReq = 0; 
    } else {
      inBox->mCmdValid  = 0; 
      inBox->mRemoteReq = 0; 
    }
  } else {
    io_printf((char *)"ERROR: %s vpp_getShIpcCmd SlotCheckFailed  verilog=%x_%x\n",svGetNameFromScope(svGetScope()), slotId,cpuId);
    inBox->mCmdValid  = 0; //tf_putp ( 3, 0 ); // no cmdValid
    inBox->mRemoteReq = 0; //tf_putp ( 4, 0 );
  }
}

// Replacement for vpp_clearShIpcCmd()
void send_v2c_mail(const int slotId, const int cpuId, const mailBox *inBox) {
  int __shIpc_remoteReq = 0; // inBox->mRemoteReq; // tf_getp ( 4 ); // was it a remote command??
  // check if there is a command
  shIpc *ptr = GlobalShMemory.getIpcPtr(slotId,cpuId);
  shIpc *localPtr = ptr;

  if (__shIpc_remoteReq) { // change pointer
    int remoteOffset = ptr->GetRemoteOffset();
    ptr = GlobalShMemory.getIpcPtr(remoteOffset);
  }
  memcpy(ptr->GetArgPtr(),&(inBox->mPar[0]), 8 * sizeof(u_int64_t));
  
  if (__shIpc_remoteReq) { 
    // clear remote 
    ptr->AckRemoteCmdNdone(SHIPC_STATUS_SUCCESS);
    // clear the remote req
    localPtr->ClrRemoteReq();    
  } else if (ptr->SlotCheckOK(slotId,cpuId)) {
    // done
    ptr->AckCmdNdone(SHIPC_STATUS_SUCCESS);
  } else {
    io_printf((char *)"ERROR: %s vpp_clearShIpcCmd SlotCheckFailed  verilog=%x_%x\n",svGetNameFromScope(svGetScope()), slotId,cpuId);
  }
}

