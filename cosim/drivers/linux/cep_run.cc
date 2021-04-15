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

//
// ======================================
// handle run, dump, menu command
// ======================================
//
#include <iostream>
#include <mutex>
#include <thread>
#include <pthread.h>
#include <vector>

#include <string.h>
#include "cep_adrMap.h"

#include "cep_io.h"
#include "cep_cmds.h"
#include "cep_vars.h"
#include "cep_token.h"
#include "cep_run.h"
#include "cep_exports.h"
#include <unistd.h>


// 
// usefull for printing out temperature at start of the loop for chmber testing
//
cep_exe_run_vect_t start_of_loop_check = NULL;
void cep_set_start_of_loop_func(cep_exe_run_vect_t checkFunc) {
  start_of_loop_check = checkFunc;
}
//
// global array to hold run-able name
//
#define __CEP_RUN_MAX  256 // for now
int __cep_run_curCount = 0; // total available tests (some might not valid)
int __cep_validTestCount = 0; // actual valid test count
cep_run_info __cep_run_array[__CEP_RUN_MAX];
int __cep_run_testid2index[__CEP_RUN_MAX]; // array[testId] = index to run_array
// for sorted list
cep_run_info *__cep_run_head_p = NULL;

static int __cep_runAll_flag = 0;
// do not run this test if it is in regression since it is kind of special (suicide test)
int NORUN_IN_REGRESS(void) {
  return __cep_runAll_flag;
}

void cep_search_run_and_printAll(char *runName)
{
  int i=0;
  int l0;
  l0 = strlen(runName);
  PRNT("Possible matching RUN(s)\n");
  while (i < __cep_run_curCount) {
    if (strncmp(runName, __cep_run_array[i].name, 
		//MIN(l0,strlen(__cep_run_array[i].name))) == 0) { // found it
		l0) == 0) {
      cep_run_print(&__cep_run_array[i]);
    }
    i++;
  }
}

//
// ==========================
// return NULL if not found
// ==========================
//
cep_run_info *cep_search_run(char *runName, int *matchFound)
{
  int i=0;
  int saveP=0;
  int l0;
  //
  *matchFound = 0;
  // search entire thing to get number of possible matches
  l0 = strlen(runName);
  while (i < __cep_run_curCount) {
    if (strncmp(runName, __cep_run_array[i].name, 
		//MIN(l0,strlen(__cep_run_array[i].name))) == 0) { // found it
		l0) == 0) {
      saveP = i;
      *matchFound = *matchFound + 1;
    }
    i++;
  }
  if (*matchFound == 1) { // exactly 1
    return &__cep_run_array[saveP];
  } else if (*matchFound > 1) {
    if (GET_VAR_VALUE(verbose)) {
      PRNT("Warning: There are %d matches found for test %s\n",*matchFound,runName);
    }
    return &__cep_run_array[saveP];
  } else {
    //PRNT("Warning: Could not find command %s\n",runName);
    return NULL;
  }
}

//
// ====================================
// to add a test to array
// ====================================
//
cep_run_info *cep_run_add2idx(int groupId, int idx, const char *name, int regress,
				      cep_exe_run_vect_t run_func_t, //cep_exe_run_vect_t dump_func_t,
 				      cep_exe_run_vect_t pre_run_func_t,
 				      cep_exe_run_vect_t post_run_func_t,
				      const char *help)
{
  cep_run_info *tmp_p = NULL;
#ifdef DEBUG
  PRNT("%s: i=%d name=%s\n",
       __FUNCTION__,idx,name);
#endif
  if (idx >= __CEP_RUN_MAX) {
    LOGE("%s: ERROR Can't add any more test. idx=%d exceeds its limit of %d\n",
	 __FUNCTION__,idx,__CEP_RUN_MAX);
    return tmp_p;
  }
  // add to the current one and move to next
  strcpy(__cep_run_array[idx].name, name);
  strcpy(__cep_run_array[idx].help, help);
  __cep_run_array[idx].groupId       = groupId;
  __cep_run_array[idx].id            = idx;
  __cep_run_array[idx].ignore        = 0; // clear
  __cep_run_array[idx].filter        = 0; // clear
  __cep_run_array[idx].regressAble   = regress;
  __cep_run_array[idx].status        = TEST_NOT_RUN;
  __cep_run_array[idx].valid         = 1;
  __cep_run_array[idx].run_vect      = run_func_t;
  __cep_run_array[idx].pre_run_vect  = pre_run_func_t;
  __cep_run_array[idx].post_run_vect = post_run_func_t;

  // next
  tmp_p = &__cep_run_array[idx];
  //
  return tmp_p;
}
void cep_run_delete(int idx) {
  strcpy(__cep_run_array[idx].name, "UNKNOWN");		
  __cep_run_array[idx].valid = 0;
}

