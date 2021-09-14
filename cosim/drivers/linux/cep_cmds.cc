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

#include <unistd.h>
#include <string.h>
#include "cep_io.h"
#include "cep_cmds.h"
#include "cep_vars.h"
#include "cep_token.h"
#include "cep_run.h"
#include "cep_acc.h"
#include "cep_script.h"
#include "cep_macro.h"
#include "cep_dbg.h"

//
// global array to hold cmdiable name
//
#define __CEP_CMD_MAX  128 // for now
int __cep_cmd_curCount = 0; // point to first one
cep_cmd_info __cep_cmd_array[__CEP_CMD_MAX];
// for sorted list
cep_cmd_info *__cep_cmd_head_p = NULL;

void cep_search_cmd_and_printAll(char *cmdName)
{
  int i=0;
  int l0;
  l0 = strlen(cmdName);
  PRNT("Possible matching CMD(s)\n");
  while (i < __cep_cmd_curCount) {
    if (strncasecmp(cmdName, __cep_cmd_array[i].name, 
		    //MIN(l0,strlen(__cep_cmd_array[i].name))) == 0) { // found it
		    l0) == 0) {
      cep_cmd_print(&__cep_cmd_array[i]);
    }
    i++;
  }
}

//
// ==========================
// return NULL if not found
// ==========================
//
cep_cmd_info *cep_search_cmd(char *cmdName, int *matchFound)
{
  int i=0;
  int saveP=0;
  int l0;
  //
  *matchFound = 0;
  // search entire thing to get number of possible matches
   l0 = strlen(cmdName);
  while (i < __cep_cmd_curCount) {
    if (strncasecmp(cmdName, __cep_cmd_array[i].name, 
		    //MIN(l0,strlen(__cep_cmd_array[i].name))) == 0) { // found it
		    l0) == 0) {
      saveP = i;
      *matchFound = *matchFound + 1;
    }
    i++;
  }
  if (*matchFound == 1) { // exactly 1
    return &__cep_cmd_array[saveP];
  } else if (*matchFound > 1) {
    if (GET_VAR_VALUE(verbose)) {
      PRNT("Warning: There are %d matches found for command %s\n",*matchFound,cmdName);
    }
    return &__cep_cmd_array[saveP];
  } else {
    //PRNT("Warning: Could not find command %s\n",cmdName);
    return NULL;
  }
}

//
// ====================================
// to add a command to array
// ====================================
//
cep_cmd_info *cep_cmd_add2idx(int idx, const char *name, int args_count, cep_exe_cmd_vect_t func_t, 
				      const char *funcName, const char *help)
{
  cep_cmd_info *tmp_p = NULL;
  if (idx >= __CEP_CMD_MAX) {
    LOGE("%s: ERROR Can't add any more command. idx=%d exceeds its limit of %d\n",
	 __FUNCTION__,idx,__CEP_CMD_MAX);
    return tmp_p;
  }
  // add to the current one and move to next
  strcpy(__cep_cmd_array[idx].name, name);
  strcpy(__cep_cmd_array[idx].help, help);
  strcpy(__cep_cmd_array[idx].exe_vect_name, funcName);
  __cep_cmd_array[idx].exe_vect = func_t;
  __cep_cmd_array[idx].args_count = args_count;
  __cep_cmd_array[idx].valid = 1; // mark good
  __cep_cmd_array[idx].next = NULL;
  // next
  tmp_p = &__cep_cmd_array[idx];
  //
  return tmp_p;
}
void cep_cmd_print(cep_cmd_info *cmd_p)
{
  char tmp[64];
  int i;
  if (cmd_p == NULL) {
    PRNT("%s; ERROR NULL cmd_p\n",__FUNCTION__);
    return;
  }
  strcpy(tmp,cmd_p->name);
  for (i=strlen(tmp);i<MAX_ZERO_FILL;i++) {
    strcat(tmp," ");
  }
  tmp[MAX_ZERO_FILL] = '\0';
  if (GET_VAR_VALUE(detail)) {
    PRNT("Details info for cmd_struct = %s\n", cmd_p->name);
    PRNT("\t args_count = %d\n",cmd_p->args_count);
    PRNT("\t exe_vect   = %s\n",cmd_p->exe_vect_name);
    PRNT("\t Args/Misc  = %s\n",cmd_p->help);
  } else { // short description
    PRNT("%s Args: %s\n",tmp,cmd_p->help);
  }
}
void cep_cmd_dump_all(void) 
{
  // sorted
  cep_cmd_info *cur_p = __cep_cmd_head_p;
  PRNT("\n==== Available Commands === \n");
  while (cur_p != NULL) {
    cep_cmd_print(cur_p);
    cur_p = cur_p->next;
  }
}


