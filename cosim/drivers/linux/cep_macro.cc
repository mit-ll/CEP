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
#include <ctype.h>

#ifdef _MICROKERNEL
#include "malloc.h"
#endif

#include "simdiag_global.h"
#include "cep_io.h"
#include "cep_token.h"
#include "cep_script.h"
#include "cep_macro.h"
//
//
//
#define CEP_MACRO_MAXSIZE   (1<<16)
//
//
// structure for macro file (do not have to expose this) OOP/C++ style
//
typedef struct cep_macro_s cep_macro_info;
typedef struct cep_macro_s {
  char macroName[64];
  int cmdCnt;
  int active;
  char *buf; // point to the buffer where all the command are stored.
  //
  cep_macro_info *next; // 
} cep_macro_t;

//
// some globals
//
cep_macro_t *_cep_macroPtr = NULL;
int _cep_macro_active = 0;
//
// Return 1 if macro in progress
//
static void set_macroPtr(cep_macro_t *new_p) 
{
  // make it the top
  new_p->next = _cep_macroPtr;
  _cep_macroPtr = new_p;
  _cep_macro_active = 1;
}

//
// Open the macro and start executing if no error else return 1
//
static int cep_macro_start(char *macroName)
{
  int errCnt =  0;
  cep_macro_t *ptr;
  if (VERBOSE2()) { PRNT("%s: macroName=%s\n",__FUNCTION__,macroName); }
  //
  // allocate struct for it and fill in the info
  ptr = (cep_macro_t *)malloc(sizeof(cep_macro_t));
  if (ptr == NULL) {
    PRNT("%s: ERROR: Can't malloc pointer for macro %s\n",__FUNCTION__,macroName);
    return 1;
  }
  //
  strcpy(ptr->macroName,macroName);
  ptr->next     = NULL;
  ptr->cmdCnt   = 0;
  ptr->active   = 1;
  ptr->buf      = (char *)malloc(CEP_MACRO_MAXSIZE);
  if (ptr->buf == NULL) {
    PRNT("%s: ERROR: Can't malloc pointer for macro'sbuf %s\n",__FUNCTION__,macroName);
    free(ptr);
    return 1;
  }
  ptr->buf[0]   = '\0'; // make it empty
  // make this current
  set_macroPtr(ptr);
  // done
  return errCnt;
}

static void cep_macro_stop(void) {
  _cep_macro_active = 0;
}

static void cep_macro_delete(char *macroName)
{
  cep_macro_t *mom = NULL, *cur, *tmp;
  cur = _cep_macroPtr ;
  while (cur != NULL) {
    if (strncasecmp(cur->macroName,macroName, strlen(macroName)) == 0) { // found
      PRNT("\t Deleting macro %s\n",cur->macroName);
      tmp = cur;
      if (mom == NULL) { // top
	cur = tmp->next;
      } else {
	mom->next = tmp->next; 
      }
      free(tmp->buf);
      free(tmp);
      break;
    } else {
      mom = cur;
      cur = cur->next;
    }
  }
}

static void cep_macro_list(void)
{
  cep_macro_t *cur = _cep_macroPtr;
  if (cur != NULL) {
    PRNT("ACTIVE MACRO LIST:\n");
    while (cur != NULL) {
      PRNT("\t %s\n",cur->macroName);
      cur = cur->next;
    }
  } else {
    PRNT("Sorry: There is no active macro\n");
  }
}

static void cep_macro_dump(char *macroName)
{
  cep_macro_t *cur;
  cur = _cep_macroPtr ;
  while (cur != NULL) {
    if (strncasecmp(cur->macroName,macroName, strlen(macroName)) == 0) { // found
      PRNT("Dumping Macro %s:\n",cur->macroName);
      PRNT("%s\n",cur->buf);
      break;
    } else {
      cur = cur->next;
    }
  }
}

static int cep_macro_run(char *macroName)
{
  int errCnt = 0;
  cep_macro_t *cur;
  cur = _cep_macroPtr ;
  while (cur != NULL) {
    if (strncasecmp(cur->macroName,macroName, strlen(macroName)) == 0) { // found
      // call execute function
      errCnt = cep_execute_macro(cur->macroName, &(cur->buf));
      break;
    } else {
      cur = cur->next;
    }
  }
  return errCnt;
}


int cep_macro_inProgress(void)
{
  return _cep_macro_active;
}
// add new command to macro
void cep_macro_addCmd(char *cmd) {
  char tmp[256];
  sprintf(tmp,"\t%s\n",cmd); // add a tab and newline
  if (_cep_macroPtr != NULL) {
    strcat(_cep_macroPtr->buf,tmp);
    _cep_macroPtr->cmdCnt++;
  }
}
//
// ===========================
// Macro Command (recording)
// ===========================
//
int cep_process_macro(void)
{
  int errCnt = 0;
  char macroName[128];
  //
  int len = strlen(get_token(1)->ascii);
  if (strncasecmp(get_token(1)->ascii,"start",len) == 0) {
    errCnt = cep_macro_start(get_token(2)->ascii);
  } else if (strncasecmp(get_token(1)->ascii,"stop",len) == 0) {
    cep_macro_stop();
  } else if (strncasecmp(get_token(1)->ascii,"delete",len) == 0) {
    cep_macro_delete(get_token(2)->ascii);
  } else if (strncasecmp(get_token(1)->ascii,"list",len) == 0) {
    cep_macro_list();
  } else if (strncasecmp(get_token(1)->ascii,"dump",len) == 0) {
    cep_macro_dump(get_token(2)->ascii);
  } else if (strncasecmp(get_token(1)->ascii,"run",len) == 0) {
    strcpy(macroName,get_token(2)->ascii);
    errCnt = cep_macro_run(macroName);
  } else {
    PRNT("%s: %s is not supported\n",__FUNCTION__,get_token(1)->ascii);
    errCnt++;
  }
  return errCnt;
}

