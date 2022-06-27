//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      vpp2shMem.cc
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include <math.h>
#include "svdpi.h"
#include "veriuser.h"
#include "acc_user.h"
#include "malloc.h"
#include <sys/stat.h>
#include <errno.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
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
  long unsigned int mask = -1;
  int myId                = tf_getp(1);
  int onOff               = ((long unsigned int)(1<<myId) & GlobalShMemory.getActiveMask()) ? 1 : 0;
  tf_putp ( 2, onOff ); // Yes  
}

void vpp_WaitTilStart() {

  // Get slot and CPU ID
  int slotId  = tf_getp ( 1 );
  int cpuId   = tf_getp ( 2 );
  
  // check if there is a command
  shIpc *ptr = GlobalShMemory.getIpcPtr(slotId, cpuId);
  shIpc *localPtr = ptr;
 
  if (ptr->SlotCheckOK(slotId,cpuId)) {
    ptr->WaitTilStart();
  }
}

void vpp_shMemDestroy() {  
  GlobalShMemory.shMemDestroy();
}

// Replacement for vpp_getShIpcCmd()
void get_v2c_mail(const int slotId, const int cpuId, mailBox *inBox)  {
  shIpc *ptr = GlobalShMemory.getIpcPtr(slotId,cpuId);
  shIpc *localPtr = ptr;


  // Ensure a valid slotId / cpuId is specified
  if (ptr->SlotCheckOK(slotId, cpuId)) {
    int localCmdValid = ptr->IsCmdValid();
    int __shIpc_remoteReq = ptr->GetRemoteReq();

    // process remote first => change the pointer
    if (__shIpc_remoteReq) {
      int remoteOffset = ptr->GetRemoteOffset();
      ptr = GlobalShMemory.getIpcPtr(remoteOffset);
    }

    inBox->mActive    = 1;    
    Int32U curTime    = (Int32U)(tf_gettime() * pow(10,tf_gettimeprecision()+9));
    
    // save the time
    int __shIpc_cmdValid = localCmdValid | __shIpc_remoteReq;
    
    // Is there a valid command?
    if (__shIpc_cmdValid) {
      
      // Save the command
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
        sprintf(oStr,"%8d %s %s",curTime,svGetNameFromScope(svGetScope()),ptr->GetStr());  
        io_printf(oStr);
  
        if (__shIpc_remoteReq) {
          ptr->AckRemoteCmdNdone(SHIPC_STATUS_SUCCESS);
          localPtr->ClrRemoteReq();
        } else {
          ptr->AckCmdNdone(SHIPC_STATUS_SUCCESS); 
        } // end if (__shIpc_remoteReq)

        inBox->mCmdValid  = 0; 
        inBox->mRemoteReq = 0; 
      } else {
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
 
        memcpy(&(inBox->mPar[0]),ptr->GetArgPtr(), 32 * sizeof(u_int64_t)); 

        if (ptr->GetVerbose()) {
          // print the verbose string before execute the command
          sprintf(oStr,"%8d %s %s",curTime,svGetNameFromScope(svGetScope()),ptr->GetStr());
          io_printf(oStr);
        } // end if (ptr->GetVerbose())
      } // end if (__shIpc_cmd == SHIPC_PRINTF)

    } else if (ptr->IsInActiveStatus() || ptr->GetThreadDone()) { // thread done?
      io_printf((char *)"%s InactiveStatus detected..Shutting down!!!\n",svGetNameFromScope(svGetScope()));
      inBox->mActive    = 0;  
      inBox->mCmdValid  = 0; 
      inBox->mRemoteReq = 0; 
    } else {
      inBox->mCmdValid  = 0; 
      inBox->mRemoteReq = 0; 
    }
  // SlotCheckFailed
  } else {
    io_printf((char *)"ERROR: %s vpp_getShIpcCmd SlotCheckFailed  verilog=%x_%x\n",svGetNameFromScope(svGetScope()), slotId,cpuId);
    inBox->mCmdValid  = 0; //tf_putp ( 3, 0 ); // no cmdValid
    inBox->mRemoteReq = 0; //tf_putp ( 4, 0 );
  } // if (ptr->SlotCheckOK(slotId, cpuId))
} // get_v2c_mail

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
  memcpy(ptr->GetArgPtr(),&(inBox->mPar[0]), 32 * sizeof(u_int64_t));
  
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