//
// ===============================
// be carefull: no check!!!
// ===============================
//
cep_cmd_info *cep_get_cmd(int idx)
{
  return &__cep_cmd_array[idx];
}
char *cep_get_cmd_name(int idx) 
{
  return (cep_get_cmd(idx))->name;
}
char *cep_get_cmd_help(int idx) 
{
  return (cep_get_cmd(idx))->help;
}
int cep_get_cmd_args_count(int idx) 
{
  return (cep_get_cmd(idx))->args_count;
}
int cep_get_max_cmd_count(void)
{
  return __cep_cmd_curCount;
}
void cep_set_max_cmd_count(int value)
{
  __cep_cmd_curCount = value;
}
//
// ==================================
// based on token, process command and execute it
// ==================================
//
int cep_cmd_process_and_execute(void)
{
  cep_cmd_info *curCmd_p;
  cep_var_info *curVar_p;
  int numMatches;
  int errCnt = 0;
  int ok2modify = 0;
  if (cep_get_token_count() <= 0) {
    LOGE("%s: ERROR: Nothing to process. token count=%d\n",__FUNCTION__,cep_get_token_count());
    return 1;
  }
  // get first token which is the command
  curCmd_p = cep_search_cmd(get_token(0)->ascii,&numMatches);
  //
  if (numMatches > 1) { // multiple?
    LOGE("%s: AMBIGUOUS: found %d (multiple) matches for command (%s)\n",__FUNCTION__,numMatches,get_token(0)->ascii);
    cep_search_cmd_and_printAll(get_token(0)->ascii);
    return 1;
  } 
  else if (numMatches == 1) { // exact match command => execute it
    // ===================================
    // executing command only if enough argument is given
    // ===================================
    if (cep_get_args_count() < curCmd_p->args_count) {
      LOGE("%s: ERROR: Not enough argument(s) for command (%s)\n",__FUNCTION__,curCmd_p->name);
      PRNT("Check Syntax:\n");
      cep_cmd_print(curCmd_p);
      return 1;
    }
    errCnt = (*curCmd_p->exe_vect)();
    // if error is seen, stop script processing and revert back to stdin
    if ((errCnt >= (int)GET_VAR_VALUE(maxErr)) && cep_script_inProgress()) {
      PRNT("%s: Stop scrip executing due to errCnt=%d\n",__FUNCTION__,errCnt);
      cep_script_print_info();
      cep_script_cleanup_all();
    }
  }
  else { // might be variable setting command
    // ===================================
    // search and might change variable setting
    // ===================================
    curVar_p = cep_search_var(get_token(0)->ascii, &numMatches);
    if (numMatches == 1) {
      // might change and then print it
      if (cep_get_args_count() >= 1) { // there is argument to modify the variable
	ok2modify = 1; // assume OK
	if (curVar_p->readOnly) {
	  // did he says "please"? hahhah!!
	  if ((cep_get_args_count() >= 2) && (strcmp(get_token(2)->ascii,"please") == 0)) {
	    ok2modify = 1; // I guess he/she knows what he/she is doing
	  } else {
	    LOGE("%s: ERROR: variable %s is READ-ONLY\n",__FUNCTION__, get_token(0)->ascii);
	    ok2modify = 0;
	  }
	} else if (curVar_p->isNum && !get_token(1)->isNum) {
	  LOGE("%s: ERROR: variable %s is expecting a number but got non-numerical argument instead arg=(%s)\n",
	       __FUNCTION__, get_token(0)->ascii,get_token(1)->ascii);
	  return 1;
	} 
	if (ok2modify) {
	  if (get_token(1)->isNum)  curVar_p->value = get_token(1)->value;
	  strcpy(curVar_p->valStr, get_token(1)->ascii);
	  // execute the var function if it is defined and change
	  if (curVar_p->var_vect != NULL) {
	    errCnt = (*curVar_p->var_vect)();      
	  }
	}
      }
      // SPECIAL!!! Only variable that does not take an argument is QUIT!!!
      if (strncasecmp(get_token(0)->ascii,"quit",4) == 0) {// set it
	SET_VAR_VALUE(quit,1);
      }
      // print it out
      cep_var_print(curVar_p);
    }
    else if (numMatches > 1) {
      LOGE("%s: AMBIGUOUS: found %d (multiple) matches for var (%s)\n",__FUNCTION__,numMatches,get_token(0)->ascii);
      cep_search_var_and_printAll(get_token(0)->ascii);
      return 1;
    } else {
	LOGE("%s: ERROR: Can't find anything to act on (%s)\n",__FUNCTION__,get_token(0)->ascii);
	return 1;
    }
  }
  // if no error , add to the macro if it is active and not a macro command itself
  if (!errCnt && 
      cep_macro_inProgress() && 
      (strcasecmp(curCmd_p->name,"macro") != 0) &&   // skip the macro command
      (strcasecmp(curCmd_p->name,"execute") != 0)) { // skip the execute command (it will expand itself during recording time)
    cep_macro_addCmd(cep_get_curhistory_line());
  }
  return errCnt;
}

