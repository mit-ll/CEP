//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#include <stdlib.h>
#include <string.h>


#include "simdiag_global.h"
//#include "supercon_io.h"

static int verbosity = 0x1F;
static int __standalone = 0;
static int __revisionCheck = 1;
int __board_type = 0;
int __board_subtype = 0;

int simdiag_GetVerbose(void) { return verbosity; }

void simdiag_SetVerbose(int verbose) {
  verbosity = verbose;
}

int simdiag_GetStandalone(void) { return __standalone; }

void simdiag_SetStandalone(int standalone) {
  __standalone = standalone;
}

void simdiag_revCheck_set(int enable)
{
  __revisionCheck = enable;
}
int simdiag_revCheck_get(void)
{ 
  return __revisionCheck;
}

void set_board_type(int type)
{
  __board_type = type;
  //i2c_table_init();
}
int get_board_type(void)
{
  return __board_type;
}

void set_board_subtype(int type)
{
  __board_subtype = type;
  //i2c_table_init();
}
int get_board_subtype(void)
{
  return __board_subtype;
}

#if 0
void DELAY_N_Y(int msec) {
  //
  int i  = 0;;
  for (i=0;i<msec;i++) {
    time_delay_us(1000); // usec  
    thread_timed_yield();  
  }
}
#endif
// --------------------

