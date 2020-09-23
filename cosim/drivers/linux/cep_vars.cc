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

#include <string.h>
#ifndef _MICROKERNEL
#include <time.h>
#endif
#include "stdlib.h"
#include "simdiag_global.h"

#include "cep_io.h"
#include "cep_vars.h"
#include "cep_run.h"
#include "cep_token.h"
#include "cep_acc.h"


//extern int init_xmchip_config_param(void) ;

//
// global array to hold variable name
//
#define __CEP_VAR_MAX  512 // for now
int __cep_var_curCount = 0; // point to first one
cep_var_info __cep_var_array[__CEP_VAR_MAX];
// for sorted list
cep_var_info *__cep_var_head_p = NULL;

//
// ====================================
// var execute function
// ====================================
//
static int run_verbose(void)
{
  simdiag_SetVerbose(GET_VAR_VALUE(verbose));
  return 0;
}

int set_stty(void)
{
  if (GET_VAR_VALUE(xterm) == 0) { // stty??
    PRNT("Setting terminal to VT102 with erase=^H\n");
    system("stty erase ^H");
  }
  return 0;
}

#if 0
extern u_int8_t                 syslog_level;
static int run_syslevel(void)
{
  syslog_level = GET_VAR_VALUE(syslevel);
  return 0;
}
#endif

//
// handle revision check
//
cep_exe_var_vect_t revCheck_func = NULL;
void cep_set_run_revCheck_func(cep_exe_var_vect_t checkFunc)
{
  revCheck_func = checkFunc;
}

static int run_revCheck(void) {
  if (revCheck_func != NULL) {
    return (*revCheck_func)();
  }
  return 0;
}

#if 0
extern int bistVerifyOnly;
static int bistFunc(void)
{
  bistVerifyOnly = GET_VAR_VALUE(bistVerifyOnly);
  return 0;
}
#endif

//
// from bootCpld
//
#if 0
extern void set_mic_select_n_enable(int mic_id);
static int run_curMic(void)
{
  set_mic_select_n_enable(GET_VAR_VALUE(curMic));
  return 0;
}
#endif