void cep_run_print(cep_run_info *run_p)
{
  char tmp[64];
  int i;
  if (run_p == NULL) {
    PRNT("%s; ERROR NULL run_p\n",__FUNCTION__);
    return;
  }
  if (run_p->valid) {
    strcpy(tmp,run_p->name);
    for (i=strlen(tmp);i<MAX_ZERO_FILL;i++) {
      strcat(tmp," ");
    }
    tmp[MAX_ZERO_FILL] = '\0';
    PRNT("%s Args: %s\n",tmp,run_p->help);
  }
}
void cep_run_dump_all(void) 
{
  int i;
  for (i=0;i<__cep_run_curCount;i++) {
    cep_run_print(&__cep_run_array[i]);
  }
}

//
// ===============================
// be carefull: no check!!!
// ===============================
//
cep_run_info *cep_get_run(int idx)
{
  return &__cep_run_array[idx];
}
char *cep_get_run_name(int idx) 
{
  return (cep_get_run(idx))->name;
}
char *cep_get_run_help(int idx) 
{
  return (cep_get_run(idx))->help;
}
int cep_get_max_run_count(void)
{
  return __cep_run_curCount;
}
void cep_set_max_run_count(int value)
{
  __cep_run_curCount = value;
}
int cep_get_ignore(int idx)
{
  return (cep_get_run(idx))->ignore;
}
void cep_set_ignore(int idx, int value)
{
  (cep_get_run(idx))->ignore = value;
}
int cep_get_filter(int idx)
{
  return (cep_get_run(idx))->filter;
}
void cep_set_filter(int idx, int value)
{
  (cep_get_run(idx))->filter = value;
}

static int cep_ok2run(int idx)
{
  // use board type as mask to qualify with regress mask from test
  return (GET_VAR_VALUE(regress) & cep_get_regress(idx)) ? 1 : 0;
}
int cep_get_regress(int idx)
{
  return (cep_get_run(idx))->regressAble;
}
int cep_get_run_status(int idx)
{
  return (cep_get_run(idx))->status;
}
void cep_set_run_status(int idx, int value)
{
  (cep_get_run(idx))->status = value;
}

