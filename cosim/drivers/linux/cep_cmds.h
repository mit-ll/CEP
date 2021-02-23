//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef cep_cmds__H_
#define cep_cmds__H_

#include <stddef.h>
#include <stdlib.h>
//#include <sys/types.h>


//
// function pointer point to API to execute the command
//
typedef int (*cep_exe_cmd_vect_t)(void);
//
// to hold command name and value
//
typedef struct cep_cmd_s cep_cmd_info;
typedef struct cep_cmd_s {
  char name[64]; // max
  int args_count; // min argument count
  int valid;
  cep_exe_cmd_vect_t exe_vect; // to run
  char exe_vect_name[64];
  char help[256]; // to print out how the command look like
  cep_cmd_info *next;
} cep_cmd_s_t;

//
// macros
//
#define CEP_ADD_CMD(name,args,f,h)  do {				\
    tmp_p = cep_cmd_add2idx(CMD_ ## name, #name,args,f,#f,h);	\
    if (tmp_p == NULL) return 1;					\
  } while (0)




//
// ==========================================
// These are for quick index referencing (avoid the search every time it is used)
// And the name MUST begin with CMD_<commandName>
//
// ==========================================
// should be sorted for better display look up
// 
typedef enum {
  CMD_CEP_MARKER=-1,
  // ======== BEGIN (must start at 0) ===========
  CMD_delay       ,
  CMD_sleep       ,
  CMD_device      ,
  CMD_dump        ,
  CMD_ignore      ,
  CMD_help        ,
  CMD_menu        ,
  CMD_read        ,
  CMD_compare     ,
  CMD_regTest     ,
  CMD_report      ,
  CMD_run         ,
  CMD_write       ,
  CMD_macro       ,
  CMD_history     ,
  CMD_execute     ,
  CMD_setvar      ,
  CMD_getvar      ,
  CMD_adjvar      ,
  CMD_memWrite    ,
  CMD_memRead     ,
  CMD_memSearch   ,
  CMD_system      ,  
  // =======================================
  CMD_zzMAX_CEP_COUNT       // end marker
} cep_cmd_enum;


//
//================================
// Some macros
//================================
//
#define SEARCH_CMD(n,m)        cep_search_cmd(n,m)
#define GET_CMD_NAME(n)        cep_get_cmd_name(CMD_ ## n)
#define GET_CMD_ARGS_COUNT(n)  cep_get_cmd_args_count(CMD_ ## n)
#define GET_CMD_HELP(n)        cep_get_cmd_help(CMD_ ## n)

//
// ============================
// Function prototypes
// ============================
#ifdef __cplusplus
extern "C" {
#endif
  // =============================
  int cep_init_cmds(void);
  void cep_sort_cmds(void);
  void cep_search_cmd_and_printAll(char *cmdName);
  cep_cmd_info *cep_search_cmd(char *cmdName, int *matchFound);
  cep_cmd_info *cep_cmd_add2idx(int idx, const char *name, int args_count, cep_exe_cmd_vect_t func_t, 
					const char *funcName, const char *help);
  void cep_cmd_print(cep_cmd_info *cmd_p);
  void cep_cmd_dump_all(void);
  cep_cmd_info *cep_get_cmd(int idx);
  char *cep_get_cmd_name(int idx);
  char *cep_get_cmd_help(int idx) ;
  int cep_get_cmd_args_count(int idx) ;
  int cep_cmd_process_and_execute(void);
  int cep_get_max_var_count(void);
  void cep_set_max_var_count(int value);
  int cep_get_max_cmd_count(void);
  void cep_set_max_cmd_count(int value);
  int cep_cmd_reboot(void);

  // syslog
  int set_syslog(int enable);

  // =============================
#ifdef __cplusplus
}
#endif
// ===========================
#endif
