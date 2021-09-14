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

#ifndef cep_token__H_
#define cep_token__H_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct token_ {
  char ascii[64];
  u_int64_t value;
  int isNum;
} cep_token_t;


#define CEP_MAX_HISTORY 32
typedef struct comd_ {
  char ascii[256];
  int cmdId;
} cep_comd_t;

//
// ============================
// Function prototypes
// ============================
#ifdef __cplusplus
extern "C" {
#endif
  // =============================
void cep_ProcessTokens(char *inLine, int save2history);
cep_token_t *get_token(int token_id);
void cep_set_token(int token_id, const char *name);
void cep_copy_token(int src, int dst);
int cep_get_token_count(void);
void cep_set_token_count(int value);
int cep_get_args_count(void);
int cep_print_history(void);
void cep_GetCommand(const char *prompt, int inDebugger);
void cep_GetPrompt(void);
u_int32_t cep_token_get_id(int token_id, int *isWildcard);
char *cep_get_curhistory_line(void);
void cep_brk_push_command(void);
void cep_brk_pop_command(void);
char *cep_get_inputLine_p(void);
int cep_get_first_args_pos(void);
void cep_set_first_args_pos(int pos);

  // =============================
#ifdef __cplusplus
}
#endif
// =============================
#endif