//
// ======================================
// Tests
// ======================================
//
static void cep_clean_run_status(void) {
  int i;
  for (i=0;i<cep_get_max_run_count();i++) {
    cep_get_run(i)->status = TEST_NOT_RUN;
  }
}
static int cep_run_test(int startTestId, int endTestId)
{
  int errCnt = 0;
  int curTestId, l;
  int curIndex;
  //
  // Call and Execute the test
  //
  SET_VAR_VALUE(curErrCnt, 0); // clear error
  for (l=1;l<=(int)GET_VAR_VALUE(loop);l++) {
    SET_VAR_VALUE(cur_loop, l);
    //if (GET_VAR_VALUE(verbose) || GET_VAR_VALUE(regress)) {
    if (GET_VAR_VALUE(loop) > 1) {
      PRNT("Loop:%d (out of %d) startTestId=%d endTestId=%d\n",l,GET_VAR_VALUE(loop),startTestId,endTestId);
    }
    // do we need to do something at the start of the loop?
    if (GET_VAR_VALUE(regress) && (start_of_loop_check != NULL)) {
      (void)(*start_of_loop_check)();
    }
    // clean all test status
    cep_clean_run_status();
    for (curTestId=startTestId;curTestId<=endTestId;curTestId++) {
      curIndex = cep_testId2index(curTestId);
      if (cep_get_run(curIndex)->valid) { // only if valid
	if (cep_get_run(curIndex)->run_vect == NULL) {
	  PRNT("%s: Sorry, test %d (%s) does not have a run function defined or not enable..Skipping\n",__FUNCTION__,
	       curTestId,cep_get_run_name(curIndex));
	} else {
	  // should I run it? it might be ignore!!
	  if (!cep_get_ignore(curIndex) &&
	      !cep_get_filter(curIndex) &&
	      (!GET_VAR_VALUE(regress) || // interactive
	       (GET_VAR_VALUE(regress) && cep_ok2run(curIndex)))) { // regress but only if ok to run
	    cep_set_run_status(curIndex, TEST_NOT_RUN);
	    // get new seed if not lock
	    cep_set_new_seed();
	    // pre_run
	    errCnt = 0;
	    if (cep_get_run(curIndex)->pre_run_vect != NULL) {
	      errCnt = (*cep_get_run(curIndex)->pre_run_vect)();
	    }
	    if (errCnt) {
	      cep_set_run_status(curIndex, TEST_SKIPPED);
	      // print pass/fail
	      PRNT("%s: testId=%3d errCnt=%2d seed=0x%08x (%s)\n",
		   "   _CHKSKIP_",
		   curTestId, errCnt, GET_VAR_VALUE(seed), cep_get_run_name(curIndex));
	    }
	    //
	    else {
	      cep_set_run_status(curIndex, TEST_RUNNING);
	      errCnt = (*cep_get_run(curIndex)->run_vect)();
	      // print pass/fail
	      PRNT("%s: testId=%3d errCnt=%2d seed=0x%08x : %s\n",
		   (errCnt == -1) ? "   ==SKIPPED" : ((errCnt) ? "** TEST FAIL" : "   TEST PASS"),
		   curTestId, errCnt, GET_VAR_VALUE(seed), cep_get_run_name(curIndex));
	      if (errCnt == -1) { 
		cep_set_run_status(curIndex, TEST_SKIPPED);		
		errCnt = 0; } //  adjust due to skip
	      // keep score
	      if (errCnt) {
		cep_set_run_status(curIndex, TEST_FAILED);
		INC_VAR_VALUE(curErrCnt,1);
		//LOGE("ERROR: curErrCnt=%d\n",GET_VAR_VALUE(curErrCnt));
	      } else {
		cep_set_run_status(curIndex, TEST_PASSED);
	      }
	      // should I stop?
	      if (GET_VAR_VALUE(curErrCnt) >= GET_VAR_VALUE(maxErr)) { // stop
		LOGE("ERROR: curErrCnt=%d >= maxErr=%d..Stopping\n",GET_VAR_VALUE(curErrCnt),GET_VAR_VALUE(maxErr));
		return 1;
	      }
	    }
	  } // ok to run
	  else { //
	    cep_set_run_status(curIndex, 
				   cep_get_filter(curIndex) ? TEST_FILTERED : 
				   cep_get_ignore(curIndex) ? TEST_IGNORED : 
				   TEST_SKIPPED);
	    if (!cep_get_filter(curIndex)) {
	      PRNT("%s: testId=%3d errCnt=%2d seed=0x%08x (%s)\n",
		   (cep_get_ignore(curIndex)) ? "   _IGNORED_" : "   ___SKIP__",
		   curTestId, errCnt, GET_VAR_VALUE(seed), cep_get_run_name(curIndex));
	    }
	  }
	} // test 2 run
      } // only if valid
      else {
	PRNT("%s: Sorry, test %d (%s) is NOT valid..Skipping\n",__FUNCTION__,
	     curTestId,cep_get_run_name(curIndex));
      }
    } // for curTestId
  } // for loop
  return GET_VAR_VALUE(curErrCnt);
}