//extern int pex_int_setup(void);
//
// ====================================
// return 1 if error
// ====================================
int cep_init_vars(void)
{
  int errCnt = 0;
  cep_var_info *tmp_p = NULL; 
  int i;
  for (i=0;i<__CEP_VAR_MAX;i++) {
#if 1
    memset(&(__cep_var_array[i]), 0x0, sizeof(cep_var_s_t));
    strcpy(__cep_var_array[i].name, "___UNKNOWN__");
#else
    strcpy(__cep_var_array[i].name, "___UNKNOWN__");
    __cep_var_array[i].valid = 0; // invalid
    __cep_var_array[i].var_vect = NULL;
#endif
  }
  //
  // ======================================================
  // This is where you add global control variable to CEP
  // ======================================================
  //            name           isNum  DefaultVaue FuncVect Description
  CEP_ADD_VAR(coreMask,        1,     0xf,        NULL,       "Core Mask");
  CEP_ADD_VAR(noCoreLock,      1,     0x0,        NULL,       "no CORE Lock (default=lock)");  
  //
  CEP_ADD_VAR(loop,    	       1, 1,          NULL,           "number of test iterations");
  CEP_ADD_VAR(mloop,           1, 1,          NULL,           "loop for macro/script execution");
  CEP_ADD_VAR(curErrCnt,       1, 0,          NULL,           "current error count");
  CEP_ADD_VAR(lockSeed,        1, 0,          NULL,           "1=seed is locked, 0=seed is random");
  CEP_ADD_VAR(maxErr,          1, 1,          NULL,           "max number of errors before stop test");
  CEP_ADD_VAR(regress,         1, 0,          NULL,           "1=regress mode on, 0=off");
  CEP_ADD_VAR(quit,    	       1, 0,          NULL,           "1=quit cep_test");
  CEP_ADD_VAR(seed,    	       1, 0,          NULL,           "random seed uses for every test");
  CEP_ADD_VAR(verbose,	       1, 0,          run_verbose,    "verbose level");
  CEP_ADD_VAR(skipInit,	       1, 0,          NULL,           "1=skip the init, 0=do the init");
  CEP_ADD_VAR(prompt,	       1, 0,          NULL,           "1=wait for user input, 0=no wait");
  CEP_ADD_VAR(step   ,         1, 3,          NULL,           "memory test increment step value");
  CEP_ADD_VAR(revCheck,        1, 1,          run_revCheck,   "revision Check Enable (1), 0=don't check for revision");
  CEP_ADD_VAR(longRun,         1, 0,          NULL,           "1=full (long) test enable");
  CEP_ROA_VAR(lastDataRead,    1, 0,          NULL,           "data from last read");
  CEP_ROA_VAR(cur_loop,        1, 0,          NULL,           "DONT TOUCH");
  CEP_ADD_VAR(stop_when_done,  1, 0,          NULL,           "1=stop PRBS generator when test done, 0=no stop");
  CEP_ADD_VAR(filter,          1, -1,         cep_exe_filter, "Test filter mask");
  CEP_ADD_VAR(detail,          1, 0,          NULL,           "1=print detailed info, 0=minimal info");
  CEP_ADD_VAR(xterm,           1, 0,          set_stty,       "xterm versus tty");  
  // ReadOnly
  CEP_ADD_VAR(testMask,        1, -1,         NULL,           "test Mask to pick target to run test.");
  //
  //
  // must do this 
  //
  cep_set_max_var_count(VAR_zzMAX_CEP_COUNT);
  //
  // =====================================
  return errCnt;
}
// create/add new variable to table
int cep_exe_setvar(void)
{
  int errCnt = 0;
  cep_var_info *curVar_p = NULL;
  int numMatches;
  if (VERBOSE1()) {
    PRNT("%s: <%s> <%s>\n",__FUNCTION__,get_token(1)->ascii,get_token(2)->ascii);
  }
  // get varName and make sure it is not yet defined
  curVar_p = cep_search_var(get_token(1)->ascii, &numMatches);
  if ((curVar_p == NULL) && (numMatches == 0)) {
    // check to make sure there is room
    if (__cep_var_curCount >= __CEP_VAR_MAX) {
      PRNT("%s: Sorry, no more room for new variable <%s> because it might already there\n",
	   __FUNCTION__,get_token(1)->ascii);
      errCnt++;
    } else {
      // add to the list
      curVar_p = cep_var_add2idx(__cep_var_curCount, // next free one
				     get_token(1)->ascii, // the name
				     get_token(2)->isNum, // is a number?
				     0, // write-able
				     NULL, // FIXME!!
				     get_token(2)->ascii, // valueStr
				     "dynamic variable");
      __cep_var_curCount++; // add to the list
      // resort
      cep_sort_vars();
      PRNT("%s: variable <%s> = <%s> is added\n",__FUNCTION__,get_token(1)->ascii,get_token(2)->ascii);
    }
  } else if ((curVar_p != NULL) && (numMatches == 1)) { // replace
    // replace
    strcpy(curVar_p->valStr, get_token(2)->ascii);
    curVar_p->isNum    = get_token(2)->isNum;
    curVar_p->readOnly = 0;
    curVar_p->valid    = 1; // mark good
    if (get_token(2)->isNum) {
      curVar_p->value = strtoll(get_token(2)->ascii, (char **)NULL, 0);
    }
    PRNT("%s: variable <%s> = <%s> is replaced\n",__FUNCTION__,get_token(1)->ascii,get_token(2)->ascii);
  } else {
    PRNT("%s: Sorry, can't create new variable <%s> because it might already there\n",
	 __FUNCTION__,get_token(1)->ascii);
    errCnt++;
  }
  return errCnt;
}

