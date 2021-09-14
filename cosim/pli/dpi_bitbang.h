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

#ifndef DPI_BITBANG_H
#define DPI_BITBANG_H

#include <stdint.h>
#include <sys/types.h>

extern "C" {
    int jtag_init(void) ;
    int jtag_quit(void);
    int jtag_getSocketPortId (void);
    int jtag_cmd(const int tdo_in, int *encode);
}

#endif