static int cep_runAll(void)
{
  int errCnt = 0;
  int startTestId;
  int endTestId;
  if (GET_VAR_VALUE(verbose)) {
    PRNT("%s:\n",__FUNCTION__);
  }
  cep_set_run_status(RUN_runAll, TEST_RUNNING);
  startTestId = 1;
  endTestId   = __cep_validTestCount-1; // cep_get_max_run_count()-1;
  errCnt      = cep_run_test(startTestId, endTestId);
  if (errCnt) cep_set_run_status(RUN_runAll, TEST_FAILED);
  else        cep_set_run_status(RUN_runAll, TEST_PASSED);
  return errCnt;
}
//
// ===================================
// Execute the run command here
// ===================================
//
int cep_exe_run(void)
{
  int errCnt = 0;
  int startTestId, endTestId;
  int matchFound;
  cep_run_info *tmp_p;

  if (GET_VAR_VALUE(verbose)) {
    PRNT("%s: cep_get_args_count=%d\n",__FUNCTION__,cep_get_args_count());
  }
  //
  // Arg1 = startTestId
  //
  if (get_token(1)->isNum) {
    startTestId = get_token(1)->value;
    // check if the test is valid
    if (startTestId > cep_get_max_run_count()) {
      LOGE("%s: ERROR: Can't find test %s in the list\n",__FUNCTION__,
	   get_token(1)->ascii);
      return 1;
    }
  }
  else { // might the name of the test
    // search for testName
    tmp_p = cep_search_run(get_token(1)->ascii, &matchFound);
    if (matchFound == 1) { // exact 1
      startTestId = tmp_p->id;
    } else {
      LOGE("%s: ERROR: Can't find test %s in the list\n",__FUNCTION__,
	   get_token(1)->ascii);
      return 1;
    }
  }
  //
  // Arg2 = may be = endTestId
  //
  if (cep_get_args_count() > 1) { // there is a range
    // get second arg as endtestId
    if (get_token(2)->isNum) {
      endTestId = get_token(2)->value;
    } else { // might be the name of the test
      // search for testName
      tmp_p = cep_search_run(get_token(2)->ascii, &matchFound);
      if (matchFound == 1) { // exact 1
	endTestId = tmp_p->id;
      } else {
	LOGE("%s: ERROR: Can't find test %s in the list\n",__FUNCTION__,
	     get_token(2)->ascii);
	return 1;
      }
    }
  } else { // single
    endTestId = startTestId;
  }
  // adjust
  if (endTestId >= cep_get_max_run_count()) {
    endTestId = cep_get_max_run_count() - 1;
  }
  // run them
  if (GET_VAR_VALUE(verbose)) {
    PRNT("%s: startTestId=%d endTestId=%d\n",
	 __FUNCTION__,startTestId, endTestId);
  }
  //
  // SPECIAL
  //
  if (startTestId == RUN_runAll) {
    __cep_runAll_flag = 1;
    errCnt = cep_runAll();
  } else {
    __cep_runAll_flag = 0;
    if (startTestId < cep_get_max_run_count()) {
      errCnt = cep_run_test(startTestId, endTestId);
    } else {
      PRNT("%s: test=%d is not on the list. Type \"menu\" for available test list\n",
	   __FUNCTION__,startTestId);
    }
  }
  return errCnt;
}

//
// ===================================
// Execute the "post" run command here (for metering)
// ===================================
//
#ifdef _MICROKERNEL
extern void sleep_seconds(int numSec);
#else
#define sleep_seconds(x) sleep(x)
#endif

int cep_exe_postRun(void)
{
  int errCnt = 0;

  if (GET_VAR_VALUE(verbose)) {
    PRNT("%s: cep_get_args_count=%d\n",__FUNCTION__,cep_get_args_count());
  }
  PRNT("FIXME!!!\n");
  return errCnt;
}

//
// ===================================
// ignore a test
// ===================================
//

int cep_exe_ignore(void)
{
  int errCnt = 0;
  int test2ignore;
  int index,i;
  int igCnt = 0;
  cep_run_info *cur_p = __cep_run_head_p;
  //
  // Arg1 = startTestId to ignore
  //
  if (cep_get_args_count() > 0) {
    for (i=1;i<=cep_get_args_count();i++) {
      test2ignore = (int)get_token(i)->value;
      if (test2ignore < 0) { // un-ignore the test
	test2ignore = test2ignore * -1;
	index = cep_testId2index(test2ignore);
	cep_set_ignore(index,0);
      } else {
	index = cep_testId2index(test2ignore);
	cep_set_ignore(index,1);
      }
      /*
	PRNT("Setting test %d (%s) ignore flag to %d\n",
	test2ignore,cep_get_run_name(index),cep_get_ignore(index));
      */
    }
  }
  // dump the list
  while (cur_p != NULL) {
    if (cur_p->ignore) {
      PRNT(" \t%4d : %s\n",cur_p->id,cur_p->name);
      igCnt++;
    }
    //
    cur_p = cur_p->next;
  }
  PRNT("  Found %d test(s) to ignore\n",igCnt);
  return errCnt;
}