// add/sub/multiply/divide an exiting variable 
int cep_exe_adjvar(void)
{
  int errCnt = 0;
  cep_var_info *curVar_p = NULL;
  int numMatches;
  u_int64_t tmp;
  if (VERBOSE1()) {
    PRNT("%s: <%s> <%s> <%s>\n",__FUNCTION__,get_token(1)->ascii,get_token(2)->ascii,get_token(3)->ascii);
  }
  //
  // get varName and make sure it is there
  curVar_p = cep_search_var(get_token(1)->ascii, &numMatches);
  if ((curVar_p != NULL) && (numMatches == 1)) { // found exact match
    if (get_token(3)->isNum) { // only take number for now
      // modify the variable
      if (strcmp(get_token(2)->ascii,"+") == 0) { // add operation
	curVar_p->value += get_token(3)->value;
      } 
      else if (strcmp(get_token(2)->ascii,"-") == 0) { // subtract operation
	curVar_p->value -= get_token(3)->value;
      }  
      else if (strcmp(get_token(2)->ascii,"*") == 0) { // multiply operation
	curVar_p->value *= get_token(3)->value;
      }  
      else if (strcmp(get_token(2)->ascii,"/") == 0) { // divide operation
	if (get_token(3)->value != 0) {
	  curVar_p->value = curVar_p->value / get_token(3)->value;
	} else {
	  PRNT("%s: can't divide by zero\n", __FUNCTION__);
	  errCnt++;
	  return 1;
	}
      }
      else if (strcmp(get_token(2)->ascii,">>") == 0) { // right rotate
	tmp = ((u_int64_t)curVar_p->value << 32) >> get_token(3)->value;
	curVar_p->value = (u_int32_t)(tmp>>32) | (u_int32_t)((tmp & 0xffffffff)<<32);
      }
      else if (strcmp(get_token(2)->ascii,"<<") == 0) { // left rotate
	tmp = (u_int64_t)curVar_p->value << get_token(3)->value;
	curVar_p->value = (u_int32_t)(tmp>>32) | (u_int32_t)(tmp & 0xffffffff);
      } else {
	PRNT("%s: Sorry, non-supported operation <%s>\n",__FUNCTION__,get_token(2)->ascii);
	errCnt++;
	return 1;
      }
      // must also update the valStr
      sprintf(curVar_p->valStr,"0x%x",(u_int32_t)curVar_p->value); // might be negative
      if (VERBOSE1()) {
	PRNT("%s: variable <%s> is adjusted to <%s>\n",__FUNCTION__,curVar_p->name,curVar_p->valStr);
      }
      // there might be a function to execute after adjustment
      if (curVar_p->var_vect != NULL) {
	    errCnt = (*curVar_p->var_vect)();      
      }
    } else {
      PRNT("%s: argument#3 <%s> must be a number\n", __FUNCTION__,get_token(3)->ascii);
      errCnt++;
      return 1;
    }
  } else {
    PRNT("%s: Sorry, can't adjust variable <%s> because there might be multiple matches or non-supported ops\n",
	 __FUNCTION__,get_token(1)->ascii);
    errCnt++;
  }
  return errCnt;
}

int cep_exe_monitor(void)
{
  int errCnt = 0;
  if (get_token(1)->isNum) { 
    SET_VAR_VALUE(stop_when_done,get_token(1)->value ? 0 : 1);
  }
  PRNT("FIX ME\n");
  return errCnt;
}

//
// create/add new variable to table and prompt at the screen for its value
//
#ifdef _MICROKERNEL
extern int cep_get_a_line (char *prompt, char *line_out);
#endif
int cep_exe_getvar(void)
{
  char str[256], save[128];
  if (VERBOSE2()) {
    PRNT("%s: <%s>\n",__FUNCTION__,get_token(1)->ascii);
  }
  //
  // promtp
  //
  strcpy(save,get_token(1)->ascii); //  save away the variable name
  PRNT("Please enter the value for new variable <%s>\n",save);
  str[0] = '\0';
  do {
    // NOTE promt to cep_get_a_line must be less ten 10 character
#ifdef _MICROKERNEL
    (void)cep_get_a_line("-> ",str);
#else
    //PRNT("\n%s",prompt);
    fgets(str,256, stdin);
#endif
    // extract the value
    if (VERBOSE2()) {
      PRNT("%s: ValueEnter:<%s>\n",__FUNCTION__,str);
    }
    cep_ProcessTokens(str, 0); // not to save to history
  } while (cep_get_token_count() == 0);
  //
  // 
  cep_set_token(2,get_token(0)->ascii);
  cep_set_token(1,save);
  cep_set_token(0,"setvar");
  //
  cep_set_token_count(3); // setvar <var> <value>
  return cep_exe_setvar();
}

