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

#ifndef cep_vars__H_
#define cep_vars__H_

#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>

//
// Cable Type (bit mask)
//
#define INTERNAL_LOOPBACK_ENABLE (1 << 0)
#define EXTERNAL_LOOPBACK_ENABLE (1 << 1)
#define PARALLEL_LOOPBACK_ENABLE (1 << 2)
//
// from passing
//
#define SKIP_INIT_MASK          (1<<0)
#define STAND_ALONE_MASK        (1<<1)
#define HYPERION_PREPROTO3_MASK (1<<2)
#define FORCE_HYPERION_BOARD_ID (1<<3)
#define FORCE_DPC40x10_BOARD_ID (1<<4)
#define FORCE_DPC2x100_BOARD_ID (1<<5)
#define FORCE_VERBOSE           (1<<8)

//
// test filter
//
#define TEST_FILTER_DEF         0xFFFFFFFF
#define TEST_FILTER_MIC         (1<<16)

//
// to hold variable name and value
//
typedef int (*cep_exe_var_vect_t)(void);

typedef struct cep_var_s cep_var_info;

typedef struct cep_var_s {
  char name[64]; // max
  int isNum;
  int readOnly;
  int valid;
  u_int32_t value; // its value
  cep_exe_var_vect_t var_vect; // side effect
  char valStr[64]; // or string 
  char help[128];
  cep_var_info *next;
} cep_var_s_t;

//
// ==========================================
// These are for quick index referencing (avoid the search every time it is used)
// And the name MUST begin with VAR_<variableName>
//
// ==========================================
// should be sorted for display look up
//
typedef enum {
  VAR_CEP_MARKER=-1,
  // ======== BEGIN (must start at 0) ===========
  VAR_curErrCnt,
  VAR_coreMask,
  VAR_noCoreLock, // no core lock
  //
  VAR_lockSeed,                  
  VAR_loop,                      
  VAR_mloop,
  VAR_maxErr,                    
  VAR_prompt,                    
  VAR_quit,                      
  VAR_regress,                   
  VAR_seed,                      
  VAR_skipInit,
  VAR_testMask,
  VAR_verbose, 
  VAR_revCheck,
  VAR_longRun,
  VAR_step,
  VAR_stop_when_done,
  VAR_filter,
  VAR_detail,
  VAR_xterm,    
  // ======== ReadOnly =====================
  //
  // ReadOnly
  //
  VAR_lastDataRead,
  VAR_cur_loop,                      
  // =======================================
  VAR_zzMAX_CEP_COUNT       // end marker
} cep_var_enum;

//
//================================
// Some macros
//================================
//
#define SEARCH_VAR(v,m)     cep_search_var(v,m)

#define GET_VAR_NAME(v)     cep_get_var_name(VAR_ ## v)
#define GET_VAR_VALUE(v)    cep_get_var_value(VAR_ ## v)
#define GET_VAR_VALSTR(v)   cep_get_var_valStr(VAR_ ## v)

#define SET_VAR_VALUE(v,n)  cep_set_var_value(VAR_ ## v, n)
#define INC_VAR_VALUE(v,n)  cep_inc_var_value(VAR_ ## v, n)

#define RETURN_IF_MAXERR(e) do { if (e>= GET_VAR_VALUE(maxErr)) return e; } while (0)
//
// macros
//
#define CEP_ADD_VAR(name,num,v,f,h)  do {				\
    if (num == 1) {							\
      tmp_p = cep_var_add2idx(VAR_ ## name, #name,1,0,(cep_exe_var_vect_t)f,#v,h); \
    } else {								\
      tmp_p = cep_var_add2idx(VAR_ ## name, #name,0,0, (cep_exe_var_vect_t)f,(const char *)v,h); \
    }									\
    if (tmp_p == NULL) return 1;					\
  } while (0)

#define CEP_ROA_VAR(name,num,v,f,h)  do {				\
    if (num == 1) {							\
      tmp_p = cep_var_add2idx(VAR_ ## name, #name,1,1, (cep_exe_var_vect_t)f,#v,h); \
    } else {								\
      tmp_p = cep_var_add2idx(VAR_ ## name, #name,0,1, (cep_exe_var_vect_t)f,(const char *)v,h); \
    }									\
    if (tmp_p == NULL) return 1;					\
  } while (0)

//
// ============================
// Function prototypes
// ============================
#ifdef __cplusplus
extern "C" {
#endif
  // =============================
  int cep_init_vars(void);
  void cep_sort_vars(void);
  int cep_exe_setvar(void);
  int cep_exe_getvar(void);
  int cep_exe_adjvar(void);
  int cep_exe_monitor(void);
  void cep_set_run_revCheck_func(cep_exe_var_vect_t checkFunc);
  void cep_search_var_and_printAll(char *varName);
  cep_var_info *cep_search_var(char *varName, int *matchFound);
  cep_var_info *cep_var_add2idx(int idx, const char *name, int isNum, int readOnly, 
					cep_exe_var_vect_t var_vect,
					const char *strDefaultValue, const char *help);
  cep_var_info *cep_var_changeFuncVec(int idx, cep_exe_var_vect_t var_vect);

  int cep_var_modify(char *varName, char *newStrValue);
  void cep_var_print(cep_var_info *var_p);
  void cep_var_dump_all(int readOnly, int rdwr);
  cep_var_info *cep_get_var(int idx);
  char *cep_get_var_name(int idx);
  u_int32_t cep_get_var_value(int idx);
  void cep_set_var_value(int idx, u_int32_t value);
  void cep_inc_var_value(int idx, u_int32_t value);
  char *cep_get_var_valStr(int idx);
  int cep_get_max_var_count(void);
  void cep_set_max_var_count(int value);
  void cep_set_new_seed(void);
  int cep_exe_timeStamp(void);
  int cep_exe_timeStamp_mark(void);
  int cep_print_timeStamp(void);
  //
  void wait4prompt(void);
  int set_stty(void);  
  // =============================
#ifdef __cplusplus
}
#endif
// =============================
#endif

