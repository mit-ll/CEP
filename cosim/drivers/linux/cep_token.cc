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

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define thread_yield() 

#include "simdiag_global.h"
#include "cep_io.h"
#include "cep_token.h"
#include "cep_vars.h"
#include "cep_script.h"


//
// ==========================
// globals
// ==========================
//
cep_token_t __gTokens[16]; // max is 16
int __gTokenCount = 0;

//
// for history tracking
//
cep_comd_t _gHistoryStr[CEP_MAX_HISTORY];
int _gHistoryCurPtr = 0;
int _gHistoryPtr = 0;
int _gHistoryCnt = 0;
//
// some quickies
//
// make this global
char cep_inputLine[256];
int cep_first_args_pos = 0;

char *cep_get_inputLine_p(void) {
  return cep_inputLine;
}
int cep_get_first_args_pos(void) {
  return cep_first_args_pos;
}
void cep_set_first_args_pos(int pos)
{
  cep_first_args_pos = pos;
}

cep_token_t *get_token(int token_id)
{
  return &(__gTokens[token_id]);
}
void cep_copy_token(int src, int dst)
{
  strcpy(__gTokens[dst].ascii, __gTokens[src].ascii);
  __gTokens[dst].value = __gTokens[src].value;
  __gTokens[dst].isNum = __gTokens[src].isNum;
}
void cep_set_token(int token_id, const char *name) {
  size_t i;
//  int neg =0;
  strcpy(__gTokens[token_id].ascii, name);
  __gTokens[token_id].isNum = 1; // assume numbers
  for (i=0;i<strlen(name);i++) { 
    if ((i==0) && (name[0] == '-')) { 
      //    neg = 1; 
    }
    else if ((name[i] != 'x') && !isxdigit(name[i])) { // can be negative too
      __gTokens[token_id].isNum = 0;
      break;
    }
  }
  // get the value anyway
  __gTokens[token_id].value = strtoll(name, (char **)NULL, 0);
}

int cep_get_token_count(void)
{
  return __gTokenCount;
}
void cep_set_token_count(int value)
{
  __gTokenCount = value;
}

int cep_get_args_count(void)
{
  return __gTokenCount-1;
}
//
// ====================================
// Handling command history
// ====================================
//
int cep_print_history(void) {
  int i, max,ptr;
  if (_gHistoryCnt < CEP_MAX_HISTORY) {
    ptr = 0; // top
    max = _gHistoryCnt;
  } else { //
    ptr = _gHistoryPtr; // last
    max = CEP_MAX_HISTORY;
  }
  for (i=0;i<max;i++) {
    PRNT("   %4d : %s\n",_gHistoryStr[ptr].cmdId,_gHistoryStr[ptr].ascii);
    ptr = (ptr + 1) % CEP_MAX_HISTORY; // wrap around
  }
  return 0;
}

static char *cep_get_history_line(int hisId) 
{
  int ptr = _gHistoryPtr;
  int i;
  //PRNT("%s: %d gHistoryPtr=%d\n",__FUNCTION__,hisId,_gHistoryPtr);
  if (hisId < 0) { // relative from current
    hisId = hisId * -1; // +
    if (ptr >= hisId) { // just back
      ptr = ptr - hisId;
    } else {
      ptr = CEP_MAX_HISTORY + ptr - hisId;
    }
    //PRNT("%s: %d gHistoryPtr=%d ptr=%d\n",__FUNCTION__,hisId,_gHistoryPtr,ptr);
    if (hisId < CEP_MAX_HISTORY) {
      return _gHistoryStr[ptr].ascii;
    } else {
      return NULL;
    }
  } else { // absolute history Id
    // search for absolute one
    for (i=0;i<CEP_MAX_HISTORY;i++) {
      if (_gHistoryStr[ptr].cmdId == hisId) {
	//PRNT("%s: found %d gHistoryPtr=%d ptr=%d ->%s\n",__FUNCTION__,
	//hisId,_gHistoryPtr,ptr,_gHistoryStr[ptr].ascii);
	return _gHistoryStr[ptr].ascii;
      }
      ptr = (ptr + 1) % CEP_MAX_HISTORY; // wrap around
    }
  }
  return NULL;
}

