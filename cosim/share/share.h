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

#ifndef SHARE_H
#define SHARE_H
// ---------------------------

//extern "C" {
#ifdef SIM_ENV_ONLY

#ifdef DLL_SIM
#include "veriuser.h"
#include "vpi_user.h"
#include "acc_user.h"
#include "log.h"
#else
#include "v2c_sys.h"
#endif
#endif



#endif
