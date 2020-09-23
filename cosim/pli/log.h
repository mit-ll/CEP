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

#ifndef __LOG_H
#define __LOG_H


//

#include "veriuser.h"
//#include "vpp_ipc_pli.h"
#include "shIpc.h"


#define DEBUG  debug_mode

  /*
    #define LOGD   if (DEBUG) my_io_printf("\tMDBUG: %8d %s ",tf_gettime(),tf_mipname());if (DEBUG) my_io_printf
    #define LOGW   my_io_printf("MWARN: %8d %s ",tf_gettime(),tf_mipname());my_io_printf
    #define LOGI   my_io_printf("MINFO: %8d %s ",tf_gettime(),tf_mipname());my_io_printf
    #define LOGE   inc_error();my_io_printf("MERROR: %8d %s ",tf_gettime(),tf_mipname());my_io_printf
    #define LOGF   inc_error();my_io_printf("MFATAL: %8d %s ",tf_gettime(),tf_mipname());my_io_printf
  */

extern "C" {
  void vpp_debug_mode();
  int  vpp_debug();
  
  int inc_error();
  void vpp_getErrorCount();
  void vpp_setErrorCount();
  
  void getspname_ext ();
  
  int logD ();
  int logI ();
  int logW ();
  int logE ();
  int logF ();
  
  void my_printLog ( int logType, int curParam );
  void my_io_printf(const char *fmt, ...);
  void my_io_logI(const char *fmt, ...);
  void my_io_logD(const char *fmt, ...);
  void my_io_logW(const char *fmt, ...);
  void my_io_logE(const char *fmt, ...);
  void my_io_logF(const char *fmt, ...);
  int FoundAThread(MY_THREAD_TYPE tid, int *semaId);
  int print_nameNtime(MY_THREAD_TYPE tid, const char *prefixStr, char *subfixStr);
}

extern int error_count;
extern int *errorTS;

#define META_PERCENT '\377'

#define LOG_TYPE_DEBUG	  	1
#define LOG_TYPE_INFO	  	2
#define LOG_TYPE_WARNING  	3
#define LOG_TYPE_ERROR		4
#define LOG_TYPE_FATAL		5

#define LOG_ACTION_NONE		0x0
#define LOG_ACTION_STOP		0x1
#define LOG_ACTION_FINISH	0x2
#define LOG_ACTION_TF_WARNING	0x4
#define LOG_ACTION_TF_ERROR	0x8
#define LOG_ACTION_STOP_FINISH	0x3

typedef unsigned int uInt;

typedef struct LogDescription {
  const char	*header;	// String to start message with
  uInt	printMask;	// Mask used to see if print
  int		timeEnabled;	// Print time if true
  int		instEnabled;	// Print instance name if true
  int		maxCalls;	// Maximum number of calls before exit
  int		numCalls;	// Current number of calls
  int		msgAction;	// 0=none, 1=stop, 2=finish
} logDescription;


extern logDescription logTable[];

#endif