//
// ======================================
// Local Commands
// ======================================
//
//
// ===========================
// Script processing command
// ===========================
//
int __simvty_char_pos = 0;
char *__simvty_cmd_p = NULL;
static int run_cep_system(void)
{
#ifdef LINUX_MODE
  __simvty_cmd_p = cep_get_inputLine_p();
  __simvty_char_pos = cep_get_first_args_pos(); // for now!! pass the "vty" command
  printf("==SYSTEM: Executing linux command -> %s",&__simvty_cmd_p[__simvty_char_pos]);
  if (system(&__simvty_cmd_p[__simvty_char_pos]) != -1) return 1;
  else return 0;
#else
  printf("Sorry : system command is not supported\n");
  return 0;
#endif
}

static int cep_run_script(void)
{
  int errCnt = 0;

  errCnt = cep_execute_script(get_token(1)->ascii);
  return errCnt;
}

#define time_delay_us(x) usleep(x)

static int cep_delay(void)
{
  int errCnt = 0;
  if (get_token(1)->isNum) {
    time_delay_us((int)get_token(1)->value);
  }
  return errCnt;
}

#ifdef _MICROKERNEL
extern void sleep_seconds(int numSec);
#else
#define sleep_seconds(x) sleep(x)
#endif

static int cep_sleep(void)
{
  int errCnt = 0;
  if (get_token(1)->isNum) {
    sleep_seconds(get_token(1)->value);
  }
  return errCnt;
}

//
// ===========================
// Help Command
// ===========================
//
static int cep_help(void)
{
  int errCnt = 0;
  cep_cmd_info *cmd_p;
  cep_var_info *var_p;
  int matchFound;
  int printMask = 0;
  if (cep_get_args_count() != 0) { // some argument
    // first search for command
    cmd_p = cep_search_cmd(get_token(1)->ascii, &matchFound);
    if (matchFound != 0) { // print the last one found
      PRNT("Command Syntax:\n");
      cep_cmd_print(cmd_p);
    } else { // search for variable
      var_p = cep_search_var(get_token(1)->ascii, &matchFound);
      if (matchFound != 0) {
	PRNT("Variable Setting:\n");
	cep_var_print(var_p);
      } else if (strcmp(get_token(1)->ascii,"cmd") == 0) {
	printMask = 0x1; // var only
      } else if (strcmp(get_token(1)->ascii,"status") == 0) {
	printMask = 0x2; // readOnly only
      } else if (strcmp(get_token(1)->ascii,"var") == 0) {
	printMask = 0x6; // var only
      } else if (strcmp(get_token(1)->ascii,"acc") == 0) {
	printMask = 0x8; // acc only
      } else {
	printMask = 0xF;
      }
    }
  } else {
    printMask = 0xF;
  }
  //
  // no args or not found
  //
  if (printMask & 0x1) {
    cep_cmd_dump_all();
  }  
  if (printMask & 0x2) {
    cep_var_dump_all(1,0);
  }
  if (printMask & 0x4) {
    cep_var_dump_all(0,1);
  }
  if (printMask & 0x8) {
    cep_acc_dump_all();
  }
  return errCnt;
}
//
// ====================================
// return 1 if error
// ====================================
int cep_init_cmds(void)
{
  int errCnt = 0;
  cep_cmd_info *tmp_p = NULL;
  int i;
  //
  for (i=0;i<__CEP_CMD_MAX;i++) {
#if 1
    memset(&(__cep_cmd_array[i]), 0x0, sizeof(cep_cmd_s_t));
    strcpy(__cep_cmd_array[i].name,"__UNKNOWN__");
#else
    strcpy(__cep_cmd_array[i].name,"__UNKNOWN__");
    __cep_cmd_array[i].valid = 0; // invalid
    __cep_cmd_array[i].exe_vect = NULL;
#endif
  }
  //
  // ============================================
  // This is where you add global command to CEP
  // ============================================
  //
  //            Name  minArgCnt  func2Exe                Help Args List
  CEP_ADD_CMD(ignore,     0, cep_exe_ignore,     "[<testId2ignore> ...] (-<testId2ignore> to un-ignore)");
  CEP_ADD_CMD(run,	  1, cep_exe_run,        "<testStartId> [testEndId]"); 
  CEP_ADD_CMD(menu,	  0, cep_exe_menu,       "--(see filter)--"); 
  CEP_ADD_CMD(report,     0, cep_exe_report,     "[report test status from last run command]"); 
  CEP_ADD_CMD(read,	  2, cep_acc_read,       "<offset> <deviceName> [optionalArgs]"); 
  CEP_ADD_CMD(compare,    4, cep_acc_compare,    "<offset> <expectData> <mask> <deviceName> [optionalArgs]"); 
  CEP_ADD_CMD(regTest,    4, cep_acc_regTest,    "<offset> <expectData> <mask> <deviceName> [optionalArgs]"); 
  CEP_ADD_CMD(write,      3, cep_acc_write,      "<offset> <data> <deviceName> [optionalArgs]"); 
  CEP_ADD_CMD(delay,      1, cep_delay,          "<usecond>");
  CEP_ADD_CMD(sleep,      1, cep_sleep,          "<seconds>");
  CEP_ADD_CMD(device,     0, cep_acc_dump_all,   "[display list of deviceName for read/write/other access]");
  CEP_ADD_CMD(dump,       1, cep_acc_dump,       "<deviceName> [optionalArgs]");
  CEP_ADD_CMD(help,	  0, cep_help,           "[cmd | var | status | acc | <nameOfCmd_or_Variable]"); 
  CEP_ADD_CMD(macro,      1, cep_process_macro,  "<start|stop|delete|run|list|dump> <macroName>"); 
  CEP_ADD_CMD(history,    0, cep_print_history,  "display command history (support !<historyId> or !-<hisrotyId> )"); 
  CEP_ADD_CMD(execute,    1, cep_run_script,     "<script_file_name>"); 
  CEP_ADD_CMD(setvar ,    2, cep_exe_setvar,     "<varName> <varValue>: create/assign <varName>=<varValue>");
  CEP_ADD_CMD(getvar ,    1, cep_exe_getvar,     "<varName>: prompt for value of new variable to be created");
  CEP_ADD_CMD(adjvar ,    3, cep_exe_adjvar,     "<varName> <+,-,*,/,>>,<<> <value>: adjust variable value");
  CEP_ADD_CMD(memWrite,   6, cep_acc_memWrite,   "<deviceName> <devId> <memType> <memId> <offset> <data0 data1...>"); 
  CEP_ADD_CMD(memRead,    5, cep_acc_memRead,    "<deviceName> <devId> <memType> <memId> <offset> [word2read]"); 
  CEP_ADD_CMD(memSearch,  7, cep_acc_memSearch,  "<deviceName> <devId> <memType> <memId> <offset> <#2search> <pattern>");
  CEP_ADD_CMD(system,     1, run_cep_system,     "system <linux_cmd...>");  

  // must do this 
  cep_set_max_cmd_count(CMD_zzMAX_CEP_COUNT);

  return errCnt;
}