//
// ================================================================
// Add test from here
// ================================================================

//
// ====================================
// return 1 if error
// ====================================
int cep_init_run(void)
{
  int errCnt = 0;
  int i;
  cep_run_info *tmp_p = NULL;
#ifdef DEBUG
  PRNT("%s:\n",__FUNCTION__);
#endif
  // init to all UNKNOWN
  for (i=0;i<__CEP_RUN_MAX;i++) {
#if 1
    memset(&(__cep_run_array[i]), 0x0, sizeof(cep_run_s_t));
    tmp_p = cep_run_add2idx(99,i, "UNKNOWN",0,NULL,NULL,NULL,"-- not supported --");
    __cep_run_array[i].valid = 0;
#else
    tmp_p = cep_run_add2idx(99,i, "UNKNOWN",0,NULL,NULL,NULL,"-- not supported --");
    // mark all as invalid
    __cep_run_array[i].valid = 0;
    __cep_run_array[i].run_vect = NULL;
    __cep_run_array[i].pre_run_vect = NULL;
    __cep_run_array[i].post_run_vect = NULL;
#endif
  }
  //
  // ============================================
  // This is where you add global test to CEP
  // ============================================
  //  
  //            Name                regress      run_func              preRun post  Test Description
  CEP_ADD_RUN(0,runAll,             0xFFFFFFFF,  cep_runAll,           NULL , NULL, "Run all available tests");
  //
  // RISC-V related
  //
  CEP_ADD_RUN(1,cepThrTest,         0xFFFFFFFF,  run_threadTest,       NULL,  NULL, "Multi-thread tests (all cores)");    
  CEP_ADD_RUN(1,dcacheCoherency,    0xFFFFFFFF,  run_dcacheCoherency,  NULL,  NULL, "D-cache coherency Test (all cores)");
  CEP_ADD_RUN(1,icacheCoherency,    0xFFFFFFFF,  run_icacheCoherency,  NULL,  NULL, "I-cache coherency Test (all cores)");
  CEP_ADD_RUN(1,cacheFlush,         0xFFFFFFFF,  run_cacheFlush,       NULL,  NULL, "I+D-caches flush all cores (via self-mod-code)");    
  //
  // Memory related
  //
  CEP_ADD_RUN(2,cepSrotMemTest,     0xFFFFFFFF,  run_cepSrotMemTest,   NULL,  NULL, "CEP SRoT memory test (single core) ");
  CEP_ADD_RUN(2,ddr3Test,           0xFFFFFFFF,  run_ddr3Test,         NULL,  NULL, "Main Memory Test (all cores)");
  CEP_ADD_RUN(2,smemTest,           0xFFFFFFFF,  run_smemTest,         NULL,  NULL, "Scratchpad Memory Test (all cores)");
  CEP_ADD_RUN(2,cepMaskromTest,     0xFFFFFFFF,  run_cepMaskromTest,   NULL,  NULL, "CEP Maskrom Read-Only Test (all cores)");
  //
  // CEP misc. tests
  //
  CEP_ADD_RUN(3,cepGpioTest,        0xFFFFFFFF,  run_cepGpioTest,      NULL,  NULL, "CEP GPIO test (single core) ");
  CEP_ADD_RUN(3,cepSpiTest,         0xFFFFFFFF,  run_cepSpiTest,       NULL,  NULL, "CEP SPI test (single core) ");
  CEP_ADD_RUN(3,cepSrotErrTest,     0xFFFFFFFF,  run_cepSrotErrTest,   NULL,  NULL, "CEP SRoT Error Test (single core) ");
//CEP_ADD_RUN(3,cepCsrTest,         0xFFFFFFFF,  run_cepCsrTest,       NULL,  NULL, "CEP SPI test (all cores) ");

  CEP_ADD_RUN(4,cepPlicTest,        0xFFFFFFFF,  run_cepPlicTest,      NULL,  NULL, "CEP PLIC register test (all cores)");    
  CEP_ADD_RUN(4,cepClintTest,       0xFFFFFFFF,  run_cepClintTest,     NULL,  NULL, "CEP CLINT register test (all cores)");    
  CEP_ADD_RUN(4,cepRegTest,         0xFFFFFFFF,  run_cepRegTest,       NULL,  NULL, "CEP register tests on all cores");
  CEP_ADD_RUN(4,cepLockTest,        0xFFFFFFFF,  run_cepLockTest,      NULL,  NULL, "CEP single lock test (all cores)");
  CEP_ADD_RUN(4,cepMultiLock,       0xFFFFFFFF,  run_cepMultiLock,     NULL,  NULL, "CEP multi-lock test (all cores)");    
  CEP_ADD_RUN(4,cepLockfreeAtomic,  0xFFFFFFFF,  run_cepLockfreeAtomic,NULL,  NULL, "CEP lock-free instructions test (all cores) ");    
  CEP_ADD_RUN(4,cepLrscOps,         0xFFFFFFFF,  run_cepLrscOps,       NULL,  NULL, "CEP Load-Reserve/Store-Conditional test (all cores)");    
  CEP_ADD_RUN(4,cepAccessTest,      0xFFFFFFFF,  run_cepAccessTest,    NULL,  NULL, "CEP various bus/size access test (all cores)");    
  CEP_ADD_RUN(4,cepAtomicTest,      0xFFFFFFFF,  run_cepAtomicTest,    NULL,  NULL, "CEP atomic intructions test (all cores)");    
  //
  // CEP macro related 
  //
  CEP_ADD_RUN(5,cepMacroMix,        0xFFFFFFFF,  run_cepMacroMix,      NULL,  NULL, "CEP Macro tests (all cores)");
  CEP_ADD_RUN(5,cepMacroBadKey,     0xFFFFFFFF,  run_cepMacroBadKey,   NULL,  NULL, "CEP Macro tests with badKey (all cores)");
  //
  CEP_ADD_RUN(6,cepSrotMaxKeyTest,  0xFFFFFFFF,  run_cepMacroBadKey,   NULL,  NULL, "CEP Macro tests with maxKey (single core)");
  CEP_ADD_RUN(6,cep_AES     ,       0xFFFFFFFF,  run_cep_AES     ,     NULL,  NULL, "CEP AES test (single core)");  
  CEP_ADD_RUN(6,cep_DES3    ,       0xFFFFFFFF,  run_cep_DES3    ,     NULL,  NULL, "CEP DES3 test (single core)");  
  CEP_ADD_RUN(6,cep_DFT     ,       0xFFFFFFFF,  run_cep_DFT     ,     NULL,  NULL, "CEP DFT and IDFT test (single core)");  
  CEP_ADD_RUN(6,cep_FIR     ,       0xFFFFFFFF,  run_cep_FIR     ,     NULL,  NULL, "CEP FIR test (single core)");  
  CEP_ADD_RUN(6,cep_IIR     ,       0xFFFFFFFF,  run_cep_IIR     ,     NULL,  NULL, "CEP IIR test (single core)");  
  CEP_ADD_RUN(6,cep_GPS     ,       0xFFFFFFFF,  run_cep_GPS     ,     NULL,  NULL, "CEP GPS test (single core)");  
  CEP_ADD_RUN(6,cep_MD5     ,       0xFFFFFFFF,  run_cep_MD5     ,     NULL,  NULL, "CEP MD5 test (single core)");  
  CEP_ADD_RUN(6,cep_RSA     ,       0xFFFFFFFF,  run_cep_RSA     ,     NULL,  NULL, "CEP RSA test (single core)");  
  CEP_ADD_RUN(6,cep_SHA256  ,       0xFFFFFFFF,  run_cep_SHA256  ,     NULL,  NULL, "CEP SHA256 test (single core)");
  //
  //
  // must do this 
  //
  cep_set_max_run_count(RUN_zzMAX_CEP_COUNT);
  //
  //
  // =====================================
  return errCnt;
}
//
// ===================================
// display test menu
// ===================================
//
int cep_exe_menu(void)
{
  int errCnt = 0;
  int i,j;
  char tmp[64];
  cep_run_info *cur_p = __cep_run_head_p;
  PRNT("\t============== TEST MENU ==============\n");
  // sorted
  while (cur_p != NULL) {
    // only display if not filter out
    if ((cur_p->id == 0) || (cur_p->filter == 0)) {
      i = cep_testId2index(cur_p->id); // get the index
      strcpy(tmp,cep_get_run_name(i));
      for (j=strlen(tmp);j<MAX_ZERO_FILL;j++) strcat(tmp," ");
      tmp[MAX_ZERO_FILL] = '\0';
      PRNT(" %4d : %s : %s\n",cur_p->id,tmp,cep_get_run_help(i));
    }
    //
    cur_p = cur_p->next;
  }
  return errCnt;
}

