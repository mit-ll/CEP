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

#include <string.h>
#include <ctype.h>

#ifdef _MICROKERNEL
#include "malloc.h"
#endif

#include "simdiag_global.h"
#include "cep_io.h"
#include "cep_script.h"
#include "cep_vars.h"
//
//
//
#define CEP_MAX_SCRIPT_LEVEL 32
#define CEP_SCRIPT_MAXSIZE   (1<<20)
//
//
// structure for script file (do not have to expose this) OOP/C++ style
//
typedef struct cep_script_s cep_script_info;
typedef struct cep_script_s {
  char fileName[64];
  int level;  // point to the level where this script belong
  //
  // dynamic stuffs
  //
  int bufSize;
  int pos; // point to current char in buf
  int lineno;
  char *buf; // point to the buffer where the file is
  int noDelete; // if set, do not free the buf (might come from macro)
  //
  cep_script_info *mom; // parent for travel up
} cep_script_t;

//
//
//
typedef int (*cep_read_file_into_buffer_fn_t)(char *, char **, int *, int);
cep_read_file_into_buffer_fn_t read_sript_file_into_buffer = NULL;

//
// some globals
//
cep_script_t *_cep_script_curptr = NULL;
//
// some static
//
static cep_script_t *get_curptr(void)
{
  return _cep_script_curptr;
}
static void set_curptr(cep_script_t *new_p)
{
  _cep_script_curptr = new_p;
}
static void cep_script_line_cleanup(char *str)
{
  char *p;
  //
  // check for comments "//" or "#", if found remove it
  //
  p = strstr(str,"//");
  if (p != NULL) {
    p[0] = '\0'; // remove it
  }
  // or comment with print
  p = strstr(str,"#");
  if (p != NULL) {
    PRNT("%s\n",&(p[1])); // skip the #
    p[0] = '\0'; // remove it but print it out first
  }
}

int cep_get_lno(void)
{
  if (get_curptr() != NULL) {
    return get_curptr()->lineno;
  } else {
    return -1;
  }
}

//
// return 1 if the line is empty
//
int cep_script_line_is_empty(char *str)
{
  size_t i;
  for (i=0;i<strlen(str);i++) {
    //if (!isspace(str[i])) return 0;
    if ((str[i] != ' ') || (str[i] != '\t') || (str[i] != '\r') || (str[i] != '\n')) return 0;
  }
  return 1;
}
//
// how to read a file
//
void cep_file_read_func_init(void *func)
{
  read_sript_file_into_buffer = (cep_read_file_into_buffer_fn_t)func;
}

//
// Return 1 if script in progress
//
int cep_script_inProgress(void)
{
  return (get_curptr() != NULL) ? 1 : 0;
}
//
// Print current info
//
void cep_script_print_info(void)
{
  if (get_curptr() != NULL) {
    PRNT("%s: script=%s level=%d size=%d(bytes) lineno=%d\n",__FUNCTION__,
	 get_curptr()->fileName,get_curptr()->level,get_curptr()->bufSize,get_curptr()->lineno);
  }
}

//
// Open the script and start executing if no error else return 1
//
int __cep_cur_scriptLoop =1;
int cep_execute_script(char *fileName)
{
  int errCnt =  0;
  cep_script_t *ptr;
  char *fileBuffer = NULL;
  int fileSize;
  if (VERBOSE2()) { PRNT("%s: fileName=%s\n",__FUNCTION__,fileName); }
  //
  // check if ok to proceed
  //
  if (read_sript_file_into_buffer == NULL) {
    PRNT("%s: ERROR: read_sript_file_into_buffer MUST be defined\n",__FUNCTION__);
    return 1;
  }
  // allocate struct for it and fill in the info
  ptr = (cep_script_t *)malloc(sizeof(cep_script_t));
  if (ptr == NULL) {
    PRNT("%s: ERROR: Can't malloc pointer for script %s\n",__FUNCTION__,fileName);
    return 1;
  }
  if (get_curptr() != NULL) {
    ptr->level = get_curptr()->level + 1; // next
  } else {
    ptr->level = 1;
  }
  if (ptr->level >= CEP_MAX_SCRIPT_LEVEL) { // too much , get out
    PRNT("%s: ERROR: Too many netted script level. max is %d\n",__FUNCTION__,CEP_MAX_SCRIPT_LEVEL);
    free(ptr);
    return 1;
  }
  //
  strcpy(ptr->fileName,fileName);
  // link to mom (which might be NULL) so I can go back when I am done with this one
  ptr->mom      = get_curptr();
  // reset the dynamic
  ptr->pos      = 0;
  ptr->lineno   = 0;
  ptr->buf      = NULL;
  ptr->bufSize  = 0;
  ptr->noDelete = 0; // default
  //
  // first open it and read in buffer
  //
  errCnt = (*read_sript_file_into_buffer)(fileName, &fileBuffer, &fileSize, CEP_SCRIPT_MAXSIZE);
  if (errCnt) {
    PRNT("%s: ERROR: Can't read scriptFile %s into buffer.\n",__FUNCTION__,fileName);
    free(ptr);
    return 1;
  }
  ptr->bufSize = fileSize;
  ptr->buf     = fileBuffer;
  // make this current
  set_curptr(ptr);
  // print info about this script
  if (VERBOSE1()) { 
    cep_script_print_info();
  }
  __cep_cur_scriptLoop = GET_VAR_VALUE(mloop);
  // done
  return errCnt;
}

