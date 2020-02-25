//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef CEP_SCRIPT_H
#define CEP_SCRIPT_H

#include <stddef.h>
#include <stdlib.h>

//
// Prototypes
//
#ifdef __cplusplus
extern "C" {
#endif
  void cep_file_read_func_init(void *func);
  void cep_script_cleanup_all(void);
  void cep_script_print_info(void);
  int cep_script_line_is_empty(char *str);

  int cep_execute_script(char *fileName);
  int cep_execute_macro(char *macroName, char **buf);
  int cep_script_inProgress(void);
  int cep_get_a_line_from_script(char *lineOut);
  int cep_get_lno(void);
  //
  int cep_read_file_into_buffer(char *fileName, char **fileBuf, int *fileSize, int maxSize);
#ifdef __cplusplus
}
#endif

#endif