int cep_exe_filter(void)
{
  int errCnt = 0;
  //
  cep_run_info *cur_p = __cep_run_head_p;
  // sorted
  while (cur_p != NULL) {
    // only display/run if not filter out
    if (GET_VAR_VALUE(filter) & (1 << cur_p->groupId)) {
      cur_p->filter = 0;
    } else {
      cur_p->filter = 1;
    }
    //
    cur_p = cur_p->next;
  }
  return errCnt;
}

static char *status2name( int status)
{
  static char tmp[32];
  switch (status) {
  case TEST_NOT_RUN : strcpy(tmp,"_____NOT_RUN"); break;
  case TEST_RUNNING : strcpy(tmp,"TEST_RUNNING"); break;
  case TEST_PASSED  : strcpy(tmp,"TEST_PASSED "); break;
  case TEST_FAILED  : strcpy(tmp,"****_FAILED "); break;
  case TEST_IGNORED : strcpy(tmp,"_____IGNORED"); break;
  case TEST_FILTERED: strcpy(tmp,"____FILTERED"); break;
  case TEST_SKIPPED : strcpy(tmp,"_____SKIPPED"); break;
  }
  return tmp;
}
int cep_exe_report(void)
{
  int errCnt = 0;
  int i;
  cep_run_info *cur_p = __cep_run_head_p;
  PRNT("\t============== LAST TEST STATUS ==============\n");
  // sorted
  while (cur_p != NULL) {
    i = cep_testId2index(cur_p->id); // get the index
    PRNT(" %4d : %s : %s\n",cur_p->id,status2name(cep_get_run_status(i)),
	 cep_get_run_name(i));
    //
    cur_p = cur_p->next;
  }  
  return errCnt;
}