//
// MUST be  called to to sort the list
//
void cep_sort_cmds(void)
{
  int i;
  int inserted =0;
  cep_cmd_info *last_p, *cur_p;
  int debugMe = 0;
  // clean the link list
  if (debugMe) PRNT("__cep_cmd_curCount=%d\n",__cep_cmd_curCount);
  for (i=0;i<__cep_cmd_curCount;i++) { 
    __cep_cmd_array[i].next = NULL;
  }
  //
  __cep_cmd_head_p = &(__cep_cmd_array[0]); // start from first one
  for (i=1;i<__cep_cmd_curCount;i++) { // go down the list
    if (__cep_cmd_array[i].valid) {
      //
      // add to the sorted list
      //
      last_p = NULL; 
      cur_p  = __cep_cmd_head_p;
      if (debugMe) PRNT("Working on %s\n",__cep_cmd_array[i].name);
      inserted = 0;
      while (cur_p != NULL) {
	if (strcmp(__cep_cmd_array[i].name, cur_p->name) < 0) {  // i am his little brother!!
	  // add it in the middle
	  if (last_p == NULL) { // I am now the head
	    __cep_cmd_head_p = &(__cep_cmd_array[i]);
	  }
	  else {
	    last_p->next = &(__cep_cmd_array[i]);
	  }
	  __cep_cmd_array[i].next = cur_p; // big bother
	  inserted = 1; 
	  if (debugMe) PRNT("head=%s: link %s -> %s\n",__cep_cmd_head_p->name, last_p->name, __cep_cmd_array[i].name);
	  cur_p  =  NULL; // done
	} else { // next
	  last_p = cur_p;
	  cur_p  = cur_p->next;
	  if (debugMe) PRNT("last=%s cur=%s\n",last_p->name,(cur_p != NULL) ? cur_p->name : "NULL");
	}
      } // while
      // must be at end of list
      if (!inserted) { // link to  last
	last_p->next = &(__cep_cmd_array[i]);
	if (debugMe) PRNT("link to last=%s\n",last_p->name);
      }
    } // only if valid
  } // for
  // debug
  //cep_cmd_dump_all();
}


