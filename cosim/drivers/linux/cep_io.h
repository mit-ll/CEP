//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef CEP_IO_H
#define CEP_IO_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "portable_io.h"
#include "cep_run.h"
//#include <sys/types.h>

extern std::mutex iomutex;
#define THR_LOGI(format, ...) {		 \
    char _gstr[1024], _tstr[32];			 \
    sprintf(_tstr,"C%d:",sched_getcpu());	 \
    snprintf(_gstr,sizeof(_gstr),format, ##__VA_ARGS__);	\
    std::lock_guard<std::mutex> iolock(iomutex); \
    printf("%s %s", _tstr, _gstr);		 \
  }

#define THR_LOGE(format, ...) {		 \
    char _gstr[1024], _tstr[32];			 \
    sprintf(_tstr,"C%d: ** ERROR ** ",sched_getcpu());	 \
    snprintf(_gstr,sizeof(_gstr),format, ##__VA_ARGS__);	 \
    std::lock_guard<std::mutex> iolock(iomutex); \
    printf("%s %s", _tstr, _gstr);		 \
  }

#define THR_LOGW(format, ...) {		 \
    char _gstr[1024], _tstr[32];			 \
    sprintf(_tstr,"C%d: ** WARNING ** ",sched_getcpu());	 \
    snprintf(_gstr,sizeof(_gstr),format, ##__VA_ARGS__);	 \
    std::lock_guard<std::mutex> iolock(iomutex); \
    printf("%s %s", _tstr, _gstr);		 \
  }

#define THR_LOGF(format, ...) {		 \
    char _gstr[1024], _tstr[32];			 \
    sprintf(_tstr,"C%d: ** FATAL ** ",sched_getcpu());	 \
    snprintf(_gstr,sizeof(_gstr),format, ##__VA_ARGS__);	 \
    std::lock_guard<std::mutex> iolock(iomutex); \
    printf("%s %s", _tstr, _gstr);		 \
  }


//#define DELAY(a)        mdelay(a)
#define UDELAY(a)       _udelay(a)
#define LOGI            THR_LOGI
#define LOGE            THR_LOGE

#define MIN(a,b)        ((a<b)?a:b)

#define MAX_ZERO_FILL   20

//
// ============================
// Function prototypes
// ============================
#ifdef __cplusplus
extern "C" {
#endif
  void _mdelay(int msec);
  void _udelay(int usec);
  void Prompt(void);

#ifdef __cplusplus
}
#endif


// ==============================
#endif