//
// MUST be  called to to sort the list
//
void cep_sort_vars(void)
{
  int i;
  int inserted =0;
  cep_var_info *last_p, *cur_p;
  int debugMe = 0;
  // clean the link list
  for (i=0;i<__cep_var_curCount;i++) { 
    __cep_var_array[i].next = NULL;
  }
  //
  __cep_var_head_p = &(__cep_var_array[0]); // start from first one
  for (i=1;i<__cep_var_curCount;i++) { // go down the list
    if (__cep_var_array[i].valid) {
      //
      // add to the sorted list
      //
      last_p = NULL; 
      cur_p  = __cep_var_head_p;
      if (debugMe) PRNT("Working on %s\n",__cep_var_array[i].name);
      inserted = 0;
      while (cur_p != NULL) {
	if (strcasecmp(__cep_var_array[i].name, cur_p->name) < 0) {  // i am his little brother!!
	  // add it in the middle
	  if (last_p == NULL) { // I am now the head
	    __cep_var_head_p = &(__cep_var_array[i]);
	  }
	  else {
	    last_p->next = &(__cep_var_array[i]);
	  }
	  __cep_var_array[i].next = cur_p; // big bother
	  inserted = 1; 
	  if (debugMe) PRNT("head=%s: link %s -> %s\n",__cep_var_head_p->name, last_p->name, __cep_var_array[i].name);
	  cur_p  =  NULL; // done
	} else { // next
	  last_p = cur_p;
	  cur_p  = cur_p->next;
	  if (debugMe) PRNT("last=%s cur=%s\n",last_p->name,cur_p->name);
	}
      } // while
      // must be at end of list
      if (!inserted) { // link to  last
	last_p->next = &(__cep_var_array[i]);
	if (debugMe) PRNT("link to last=%s\n",last_p->name);
      }
    } // only if valid
  } // for
}

//
// ==========================
// return NULL if not found
// ==========================
//
cep_var_info *cep_search_var(char *varName, int *matchFound)
{
  int i=0;
  int saveP=0;
  int l0;
  // search entire thing to get number of possible matches
  *matchFound = 0;
  l0 = strlen(varName);
  while (i < __cep_var_curCount) {
    if (strncasecmp(varName, __cep_var_array[i].name, 
		    //MIN(l0,strlen(__cep_var_array[i].name))) == 0) { // found it
		    l0) == 0) {
      saveP = i;
      *matchFound = *matchFound + 1;
    }
    i++;

  }
  if (*matchFound == 1) { // exactly 1
    return &__cep_var_array[saveP];
  } else if (*matchFound > 1) {
    if (GET_VAR_VALUE(verbose)) {
      PRNT("Warning: There are %d matches found for this variable %s\n",*matchFound,varName);
    }
    return &__cep_var_array[saveP];
  } else {
    //PRNT("ERROR: Could not find this variable %s\n",varName);
    return NULL;
  }
}

//
void cep_search_var_and_printAll(char *varName)
{
  int i=0;
  int l0;
  l0 = strlen(varName);
  PRNT("Possible matching VAR(s)\n");
  while (i < __cep_var_curCount) {
    if (strncasecmp(varName, __cep_var_array[i].name, 
		    //MIN(l0,strlen(__cep_var_array[i].name))) == 0) { // found it
		    l0) == 0) {
      cep_var_print(&__cep_var_array[i]);
    }
    i++;
  }
}