//
// MUST be  called to to sort the list
//
void cep_sort_run(void)
{
  int i;
  int inserted =0;
  cep_run_info *last_p, *cur_p;
  int debugMe = 0;
  int maxRunCount = 2; // start from 2
  //
  // clean the link list
  //
  if (debugMe) PRNT("__cep_run_curCount=%d\n",__cep_run_curCount);
  for (i=0;i<__cep_run_curCount;i++) { 
    __cep_run_array[i].next = NULL;
  }
  //
  // first one ALWAYS 0 (runAll)
  //
  __cep_run_head_p = &(__cep_run_array[1]); // start from second one (first one is special)
  for (i=2;i<__cep_run_curCount;i++) { // go down the list
    if (__cep_run_array[i].valid) {
      maxRunCount++; //  
      //
      // add to the sorted list
      //
      last_p = NULL; 
      cur_p  = __cep_run_head_p;
      if (debugMe) PRNT("i=%d Working on %s\n",i,__cep_run_array[i].name);
      inserted = 0;
      while (cur_p != NULL) {
	if (( __cep_run_array[i].groupId < cur_p->groupId) ||
	    ((__cep_run_array[i].groupId == cur_p->groupId) &&
	     (strcmp(__cep_run_array[i].name, cur_p->name) < 0))) {  // i am his little brother!!
	  // add it in the middle
	  if (last_p == NULL) { // I am now the head
	    __cep_run_head_p = &(__cep_run_array[i]);
	  }
	  else {
	    last_p->next = &(__cep_run_array[i]);
	  }
	  __cep_run_array[i].next = cur_p; // big bother
	  inserted = 1; 
	  if (debugMe) PRNT("head=%s: link %s -> %s\n",
			    __cep_run_head_p->name, last_p->name, __cep_run_array[i].name);
	  cur_p  =  NULL; // done
	} else { // next
	  last_p = cur_p;
	  cur_p  = cur_p->next;
	  if (debugMe) PRNT("last=%s cur=%s\n",last_p->name,(cur_p != NULL) ?cur_p->name : "NULL" );
	}
      } // while
      // must be at end of list
      if (!inserted) { // link to  last
	last_p->next = &(__cep_run_array[i]);
	if (debugMe) PRNT("link to last=%s\n",last_p->name);
      }
    } // only if valid entry
  } // for
  // put the first one to the head
  __cep_run_array[0].next = __cep_run_head_p;
  __cep_run_head_p = &(__cep_run_array[0]); // first one always first!! (runAll)
  //
  // now mark the test ID
  cur_p = __cep_run_head_p;
  i = 0; // test starts at 0
  while (cur_p != NULL) {
    cur_p->id = i++;
    cur_p = cur_p->next;
  }
  // create the testid2index map for quick look up instead have to walkdown the linklist
  for (i=0;i<__CEP_RUN_MAX ;i++) {
    __cep_run_testid2index[i] = -1; // clean
  }
  for (i=0;i<__cep_run_curCount;i++) { 
    __cep_run_testid2index[__cep_run_array[i].id] = i;
  }
  // need to adjust __cep_run_curCount
  __cep_validTestCount = maxRunCount;
  // print the menu
  //if (debugMe) (void)cep_exe_menu();
}

