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

#ifdef _TESTONLY_
#define _MICROKERNEL
#endif

#ifndef _MICROKERNEL
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#endif


#ifndef _MICROKERNEL
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <time.h>
#endif


#include "cep_io.h"

void _mdelay(int msec) {
#ifndef _MICROKERNEL
  // millisecdelay
  struct timespec rqtp, rmtp;

  rqtp.tv_sec  = msec / 1000000;
  rqtp.tv_nsec =  (msec % 1000000) * 1000;
                                                                                
  (void) nanosleep(&rqtp, &rmtp);
#endif
}
void _udelay(int usec) {
#ifndef _MICROKERNEL
  // usec delay
  struct timespec rqtp, rmtp;
  
  rqtp.tv_sec = 0;
  rqtp.tv_nsec = usec * 1000;
                                                                                
  (void) nanosleep(&rqtp, &rmtp);
#endif
}
//
// ====================
//
// ====================
//
int gWait4Prompt = 0;

void Prompt(void) {
#ifndef _MICROKERNEL
  if (gWait4Prompt) {
    char junk[256];
    printf("\n Press Any key to continue ");
    fgets(junk,256, stdin);
    fgets(junk,256, stdin);
    //fgetc(stdin);
  }
#endif
}