// return true of it is starts with a "!", !!, !<hisId>, !-<hisId>, !<cmd..> are supportted
static int cep_is_history_cmd(char *cmd, char **outLine) {
  int isHis = 0; // default
  int i,hisId, max, len;
  int ptr;
  //
  if (VERBOSE3()) { PRNT("%s: <%s>\n",__FUNCTION__,cmd); }
  if (cmd[0] == '!') {
    if (cmd[1] == '!') { // !!
      if (VERBOSE3()) { PRNT("%s: i am here1\n",__FUNCTION__); }
      isHis = 1;
      hisId = -1; // last command
      *outLine = cep_get_history_line(hisId);
      if (*outLine == NULL) { // can't find it
	isHis = 0;
      }
      return isHis;
    }
    else if ((isdigit(cmd[1])) || // can't be xditgit because !d (d = hex)
	     ((cmd[1] == '-') && (isdigit(cmd[2])))) { // !<hisId> or !-<hisId>
      if (VERBOSE3()) { PRNT("%s: i am here2\n",__FUNCTION__); }
      isHis = 1;
      hisId = strtoll(&(cmd[1]), (char **)NULL, 0);
      *outLine = cep_get_history_line(hisId);
      if (*outLine == NULL) { // can't find it
	isHis = 0;
      }
      return isHis;
    } else { // search backward for the string starting from last command
      if (VERBOSE3()) { PRNT("%s: i am here3\n",__FUNCTION__); }
      len = strlen(&cmd[1]);
      ptr = _gHistoryPtr - 1; // start from last
      if (ptr < 0) { ptr += CEP_MAX_HISTORY; } // wrap around
      if (_gHistoryCnt < CEP_MAX_HISTORY) {
	max =  _gHistoryCnt;
      } else { //
	max = CEP_MAX_HISTORY;
      }
      if (VERBOSE3()) { 
	PRNT("search: max=%d ptr=%d hisPtr=%d len=%d cmd=<%s>\n",max,ptr,_gHistoryPtr,len,cmd);
      }
      for (i=0;i<max;i++) {
	if (VERBOSE3()) { PRNT("%s: i=%d checking <%s>\n",__FUNCTION__,i,_gHistoryStr[ptr].ascii); }
	if (strncasecmp(_gHistoryStr[ptr].ascii, &cmd[1],len) == 0) { // found it
	  isHis = 1;
	  *outLine = _gHistoryStr[ptr].ascii;
	  if (VERBOSE3()) { PRNT("found i=%d: %s\n",i,*outLine); }
	  return isHis; // done
	}
	ptr = ptr - 1; // next last
	if (ptr < 0) { ptr += CEP_MAX_HISTORY; } // wrap around
      }
    }
  }
  return isHis;
}

//
// return true of it is starts with a "$", then search variable list and replace with an equivalent string
//
static int cep_is_dynamic_var(char *cmd, char **outLine) {
  int isSetVar = 0; // default
  cep_var_info *curVar_p = NULL;
  int numMatches=99;
  //
  //PRNT("%s: <%s>\n",__FUNCTION__,cmd);
  if (cmd[0] == '$') {
    // search the var list
    curVar_p = cep_search_var(&(cmd[1]), &numMatches);
    if ((curVar_p != NULL) && (numMatches == 1)) { // found it
      isSetVar = 1;
      *outLine = curVar_p->valStr;
    }
  }
  return isSetVar;
}