int cep_testId2index(int testId) {
  if (__cep_run_testid2index[testId] == -1) {
    LOGE("%s: ERROR, there is no such mapping for testId%d\n",__FUNCTION__,testId);
  }
  return __cep_run_testid2index[testId];
}
int cep_index2testId(int index) {
  return __cep_run_array[index].id;
}

//
// multi threads
//
//
// ==========================================
// Util to lock a thread to particular core
// ==========================================
//
//
//
//
int _thr_errCnt[MAX_CORES];
void cep_set_thr_errCnt(int value) {
  int cpu=  sched_getcpu();
  _thr_errCnt[cpu] = value;
}
int cep_get_thr_errCnt(void) {
  int cpu=  sched_getcpu();
  return _thr_errCnt[cpu];
}
//
// for printf
std::mutex iomutex;
cep_thread_funct_t thr_func; // global function
//
void cep_set_thr_function(cep_thread_funct_t funct) {
  thr_func = funct;
}
cep_thread_funct_t cep_get_thr_function(void) {
  return thr_func;
}

// wait until threadId == cpuID
int thr_waitTilLock(int id, int maxTO) {
  if (GET_VAR_VALUE(noCoreLock)) { return 0; }
  //
  while ((id != sched_getcpu()) && (maxTO > 0)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    maxTO--;
  }
  if (maxTO <= 0) {
    LOGE("Thread %d not locked to same CPU after several attemps\n",id);
    return 1;
  }
  else { return 0; }
}


int run_multiThreads(int coreMask) { // , cep_thread_funct_t funct) {
  int errCnt = 0;
  constexpr unsigned num_threads = MAX_CORES;
  // A mutex ensures orderly access to std::cout from multiple threads.
  //std::mutex iomutex;
  std::vector<std::thread> threads(num_threads);
  for (unsigned i = 0; i < num_threads; ++i) {
    if ((1 << i) & coreMask) {
      _thr_errCnt[i] = 0; // clear the error
      threads[i] = std::thread(cep_get_thr_function(), i);
      //
      // Create a cpu_set_t object representing a set of CPUs. Clear it and mark
      // only CPU i as set.
      if (!GET_VAR_VALUE(noCoreLock)) {
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(i, &cpuset);
	int rc = pthread_setaffinity_np(threads[i].native_handle(), sizeof(cpu_set_t), &cpuset);
	if (rc != 0) {
	  std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
	  errCnt++;
	}
      }
    }
  }
  for (unsigned i = 0; i < num_threads; ++i) {
    if ((1 << i) & coreMask) {
      if (threads[i].joinable()) {
	threads[i].join();
      }
      // get error
      if (_thr_errCnt[i]) { errCnt |= 1 << i; }
    }
  }
  /*
  for (auto& t : threads) {
    if (t.joinable()) {
      t.join();
    }
  }
  */
  return errCnt;
}