int cep_execute_macro(char *macroName, char **buf)
{
  int errCnt =  0;
  cep_script_t *ptr;
  if (VERBOSE2()) { PRNT("%s: macroName=%s\n",__FUNCTION__,macroName); }
  //
  // allocate struct for it and fill in the info
  ptr = (cep_script_t *)malloc(sizeof(cep_script_t));
  if (ptr == NULL) {
    PRNT("%s: ERROR: Can't malloc pointer for script %s\n",__FUNCTION__,macroName);
    return 1;
  }
  if (get_curptr() != NULL) {
    ptr->level = get_curptr()->level + 1; // next
  } else {
    ptr->level = 1;
  }
  if (ptr->level >= CEP_MAX_SCRIPT_LEVEL) { // too much , get out
    PRNT("%s: ERROR: Too many netted script level. max is %d\n",__FUNCTION__,CEP_MAX_SCRIPT_LEVEL);
    free(ptr);
    return 1;
  }
  //
  strcpy(ptr->fileName,macroName);
  // link to mom (which might be NULL) so I can go back when I am done with this one
  ptr->mom      = get_curptr();
  // reset the dynamic
  ptr->pos      = 0;
  ptr->lineno   = 0;
  ptr->buf      = *buf; // link to macro command list
  ptr->bufSize  = 0;
  ptr->noDelete = 1; // DO NOT Delete buffer when done
  ptr->bufSize  = strlen(*buf);
  // make this current
  set_curptr(ptr);
  // print info about this script
  if (VERBOSE1()) { 
    cep_script_print_info();
  }
  __cep_cur_scriptLoop = GET_VAR_VALUE(mloop);
  // done
  return errCnt;
}

//
// Cleanup the current and go back to Mom if there is one
//
static void cep_script_cleanup(void)
{
  cep_script_t *mom;
  if (get_curptr() != NULL) {
    PRNT("\t Closing script file %s level=%d\n",get_curptr()->fileName,get_curptr()->level);
    mom = get_curptr()->mom;
    if (!get_curptr()->noDelete) {
      free(get_curptr()->buf);  // the buffer first
    }
    free(get_curptr());       // then itself
    set_curptr(mom);
  }
}
void cep_script_cleanup_all(void)
{
  while (get_curptr() != NULL) { // go thru until empty
    cep_script_cleanup(); // cleanup current and point to next
  }
}
//
// Get a line from active, remove comment and return it if there is one, else clean this up and 
// get back to mom and repeat the process until either done or a line is obtained
//
int cep_get_a_line_from_script(char *lineOut)
{
  int errCnt = 0;
  int i = 0;
  //
  lineOut[0] = '\0'; // clear the line
  //
  // no more!!
  //
  if (get_curptr()->pos >= get_curptr()->bufSize) { // clean this up and back to Mom
    // see if we are in a loop (only if it is level 1 (top level)
    if (get_curptr()->level == 1) {
      __cep_cur_scriptLoop--;
      if (__cep_cur_scriptLoop > 0) { // more to go
	PRNT("\t Resetting script file %s level=%d and rerun. loop2go=%d\n",
	     get_curptr()->fileName,get_curptr()->level,__cep_cur_scriptLoop);
	get_curptr()->pos      = 0;
	get_curptr()->lineno   = 0;
      } else {
	cep_script_cleanup();
      }
    } else {
      // close if not top level (no loop support in lower level)
      cep_script_cleanup();
    }
  }
  if (cep_script_inProgress()) { // not done yet
    i = 0;
    do {
      if ((get_curptr()->buf[get_curptr()->pos] != '\n') && 
	  (get_curptr()->buf[get_curptr()->pos] != '\r')) { // put it in tmp str
	lineOut[i] = get_curptr()->buf[get_curptr()->pos]; // continue
	i++;
	get_curptr()->pos += 1;
      } else {
	// move past this newline char
	get_curptr()->pos += 1;
	get_curptr()->lineno++;
	lineOut[i] = '\0'; // terminate the line
	// remove comment if any
	cep_script_line_cleanup(lineOut);
	if (cep_script_line_is_empty(lineOut)) {  // may be just comment line?
	  i = 0; // continue to next line
	} else {
	  return 0; // done
	}
      } 
    } while ((get_curptr()->pos < get_curptr()->bufSize));
  }
  //
  // must be last line
  //
  if (i > 0) {
    // move past this newline char
    get_curptr()->pos += 1;
    get_curptr()->lineno++;
    lineOut[i] = '\0'; // terminate the line
    // remove comment and check for empty line
    cep_script_line_cleanup(lineOut);
    return 0; // done
  } 
  //
  return errCnt;
}

int cep_read_file_into_buffer(char *fileName, char **fileBuf, int *fileSize, int maxSize)
{
  int errCnt =  0;
  if (VERBOSE2()) { PRNT("%s: fileName=%s\n",__FUNCTION__,fileName); }
  //
  // check if ok to proceed
  //
  if (read_sript_file_into_buffer == NULL) {
    PRNT("%s: ERROR: read_sript_file_into_buffer MUST be defined\n",__FUNCTION__);
    return 1;
  }
  //
  // first open it and read in buffer
  //
  errCnt = (*read_sript_file_into_buffer)(fileName, fileBuf, fileSize, maxSize);
  if (errCnt) {
    PRNT("%s: ERROR: Can't read file %s into buffer.\n",__FUNCTION__,fileName);
    free(*fileBuf);
    return 1;
  }
  // done
  return errCnt;
}