//
// ====================================
// Break the input line to tokens
// ====================================
//
#define DOUBLE_QUOTE_CHAR 0x22
void cep_ProcessTokens(char *inLine, int save2history) 
{
  char *curL, *tmp = NULL;
  char *tk, *new_tk, *p;
  char *comd;
  char *savetoken;
  int i; // , neg = 0;
  //char saveLine[256];
  //int hisId;
  curL = inLine;
  // save away history before curL get mess up
  //strcpy(saveLine, curL);
  // reset
  __gTokenCount = 0;
  // get first one = command
  __gTokens[__gTokenCount].isNum = 0;
  strcpy(__gTokens[__gTokenCount].ascii,"\n");
  //
  comd = (char *)strtok_r(curL, " \t\n", &savetoken);
  if (comd != NULL) {
    //
    // SPECIAL!!!
    // check if first character is a "!" => execute based on history command
    //
    //if (cep_is_history_cmd(comd,&tmp)) {
    if (cep_is_history_cmd(cep_get_inputLine_p(),&tmp)) { // use saveLine instead
      if (VERBOSE1()) {
	PRNT("%s: <%s> -> <%s>\n",__FUNCTION__,comd,tmp);
      }
      #if 1
      // put that back to inLine so we won't screw up the history
      strcpy(inLine,tmp);
      // resave
      curL = inLine;
      strcpy(cep_get_inputLine_p(), curL);
      // re-process the first token
      comd = (char *)strtok_r(curL, " \t\n", &savetoken);
      #endif
    }
    //
    // end of SPECIAL
    //
    cep_first_args_pos = strlen(comd); // next arg 
    strcpy(__gTokens[__gTokenCount++].ascii,comd);
    // the rest
    do {
      tk = strtok_r(NULL, " \t\n", &savetoken);
      if (tk != NULL) {
#if 1
	// replace the token if it start with $xxxx
	if (cep_is_dynamic_var(tk, &new_tk)) {
	  tk = new_tk;
	}
#endif
	strcpy(__gTokens[__gTokenCount].ascii,tk);
	if (tk[0] == (char)DOUBLE_QUOTE_CHAR) { 	// start of double quote?
	  // get the rest of the string in savetoken
	  if (VERBOSE3()) {
	    PRNT("See start of Double quote as <%s>\n",__gTokens[__gTokenCount].ascii);
	  }
	  p = strstr(savetoken,"\"");
	  if (p != NULL) { // found it
	    // recopy
	    strcpy(__gTokens[__gTokenCount].ascii,&(tk[1])); // skip the start of double quote
	    // append the rest of string until "
	    p[0] = '\0'; // remove and mark end of string
	    strcat(__gTokens[__gTokenCount].ascii," "); // savetoken already point pass it
	    strcat(__gTokens[__gTokenCount].ascii,savetoken); // start from savetoken back 1 char
	    savetoken = p + 1; // move to the next
	  } // else string has no space so double quote is first and last already in tk
	  else {
	    strcpy(__gTokens[__gTokenCount].ascii,&(tk[1])); // skip the start of double quote
	    p = strstr(__gTokens[__gTokenCount].ascii,"\"");
	    if (p != NULL) { // found it	
	      p[0] = '\0'; // remove and mark end of string
	    }    
	  }
	  if (VERBOSE3()) {
	    PRNT("Final Double quote as <%s>\n",__gTokens[__gTokenCount].ascii);
	  }
	}
	// check if it is number 
	__gTokens[__gTokenCount].isNum = 1; // assume numbers
	for (i=0;i<(int)strlen(tk);i++) { 
	  if ((i==0) && (tk[0] == '-')) { 
//	    neg = 1; 
	  }
	  else if ((tk[i] != 'x') && !isxdigit(tk[i])) { // can be negative too
	    __gTokens[__gTokenCount].isNum = 0;
	    break;
	  }
	}
	// get the value anyway
	__gTokens[__gTokenCount].value = strtoll(tk, (char **)NULL, 0);
	__gTokenCount++;
      } // if
    } while (tk != NULL);
  }
  //
  // save to history list
  //
  if ((__gTokenCount > 0) && save2history) {
    strcpy(_gHistoryStr[_gHistoryPtr].ascii,cep_get_inputLine_p());
    _gHistoryStr[_gHistoryPtr].cmdId = _gHistoryCnt;
    // next
    _gHistoryCurPtr = _gHistoryPtr;
    _gHistoryPtr = (_gHistoryPtr + 1) % CEP_MAX_HISTORY; // wrap around
    _gHistoryCnt++;
  }
  //PRNT("Input:<%s> __gTokenCount=%d\n",inLine,__gTokenCount);
}

char *cep_get_curhistory_line(void) 
{
  return _gHistoryStr[_gHistoryCurPtr].ascii;
}

