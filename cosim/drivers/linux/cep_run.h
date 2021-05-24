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

#ifndef cep_run__H_
#define cep_run__H_

#include <stddef.h>
#include <stdlib.h>
#include <mutex>
#include <sys/types.h>

//
// function pointer point to API to execute the command
//
typedef int (*cep_exe_run_vect_t)(void);
//
// to hold command name and value
//
#define TEST_NOT_RUN  0
#define TEST_RUNNING  1
#define TEST_PASSED   2
#define TEST_FAILED   3
#define TEST_IGNORED  4
#define TEST_SKIPPED  5
#define TEST_FILTERED 6
//
// regression mode
//
#define SHORT_REGRESS      0x01
#define LONG_REGRESS       0x02
#define VERBOSE_REGRES     0x8000


typedef struct cep_run_s cep_run_info;
typedef struct cep_run_s {
  cep_run_info *next;
  int groupId;
  int id;
  int ignore; // skip it set
  int filter; // filter out
  int regressAble; // regress mode
  int status; // last run status
  int valid; // mark valid entry
  cep_exe_run_vect_t run_vect; // to run
  cep_exe_run_vect_t pre_run_vect; // to do needed setup before the test can be run
  cep_exe_run_vect_t post_run_vect; // for performance meter after test run
  char name[64]; // max
  char help[256]; // test description
} cep_run_s_t;
//
// macros
//
#define CEP_ADD_RUN(g,name,s,r,p,post,h)  do {			\
    tmp_p = cep_run_add2idx(g,RUN_ ## name, #name,s,r,p,post,h);	\
    if (tmp_p == NULL) return 1;					\
  } while (0)


#define CEP_DELETE_RUN(name)  do {					\
    cep_run_delete(RUN_ ## name);					\
  } while (0)

//
// multi-Thread stuffs
//
typedef void (*cep_thread_funct_t)(int);
//

//
// ==========================================
// These are for quick index referencing (avoid the search every time it is used)
// And the name MUST begin with RUN_<commandName>
//
// ==========================================
// should be sorted for better display look up
// 
typedef enum {
  RUN_CEP_MARKER=-1,
  // ======== BEGIN (must start at 0) ===========
  RUN_runAll       , 
  RUN_cepThrTest   , 
  RUN_cepRegTest   ,
  RUN_cepLockTest  ,
  RUN_cepMultiLock ,    
  // single thread
  RUN_cepGpioTest  ,
  RUN_cepSrotMemTest  ,
  RUN_cepSrotMaxKeyTest,
  RUN_cepMacroBadKey,
  RUN_smemTest     ,  
  RUN_cepLockfreeAtomic,
  RUN_cepLrscOps,
  RUN_cepClintTest,
  RUN_cepPlicTest,
  RUN_cepAccessTest,
  RUN_cepAtomicTest,
  RUN_cepSpiTest,
  RUN_cepMaskromTest,
  //  RUN_cepCsrTest,
  RUN_cepSrotErrTest,
//  RUN_cepPlicPrioIntrTest,  
  //
  RUN_cepMacroMix  , // multi-threaded
  RUN_cep_AES ,
  RUN_cep_DES3 ,
  RUN_cep_DFT ,
  RUN_cep_FIR ,
  RUN_cep_IIR ,
  RUN_cep_GPS ,
  RUN_cep_MD5 ,
  RUN_cep_RSA ,
  RUN_cep_SHA256 ,
  RUN_cepMultiThread,
  //
  RUN_ddr3Test     ,
  RUN_dcacheCoherency,
  RUN_icacheCoherency,
  RUN_cacheFlush,
  // =======================================
  RUN_zzMAX_CEP_COUNT       // end marker
} cep_run_enum;

//
//================================
// Some macros
//================================
//
#define SEARCH_RUN(n,m)        cep_search_run(n,m)
#define GET_RUN_NAME(n)        cep_get_run_name(RUN_ ## n)
#define GET_RUN_HELP(n)        cep_get_run_help(RUN_ ## n)

//
// ============================
// Function prototypes
// ============================
#ifdef __cplusplus
extern "C" {
#endif
  // =============================
  int cep_init_run(void);
  int cep_exe_run(void);
  int cep_exe_postRun(void);
  void cep_sort_run(void);
  //int cep_exe_dump(void);
  int cep_exe_report(void);
  int cep_exe_ignore(void);
  int cep_exe_filter(void);
  int cep_exe_menu(void);
  void cep_search_run_and_printAll(char *runName);
  cep_run_info *cep_search_run(char *runName, int *matchFound);
  cep_run_info *cep_run_add2idx(int groupId,
					int idx, const char *name, int regress,
					cep_exe_run_vect_t run_func_t, // cep_exe_run_vect_t dump_func_t, 
					cep_exe_run_vect_t pre_run_func_t,
					cep_exe_run_vect_t post_run_func_t,
					const char *help);
  void cep_run_delete(int idx);
  void cep_set_start_of_loop_func(cep_exe_run_vect_t checkFunc);

  void cep_run_print(cep_run_info *run_p);
  void cep_run_dump_all(void);
  cep_run_info *cep_get_run(int idx);
  char *cep_get_run_name(int idx);
  char *cep_get_run_help(int idx) ;
  int cep_get_max_run_count(void);
  void cep_set_max_run_count(int value);
  int cep_get_regress(int idx);  
  int cep_get_ignore(int idx);
  void cep_set_ignore(int idx, int value);
  int cep_get_filter(int idx);
  void cep_set_filter(int idx, int value);
  void cep_set_run_status(int idx, int value);
  int cep_get_run_status(int idx);  

  // allow one same test to do both dump and run
  void cep_set_dump_mode(int value);
  int cep_get_dump_mode(void);  
  int cep_testId2index(int testId);
  int cep_index2testId(int index);  

  int NORUN_IN_REGRESS(void);

  //
  // to support multi thread
  //
  void cep_set_thr_function(cep_thread_funct_t funct);
  cep_thread_funct_t cep_get_thr_function(void);
  void cep_set_thr_errCnt(int value);
    void cep_set_mthr_errCnt(int thrId,int value);    
  int cep_get_thr_errCnt(void);
  int run_multiThreads(int coreMask); // , cep_thread_funct_t funct);
  int run_multiThreadFloats(int num_threads);
  int thr_waitTilLock(int id, int maxTO);
  
  // =============================
#ifdef __cplusplus
}
#endif
// ===========================
#endif
