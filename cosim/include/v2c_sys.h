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

#ifndef __V2C_SYS__
#define __V2C_SYS__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


/* ================================= */
/* For comunication stuffs           */
/* ================================= */
//#include "v2c_ipc.h"
//#include <string.h>
//#include <stdarg.h>

extern char bStr[128];
#define WAVE_START    0x1
#define WAVE_STOP     0x2

#define ON  0x1
#define OFF 0x0

#define PARTIAL_MC    0x10
#define FRONTDOOR_MC  0x20

// =================================
#ifndef DLL_SIM
#include "v2c_cmds.h"

#ifdef __cplusplus
extern "C" {
#endif

  void v2cLogI(const char *fmt, ...);
  void v2cLogE(const char *fmt, ...);
  void v2cLogF(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif

/* ================================= */
#endif