//
// ====================================
// return number if argument is 0 - 9
//        0xFFFFFFFF if argument is *
//        0 for everything else
// ====================================
//
u_int32_t cep_token_get_id(int token_id, int *isWildcard)
{
  cep_token_t *tmp_p;
  *isWildcard = 0; // assume single
  tmp_p = get_token(token_id);
  if (tmp_p->isNum) {
    //PRNT("%s: isWildcard=%d good=0x%08x\n",__FUNCTION__,*isWildcard,tmp_p->value);
    return tmp_p->value;
  }
  else if (strncmp(tmp_p->ascii,"*",1) == 0) {
    *isWildcard = 1;
    //PRNT("%s: isWildcard=%d data=0xFFFFFFFF\n",__FUNCTION__,*isWildcard);
    return 0xFFFFFFFF;
  } else { // might be range?? like 1-5 FIXME!!!!
    //PRNT("%s: isWildcard=%d data=empty\n",__FUNCTION__,*isWildcard);
    return 0;
  }
}


//
// ====================================
// Get the command from std (or file) and break it into tokens
// ====================================
//
#ifdef _MICROKERNEL
extern int cep_get_a_line (char *prompt, char *line_out);
#endif



void cep_GetCommand(const char *prompt, int inDebugger) {
//  int errCnt = 0;
  char inputLine[256];

  //
  // wait for the command
  inputLine[0] = '\0';
  do {
    thread_yield();
    // go get a line from command script
    if (cep_script_inProgress() && !inDebugger) {
      (void) cep_get_a_line_from_script(inputLine);
      if (!cep_script_line_is_empty(inputLine)) { // print it
	PRNT("CmdLno%d> %s\n",cep_get_lno(),inputLine);
      }
    }
    // just in case script is empty
    if (!cep_script_inProgress() || inDebugger) {
      //      PRNT("%s ",prompt);
      printf("%s ",prompt);      // use printf here since multi-thread
      fgets(inputLine,256, stdin);
      // rmeove newline?? if only xterm (Linux window)
      if (GET_VAR_VALUE(xterm) && ((inputLine[strlen(inputLine)-1] == '\n') || (inputLine[strlen(inputLine)-1] == '\r'))) {
	inputLine[strlen(inputLine)-1] = 0;
      }
    }
    // save this away
    strcpy(cep_inputLine, inputLine);
    // process it and execute
    if (!cep_script_line_is_empty(inputLine)) {
      //PRNT("inputLine=<%s> len=%d c=0x%x\n",inputLine,strlen(inputLine),(int)inputLine[0]);
      cep_ProcessTokens(inputLine,1);
    } else {
      cep_set_token_count(0);
    }
  } while (cep_get_token_count() == 0);

}

void cep_GetPrompt(void) {
  char inputLine[256];
  //
  // wait for the command
  //
  if (GET_VAR_VALUE(prompt)) {
#ifdef _MICROKERNEL
    (void)cep_get_a_line("Press return to conitnue... ",inputLine);
#else
    PRNT("\nPress return to conitnue...");
    fgets(inputLine,256, stdin);
#endif
  }
}
//
// =======================================
// to support break point
// =======================================
//
typedef struct brkcomd_ cep_brkCmd;
typedef struct brkcomd_ {
  char ascii[256]; // save command line when break point is hit
  cep_brkCmd *next;
} cep_brkCmd_s;

cep_brkCmd *cep_brkCmd_headPtr = NULL; // empty to begin with

// save away the last command line when breakpoint is hit
void cep_brk_push_command(void) { 
  cep_brkCmd *new_p;
  new_p = (cep_brkCmd *)malloc(sizeof(cep_brkCmd_s));
  if (new_p == NULL) {
    PRNT("%s: ERROR Can't alloc memory for breakpoint command saving\n",__FUNCTION__);
    return;
  }
  // save away the command
  strcpy(new_p->ascii,_gHistoryStr[_gHistoryCurPtr].ascii);
  new_p->next = cep_brkCmd_headPtr;
  cep_brkCmd_headPtr = new_p;
  PRNT("%s: Saving: <%s>\n",__FUNCTION__,cep_brkCmd_headPtr->ascii);
}

// restore away the last command line when about to return after breakpoint 
void cep_brk_pop_command(void) {
  cep_brkCmd *tmp;
  if (cep_brkCmd_headPtr != NULL) {
    PRNT("%s: Restoring: <%s>\n",__FUNCTION__,cep_brkCmd_headPtr->ascii);    
    cep_ProcessTokens(cep_brkCmd_headPtr->ascii, 0);
    tmp = cep_brkCmd_headPtr;
    cep_brkCmd_headPtr = cep_brkCmd_headPtr->next;
    free(tmp);
  }
}
