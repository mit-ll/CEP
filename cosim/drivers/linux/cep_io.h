//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
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
#define THR_LOGI(format, ...) {                 \
        std::scoped_lock iolock(iomutex);       \
        printf("C%d: ",sched_getcpu());                 \
        printf(format,  ##__VA_ARGS__);fflush(NULL);    \
    } 

#define THR_LOGE(format, ...) {                                  \
        std::scoped_lock iolock(iomutex);                        \
        printf("C%d: ** ERROR ** ",sched_getcpu());              \
        printf(format, ##__VA_ARGS__); fflush(NULL);             \
    }

#define THR_LOGW(format, ...) {                                  \
        std::scoped_lock iolock(iomutex);                        \
        printf("C%d: ** WARNING ** ",sched_getcpu());            \
        printf(format, ##__VA_ARGS__);fflush(NULL);              \
    }

#define THR_LOGF(format, ...) {                                  \
        std::scoped_lock iolock(iomutex);                        \
        printf(_tstr,"C%d: ** FATAL ** ",sched_getcpu());        \
        printf(format, ##__VA_ARGS__); fflush(NULL);             \
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
