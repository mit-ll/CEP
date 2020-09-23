//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef CEP_MACRO_H
#define CEP_MACRO_H

#include <stddef.h>
#include <stdlib.h>

//
// Prototypes
//
#ifdef __cplusplus
extern "C" {
#endif
  int cep_process_macro(void);
  int cep_macro_inProgress(void);
  void cep_macro_addCmd(char *cmd);

#ifdef __cplusplus
}
#endif

#endif