//
// ====================================
// to add a variable to array
// ====================================
//
cep_var_info *cep_var_add2idx(int idx, const char *name, int isNum, int readOnly,
				      cep_exe_var_vect_t var_vect,
				      const char *strDefaultValue, const char *help)
{
  cep_var_info *tmp_p = NULL;
  if (idx  >= __CEP_VAR_MAX) {
    PRNT("%s: ERROR Can't add any more variable. idx=%d exceed its limit of %d\n",
	 __FUNCTION__,idx,__CEP_VAR_MAX);
    return tmp_p;
  }
  // add to the current one and move to next
  strcpy(__cep_var_array[idx].name, name);
  strcpy(__cep_var_array[idx].valStr, strDefaultValue);
  strcpy(__cep_var_array[idx].help, help);
  __cep_var_array[idx].isNum    = isNum;
  __cep_var_array[idx].readOnly = readOnly;
  __cep_var_array[idx].valid    = 1; // mark good
  __cep_var_array[idx].var_vect = var_vect;
  // for link list
  __cep_var_array[idx].next     = NULL;
  if (isNum) {
    __cep_var_array[idx].value = strtoll(strDefaultValue, (char **)NULL, 0);
  }
  // next
  tmp_p = &__cep_var_array[idx];
  //
  return tmp_p;
}

//
// ====================================
// to overload a function vector to exiting one
// ====================================
//
cep_var_info *cep_var_changeFuncVec(int idx, cep_exe_var_vect_t var_vect)
{
  cep_var_info *tmp_p = NULL;
  if (idx  >= __CEP_VAR_MAX) {
    PRNT("%s: ERROR Can't add any more variable. idx=%d exceed its limit of %d\n",
	 __FUNCTION__,idx,__CEP_VAR_MAX);
    return tmp_p;
  }
  __cep_var_array[idx].var_vect = var_vect;
  //
  return tmp_p;
}

//
// ====================================
// modify a variable to its new value
// return errCnt
// ====================================
//
int cep_var_modify(char *varName, char *newStrValue)
{
  cep_var_info *tmp_p;
  int errCnt = 0;
  int matchFound=0;
  //
  // first search for it
  //
  if (((tmp_p = cep_search_var(varName, &matchFound)) == NULL) || (matchFound != 1)) {
    PRNT("%s: ERROR: problem with finding variable %s to modify. matchFound=%d\n",__FUNCTION__,varName,matchFound);
    return 1;
  }
  //
  // print
  //
  if (GET_VAR_VALUE(verbose)) {
    if (tmp_p->isNum) {
      PRNT("var=%s old=%s(0x%08x) -> new=%s\n",tmp_p->name, tmp_p->valStr, tmp_p->value, newStrValue);
    } else {
      PRNT("var=%s old=%s -> new=%s\n",tmp_p->name, tmp_p->valStr, newStrValue);
    }
  }
  //
  // modify
  //
  if (tmp_p->isNum) { // convert to number
    tmp_p->value = strtoll(newStrValue, (char **)NULL, 0);
  }
  // also its string just in case
  strcpy(tmp_p->valStr, newStrValue);
  //
  return errCnt;
}

