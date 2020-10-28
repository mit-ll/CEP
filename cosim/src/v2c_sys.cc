//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "v2c_cmds.h"
#include "v2c_sys.h"

#ifndef DLL_SIM
#include "shLog.h"

char bStr[128];


void v2cLogI(const char *fmt, ...) {
  char gStaticBuffer[2048];
  char gStaticBuffer2[2048];
  char hexV[3];
  // copy from Monte debugPrintf
  int lenny = 0;
  va_list  arglist;
  va_start(arglist,fmt);

  strcpy(gStaticBuffer,"CLOG: ");
  lenny = vsprintf (gStaticBuffer2, fmt, arglist );
  va_end(arglist);
  //
  strcat(gStaticBuffer,gStaticBuffer2);
  //printf("%s: %s\n",__FUNCTION__,gStaticBuffer);
  shLog log;
  int slotId, cpuId;
  if (log.isAThread(&slotId,&cpuId)) {
    log.shMem_printf(gStaticBuffer);
  } else {
    printf("%s: ERROR: can't print %s",__FUNCTION__,gStaticBuffer);    
  }
  
}

void v2cLogE(const char *fmt, ...) {
  char gStaticBuffer[2048];
  char gStaticBuffer2[2048];
  char hexV[3];
  // copy from Monte debugPrintf
  int lenny = 0;
  va_list  arglist;
  va_start(arglist,fmt);

  strcpy(gStaticBuffer,"C_ERROR: ");
  lenny = vsprintf (gStaticBuffer2, fmt, arglist );
  va_end(arglist);

  strcat(gStaticBuffer,gStaticBuffer2);

  shLog log;
  int slotId, cpuId;
  if (log.isAThread(&slotId,&cpuId)) {
    log.shMem_printf(gStaticBuffer);
  } else {
    printf("%s: ERROR: can't print %s",__FUNCTION__,gStaticBuffer);        
  }
  
}

void v2cLogF(const char *fmt, ...) {
  char gStaticBuffer[2048];
  char gStaticBuffer2[2048];
  char hexV[3];
  // copy from Monte debugPrintf
  int lenny = 0;
  va_list  arglist;
  va_start(arglist,fmt);

  strcpy(gStaticBuffer,"C_FATAL: ");
  lenny = vsprintf (gStaticBuffer2, fmt, arglist );
  strcat(gStaticBuffer,gStaticBuffer2);

  shLog log;
  int slotId, cpuId;
  if (log.isAThread(&slotId,&cpuId)) {
    log.shMem_printf(gStaticBuffer);
  } else {
    printf("%s: ERROR: can't print %s",__FUNCTION__,gStaticBuffer);
  }
  va_end(arglist);
  
}

#endif