void cep_var_print(cep_var_info *var_p)
{
  char str[128],tmp[64];
  int i;
  if (var_p == NULL) {
    PRNT("%s; ERROR NULL var_p\n",__FUNCTION__);
    return;
  }
  strcpy(tmp,var_p->name);
  for (i=strlen(tmp);i<MAX_ZERO_FILL;i++) {
    strcat(tmp," ");
  }
  tmp[MAX_ZERO_FILL] = '\0';
  // 
  if (var_p->isNum) {
    snprintf(str,sizeof(str),"0x%08x (%d)",var_p->value,var_p->value);
  } else {
    snprintf(str,sizeof(str),"%s (string)",var_p->valStr);
  }
  for (i=strlen(str);i<MAX_ZERO_FILL+5;i++) {
    strcat(str," ");
  }
  str[MAX_ZERO_FILL+5] = '\0';
  //
  PRNT("%s = %s : %s\n",tmp,str,var_p->help);
}
void cep_var_dump_all(int readOnly, int rdwr)
{
  // sorted
  cep_var_info *cur_p;
  // first ReadOnly
  if (readOnly) {
    PRNT("\n==== Status (ReadOnly Variables) === \n");
    cur_p = __cep_var_head_p;
    while (cur_p != NULL) {
      if (cur_p->readOnly) {
	cep_var_print(cur_p);
      }
      cur_p = cur_p->next;
    }
  }
  if (rdwr) {
    PRNT("\n==== Available Variables === \n");
    cur_p = __cep_var_head_p;
    // first ReadOnly
    while (cur_p != NULL) {
      if (cur_p->readOnly == 0) {
	cep_var_print(cur_p);
      }
      cur_p = cur_p->next;
    }
  }
}
//
// ===============================
// be carefull: no check!!!
// ===============================
//
cep_var_info *cep_get_var(int idx)
{
  return &__cep_var_array[idx];
}
char *cep_get_var_name(int idx) {
  return (cep_get_var(idx))->name;
}
u_int32_t cep_get_var_value(int idx) 
{
  return (cep_get_var(idx))->value;
}
void cep_set_var_value(int idx, u_int32_t value)
{
  (cep_get_var(idx))->value = value;
  // also need to update the string associate with it so it can be used later (use as string)
  sprintf((cep_get_var(idx))->valStr,"0x%x",value);
}
void cep_inc_var_value(int idx, u_int32_t value)
{
  (cep_get_var(idx))->value += value;
}
char *cep_get_var_valStr(int idx) 
{
  return (cep_get_var(idx))->valStr;
}
int cep_get_max_var_count(void)
{
  return __cep_var_curCount;
}
void cep_set_max_var_count(int value)
{
  __cep_var_curCount = value;
}

#ifdef _MICROKERNEL
#include "time.h"
#include "stdtime/stdtime.h"
#endif

void cep_set_new_seed(void)
{
#ifdef _MICROKERNEL
  struct timeval       tv;
  if (!GET_VAR_VALUE(lockSeed)) {
    gettimeofday(&tv, NULL);
    SET_VAR_VALUE(seed, GET_VAR_VALUE(seed) + ((int) tv.tv_sec));
  }
#else
  u_int32_t tmp;
  if (!GET_VAR_VALUE(lockSeed)) {
    tmp = (u_int32_t) time(NULL);
    SET_VAR_VALUE(seed, (tmp & 0x1) ? tmp : ((tmp >> 16) | (tmp << 16)));
  }
#endif
}

#ifdef _MICROKERNEL
static struct timeval last_tod = {0,0};
#endif

int cep_exe_timeStamp_mark(void) {
#ifdef _MICROKERNEL
  gettimeofday(&last_tod, NULL);
#endif      
  return 0;
}


int cep_exe_timeStamp(void)
{
  if (cep_get_token_count() > 1) {
    if (strncasecmp(get_token(1)->ascii,"reset",3) == 0) {
#ifdef _MICROKERNEL
      gettimeofday(&last_tod, NULL);
#endif      
    }
  }
  // then print out the current
  return cep_print_timeStamp();
}

int cep_print_timeStamp(void)
{
#ifdef _MICROKERNEL
  struct timeval       tod;
  int totalSeconds, days, hours, minutes, seconds, usec;
  //
  gettimeofday(&tod, NULL);
  totalSeconds=  (int)(tod.tv_sec - last_tod.tv_sec);
  seconds = totalSeconds;
  if (last_tod.tv_usec > tod.tv_usec) {
    usec = 1000000 + tod.tv_usec - last_tod.tv_usec;
    seconds = seconds - 1;
  } else {
    usec = tod.tv_usec - last_tod.tv_usec;
  }
  days    = seconds / (24 * 60 * 60); seconds = seconds - (days * 24 * 60 * 60);
  hours   = seconds / (60 * 60); seconds = seconds - (hours * 60 * 60);
  minutes = seconds / 60; seconds = seconds - (minutes * 60); 
  printf("Current timer since last reset: %d days %d hours %d minutes %d seconds %d usec (total=%d seconds)\n",
	 days,hours,minutes,seconds,usec, totalSeconds);
#else
  printf("*** ADD ME **\n");
#endif
  return 0;
}

void wait4prompt(void)
{
  if (GET_VAR_VALUE(prompt)) {
    PRNT("Press return when ready\n");
  }
}
