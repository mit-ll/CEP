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

//
// ======================================
// handle acc, dump, menu command
// ======================================
//
#include <string.h>
#include "simdiag_global.h"
#include "cep_io.h"
#include "cep_cmds.h"
#include "cep_vars.h"
#include "cep_token.h"
#include "cep_acc.h"
#include "cep_exports.h"
//
// global array to hold acciable name
//
#define __CEP_ACC_MAX  256 // for now
int __cep_acc_curCount = 0; // point to first one
cep_acc_info __cep_acc_array[__CEP_ACC_MAX];
// for sorted list
cep_acc_info *__cep_acc_head_p = NULL;
//
// to support <deviceName>-<id> where <id> = integer or * to indicate all instances
//
u_int64_t __cep_instance_select_mask = 0; // 0 : no select, 1=id0, 2=id1, *=0xFFFFFFFF

//
// check the device for <deviceName>-<id> if found strip the -<id> and 
// put <id> in  __cep_instance_select_mask  as *mask*
static int cep_strip_device_instance_if_found(char *accName)
{
  char *p;
  int devId;
  //
  p = strstr(accName,"-"); // check if there is a '-' in device name
  if (p == NULL) { // nope
    __cep_instance_select_mask = 0;
    return 0;
  } 
  // let see what that is
  if (p[1] == '*') { // is a wild card
    __cep_instance_select_mask = (u_int64_t)(-1);
    p[0] = '\0'; // strip off the -*
    return 1;
  }
  devId = (int)strtoll(p+1, (char **)NULL, 10);
  __cep_instance_select_mask = (u_int64_t)1 << devId;
  p[0] = '\0'; // strip off the -* or -<id>
  return 1;
}
//
void cep_set_device_mask(u_int64_t def)
{
  __cep_instance_select_mask = (u_int64_t)def;
}

u_int64_t cep_get_device_mask_if_any(int def)
{
  return (__cep_instance_select_mask != 0) ? (u_int64_t)__cep_instance_select_mask : ((def >=  0) ? (1 << def) : -1);
}

u_int64_t cep_get_device_mask64_if_any(u_int64_t def)
{
  if (VERBOSE2()) {
    PRNT("%s: __cep_instance_select_mask=0x%016lx\n",__FUNCTION__,(u_int64_t)__cep_instance_select_mask);
  }
  return (__cep_instance_select_mask != 0) ? (u_int64_t)__cep_instance_select_mask : ((u_int64_t)1 << def);
}


void cep_search_acc_and_printAll(char *accName)
{
  int i=0;
  int l0;
  l0 = strlen(accName);
  PRNT("Possible matching ACC(s)\n");
  while (i < __cep_acc_curCount) {
    if (strncasecmp(accName, __cep_acc_array[i].name, 
		    //MIN(l0,strlen(__cep_acc_array[i].name))) == 0) { // found it
		    l0) == 0) {
      cep_acc_print(&__cep_acc_array[i]);
    }
    i++;
  }
}

//
// ==========================
// return NULL if not found
// ==========================
//
cep_acc_info *cep_search_acc(char *accName, int *matchFound)
{
  int i=0;
  int saveP=0;
  int l0;
  //
  *matchFound = 0;
  // but first strip the instance ID if founf
  (void)cep_strip_device_instance_if_found(accName);
  //
  // search entire thing to get number of possible matches
  l0 = strlen(accName);
  while (i < __cep_acc_curCount) {
    if (strncasecmp(accName, __cep_acc_array[i].name, 
		    //MIN(l0,strlen(__cep_acc_array[i].name))) == 0) { // found it
		    l0) == 0) {
      saveP = i;
      *matchFound = *matchFound + 1;
    }
    i++;
  }
  if (*matchFound == 1) { // exactly 1
    return &__cep_acc_array[saveP];
  } else if (*matchFound > 1) {
    if (GET_VAR_VALUE(verbose)) {
      PRNT("Warning: There are %d matches found for access device: %s\n",*matchFound,accName);
    }
    return &__cep_acc_array[saveP];
  } else {
    //PRNT("Warning: Could not find command %s\n",accName);
    return NULL;
  }
}

//
// ====================================
// to add a test to array
// ====================================
//
cep_acc_info *cep_acc_add2idx(int idx, const char *name, 
				      cep_acc_vect_t write_func_t, 
				      cep_acc_vect_t read_func_t, 
				      cep_acc_vect_t dump_func_t, 				      
				      cep_acc_vect_t reset_func_t, 				      
				      cep_acc_vect_t program_func_t, 				      
				      const char *help)
{
  cep_acc_info *tmp_p = NULL;
  int saveVerbose;
#ifdef DEBUG
  PRNT("%s: i=%d name=%s\n",
       __FUNCTION__,idx,name);
#endif
  if (idx >= __CEP_ACC_MAX) {
    LOGE("%s: ERROR Can't add any more device to access list. idx=%d exceeds its limit of %d\n",
	 __FUNCTION__,idx,__CEP_ACC_MAX);
    return tmp_p;
  }
  // check verbose and make sure it is zero 
  saveVerbose = GET_VAR_VALUE(verbose);
  if (GET_VAR_VALUE(verbose) != 0) {
    SET_VAR_VALUE(verbose,0);
    //PRNT("==== Verbose level=%d is not zero!!! Temporary change to zero due to long programing times..\n",saveVerbose);
  }
  // add to the current one and move to next
  strcpy(__cep_acc_array[idx].name, name);
  strcpy(__cep_acc_array[idx].help, help);
  __cep_acc_array[idx].id          = idx;
  __cep_acc_array[idx].valid       = 1; // mark as good
  __cep_acc_array[idx].write_vect  = write_func_t;
  __cep_acc_array[idx].read_vect   = read_func_t;
  __cep_acc_array[idx].dump_vect   = dump_func_t;
  __cep_acc_array[idx].reset_vect  = reset_func_t;
  __cep_acc_array[idx].power_vect  = NULL; // to be added individually
  __cep_acc_array[idx].program_vect= program_func_t;
  __cep_acc_array[idx].next        = NULL;
  // default are NULL, will be added later
  __cep_acc_array[idx].memwr_vect  = NULL;
  __cep_acc_array[idx].memrd_vect  = NULL;
  // default are NULL, will be added later
  __cep_acc_array[idx].bringup_vect  = NULL;
  __cep_acc_array[idx].reload_vect   = NULL;
  __cep_acc_array[idx].eyescan_vect  = NULL;
  // next
  tmp_p = &__cep_acc_array[idx];
  //
  SET_VAR_VALUE(verbose,saveVerbose);
  return tmp_p;
}

void cep_acc_delete(int idx)
{
  strcpy(__cep_acc_array[idx].name, "UNKNOWN");
  __cep_acc_array[idx].valid       = 0;
}

cep_acc_info *cep_acc_addmem2idx(int idx, cep_acc_vect_t memwr_func_t, cep_acc_vect_t memrd_func_t)

{
  cep_acc_info *tmp_p = NULL;
#ifdef DEBUG
  PRNT("%s: i=%d name=%s\n",
       __FUNCTION__,idx,name);
#endif
  if (idx >= __CEP_ACC_MAX) {
    LOGE("%s: ERROR Can't add any more device to access list. idx=%d exceeds its limit of %d\n",
	 __FUNCTION__,idx,__CEP_ACC_MAX);
    return tmp_p;
  }
  __cep_acc_array[idx].memwr_vect  = memwr_func_t;
  __cep_acc_array[idx].memrd_vect  = memrd_func_t;
  // next
  tmp_p = &__cep_acc_array[idx];
  //
  return tmp_p;
}

void cep_acc_print(cep_acc_info *acc_p)
{
  char tmp[64];
  int i;
  if (acc_p == NULL) {
    PRNT("%s; ERROR NULL acc_p\n",__FUNCTION__);
    return;
  }
  strcpy(tmp,acc_p->name);
  for (i=strlen(tmp);i<MAX_ZERO_FILL;i++) {
    strcat(tmp," ");
  }
  tmp[MAX_ZERO_FILL] = '\0';
  PRNT("%s OptionalArgs: %s\n",tmp,acc_p->help);
}
int cep_acc_dump_all(void) 
{
  // sorted
  cep_acc_info *cur_p = __cep_acc_head_p;
  PRNT("\n==== Available Devices for Read/Write === \n");
  while (cur_p != NULL) {
    cep_acc_print(cur_p);
    cur_p = cur_p->next;
  }
  return 0;
}

//
// ===============================
// be carefull: no check!!!
// ===============================
//
cep_acc_info *cep_get_acc(int idx)
{
  return &__cep_acc_array[idx];
}
char *cep_get_acc_name(int idx) 
{
  return (cep_get_acc(idx))->name;
}
char *cep_get_acc_help(int idx) 
{
  return (cep_get_acc(idx))->help;
}
int cep_get_max_acc_count(void)
{
  return __cep_acc_curCount;
}
void cep_set_max_acc_count(int value)
{
  __cep_acc_curCount = value;
}
//
// ================================================================
// Add access methods from here
// ================================================================
//
// ===================================
// display test menu
// ===================================
//
#ifdef _MICROKERNEL
extern boolean e500_protect_section ( boolean enable );
#endif



//
// ====================================
// attach/detach
// ====================================
attach_handle_t *__attach_handle = NULL;

void cep_add_attach_ops(attach_handle_t *newOp) {
  // add to the top
  newOp->next = __attach_handle;
  __attach_handle = newOp;
}

int cep_acc_attach(void)
{
  int errCnt = 0;
  char *devName	      = get_token(1)->ascii;
  int id 	      = get_token(2)->value;
  attach_handle_t *ptr;
  int found = 0;
  //
  // Attach
  //
  // search for match
  ptr = __attach_handle;
  while ((ptr != NULL) && !found) {
    if ((ptr->id == id) && (strncasecmp(ptr->name,devName,MIN(3,strlen(devName))) == 0)) { // found it
      found = 1;
      break;
    }
    ptr = ptr->next;
  }
  if (found && (ptr->device_attach != NULL)) {
    if (ptr->attach_done == 0) {
      PRNT("ATTACH: devName%d=%s\n",id,ptr->name);
      errCnt += (*ptr->device_attach)(id);
      ptr->attach_done = (errCnt == 0) ? 1 : 0;
    }
  } else {
    PRNT("Sorry: There is no such Attach operation defined for devName=%s\n",devName);
    errCnt++;
  }
  return errCnt;
}

int cep_acc_detach(void)
{
  int errCnt = 0;
  char *devName	      = get_token(1)->ascii;
  int id 	      = get_token(2)->value;
  attach_handle_t *ptr;
  int found = 0;
  //
  // Attach
  //
  // search for match
  ptr = __attach_handle;
  while ((ptr != NULL) && !found) {
    if ((ptr->id == id) && (strncasecmp(ptr->name,devName,MIN(3,strlen(devName))) == 0)) { // found it
      found = 1;
      break;
    }
    ptr = ptr->next;
  }
  if (found && (ptr->device_detach != NULL)) {
    if (ptr->attach_done == 1) { // only if active
      PRNT("DETACH: devName%d=%s\n",id,ptr->name);
      errCnt += (*ptr->device_detach)(id);
      if (errCnt == 0) { ptr->attach_done = 0; }
    }
  } else {
    PRNT("Sorry: There is no such Detach operation defined for devName=%s\n",devName);
    errCnt++;
  }
  return errCnt;
}

//
// ====================================
// CONFIG
// ====================================
config_handle_t *__config_handle = NULL;

void cep_add_config_ops(config_handle_t *newOp) {
  // add to the top
  newOp->next = __config_handle;
  __config_handle = newOp;
}

int cep_acc_config(void)
{
  int errCnt = 0;
  char *devName	      = get_token(1)->ascii;
  config_handle_t *ptr;
  int found = 0;
  //
  // CONFIG
  //
  // search for match
  ptr = __config_handle;
  while ((ptr != NULL) && !found) {
    if (strncasecmp(ptr->name,devName,MIN(3,strlen(devName))) == 0) { // found it
      found = 1;
      break;
    }
    ptr = ptr->next;
  }
  if (found && (ptr->func != NULL)) {
    PRNT("CONFIG: devName=%s\n",ptr->name);
    (*ptr->func)();
  } else {
    PRNT("Sorry: There is no such CONFIG operation defined for devName=%s\n",devName);
    errCnt++;
  }
  return errCnt;
}

//
// ====================================
// return 1 if error
// ====================================
int cep_init_acc(void)
{
  int errCnt = 0;
  cep_acc_info *tmp_p = NULL;
  int i;
  //
  for (i=0;i<__CEP_ACC_MAX;i++) {
#if 1
    memset(&(__cep_acc_array[i]), 0x0, sizeof(cep_acc_s_t));
    strcpy(__cep_acc_array[i].name,"__UNKNOWN__");
#else
    strcpy(__cep_acc_array[i].name,"__UNKNOWN__");
    __cep_acc_array[i].valid        = 0; // invalid
    __cep_acc_array[i].write_vect   = NULL;
    __cep_acc_array[i].read_vect    = NULL;
    __cep_acc_array[i].dump_vect    = NULL;
    __cep_acc_array[i].reset_vect   = NULL;
    __cep_acc_array[i].power_vect   = NULL;
    __cep_acc_array[i].program_vect = NULL;
    __cep_acc_array[i].memwr_vect   = NULL;
    __cep_acc_array[i].memrd_vect   = NULL;
#endif
  }

#ifdef DEBUG
  PRNT("%s:\n",__FUNCTION__);
#endif
  //
  // ============================================
  // This is where you add global test to CEP
  // ============================================
  //
  //              Name        write_func          read_func           dump_func  reset_func  program_func Test Description
  CEP_ADD_ACC(ddr3,       ddr3_aWrite,     ddr3_aRead,      NULL,     NULL,       NULL,        "-- none -- (r/w/d)");  
  CEP_ADD_ACC(cep,        cep_aWrite,      cep_aRead,      NULL,     NULL,       NULL,        "-- none -- (r/w/d)");
  //
  // must do this 
  //
  cep_set_max_acc_count(ACC_zzMAX_CEP_COUNT);
  //
  //
  // =====================================
  return errCnt;
}

//
// MUST be  called to to sort the list
//
void cep_sort_acc(void)
{
  int i;
  int inserted =0;
  cep_acc_info *last_p, *cur_p;
  int debugMe = 0;
  // clean the link list
  for (i=0;i<__cep_acc_curCount;i++) { 
    __cep_acc_array[i].next = NULL;
  }
  //
  __cep_acc_head_p = &(__cep_acc_array[0]); // start from first one
  for (i=1;i<__cep_acc_curCount;i++) { // go down the list
    if (__cep_acc_array[i].valid) {
      //
      // add to the sorted list
      //
      last_p = NULL; 
      cur_p  = __cep_acc_head_p;
      if (debugMe) PRNT("Working on %s\n",__cep_acc_array[i].name);
      inserted = 0;
      while (cur_p != NULL) {
	if (strcasecmp(__cep_acc_array[i].name, cur_p->name) < 0) {  // i am his little brother!!
	  // add it in the middle
	  if (last_p == NULL) { // I am now the head
	    __cep_acc_head_p = &(__cep_acc_array[i]);
	  }
	  else {
	    last_p->next = &(__cep_acc_array[i]);
	}
	  __cep_acc_array[i].next = cur_p; // big bother
	inserted = 1; 
	if (debugMe) PRNT("head=%s: link %s -> %s\n",__cep_acc_head_p->name, last_p->name, __cep_acc_array[i].name);
	cur_p  =  NULL; // done
	} else { // next
	  last_p = cur_p;
	  cur_p  = cur_p->next;
	  if (debugMe) PRNT("last=%s cur=%s\n",last_p->name,cur_p->name);
	}
      } // while
      // must be at end of list
      if (!inserted) { // link to  last
	last_p->next = &(__cep_acc_array[i]);
	if (debugMe) PRNT("link to last=%s\n",last_p->name);
      }
    } // only if valid
  } // for
}

//
// ======================================
// Access Methods (read/write)
// ======================================
//
int cep_acc_read(void)
{
  int errCnt = 0;
  cep_acc_info *curAcc_p;
  int matchFound;
  int l;
  if (GET_VAR_VALUE(verbose)) {
    PRNT("%s:\n",__FUNCTION__);
  }
  // first check for proper argument types num <str>
  if (!get_token(1)->isNum) {
    LOGE("%s: ERROR: first argument %s MUST be a number\n",__FUNCTION__,get_token(1)->ascii);
    PRNT("Command syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_read));
    return 1;
  }
  // second argument must be the name of the device in the acc list so search for it
  //
  curAcc_p = cep_search_acc(get_token(2)->ascii,&matchFound);
  if (matchFound > 1) { // multiple?
    LOGE("%s: AMBIGUOUS: found %d (multiple) matches for device %s\n",__FUNCTION__,matchFound,get_token(2)->ascii);
    cep_search_acc_and_printAll(get_token(2)->ascii);
    return 1;
  } else if (matchFound != 1) {
    LOGE("%s: ERROR: Can't find a match for device %s\n",__FUNCTION__,get_token(2)->ascii);
    cep_cmd_print(cep_get_cmd(CMD_read));
    return 1;
  }
  //
  // now execute the read function if it is defined
  //
  if (curAcc_p->read_vect == NULL) {
    PRNT("%s: Sorry read function is not defined for this device %s\n",__FUNCTION__,curAcc_p->name);
    return 1;
  }
  // now run it thru a loop if it is 
  for (l=0;l<(int)GET_VAR_VALUE(loop);l++) {
    errCnt += (*curAcc_p->read_vect)();
    if (errCnt >= (int)GET_VAR_VALUE(maxErr)) break;
    // check if need to compare as well
    if (cep_get_search()) { 
      if ((cep_get_data(0) ^ cep_get_data(1)) & cep_get_data(2)) { // mis-compare
	PRNT("  ERROR: mis-compare actual=0x%016lx != expected=0x%016lx (mask=0x%016lx)\n",
	     cep_get_data(0), cep_get_data(1), cep_get_data(2));
	errCnt++;
	if (errCnt >= (int)GET_VAR_VALUE(maxErr)) return errCnt;
      }
    }
  }
  if (errCnt) {
    LOGE("%s: ERROR: Problem with executing READ command\n",__FUNCTION__);
    PRNT("Check Syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_read));
    return 1;
  }
  return errCnt;
}

int cep_acc_write(void)
{
  int errCnt = 0;
  cep_acc_info *curAcc_p;
  int matchFound;
  int l;
  if (GET_VAR_VALUE(verbose)) {
    PRNT("%s:\n",__FUNCTION__);
  }
  // first check for proper argument types <num><num> <str>
  if (!get_token(1)->isNum || !get_token(2)->isNum) {
    LOGE("%s: ERROR: first and second argument %s %s MUST be both numbers\n",__FUNCTION__,
	 get_token(1)->ascii, get_token(2)->ascii);
    PRNT("Command syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_write));
    return 1;
  }
  // third argument must be the name of the device in the acc list so search for it
  curAcc_p = cep_search_acc(get_token(3)->ascii,&matchFound);
  if (matchFound > 1) { // multiple?
    LOGE("%s: AMBIGUOUS: found %d (multiple) matches for device %s\n",__FUNCTION__,matchFound,get_token(3)->ascii);
    cep_search_acc_and_printAll(get_token(3)->ascii);
    return 1;
  } else if (matchFound != 1) {
    LOGE("%s: ERROR: Can't find a match for device %s\n",__FUNCTION__,get_token(3)->ascii);
    cep_cmd_print(cep_get_cmd(CMD_write));
    return 1;
  }
  //
  // now execute the write function if defined
  //
  if (curAcc_p->write_vect == NULL) {
    PRNT("%s: Sorry write function is not defined for this device %s\n",__FUNCTION__,curAcc_p->name);
    return 1;
  }
  // now run it
  for (l=0;l<(int)GET_VAR_VALUE(loop);l++) {
    errCnt += (*curAcc_p->write_vect)();
    if (errCnt >= (int)GET_VAR_VALUE(maxErr)) break;
  }
  if (errCnt) {
    LOGE("%s: ERROR: Problem with executing WRITE command\n",__FUNCTION__);
    PRNT("Check Syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_write));
    return 1;
  }
  return errCnt;
}

int cep_acc_compare(void)
{
  int errCnt = 0;
  int i;
  u_int64_t expData = get_token(2)->value;
  u_int64_t mask    = get_token(3)->value;
  //
  if (GET_VAR_VALUE(verbose)) {
    PRNT("%s:\n",__FUNCTION__);
  }
  // save away expectData and mask
  cep_set_data(1,expData);
  cep_set_data(2,mask);
  //
  // adjust the tokens such that it matches "read" command by removing token 2 and 3
  for (i=4;i<cep_get_token_count();i++) {
    cep_copy_token(i, i-2); // token 4 -> token 2, etc...
  }
  cep_set_token_count(cep_get_token_count()-2); // adjust total tokens
  // call the read command with compare enable
  cep_set_search(1); // use to compare
  errCnt += cep_acc_read();
  cep_set_search(0); // reset
  //
  if (errCnt) {
    LOGE("%s: ERROR: Problem with executing COMPARE command\n",__FUNCTION__);
    PRNT("Check Syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_compare));
    return 1;
  }
  return errCnt;
}
// simple write-read back and compare
int cep_acc_regTest(void)
{
  int errCnt = 0;
  int i;
  u_int64_t expData = get_token(2)->value;
  u_int64_t mask    = get_token(3)->value;
  //
  if (GET_VAR_VALUE(verbose)) {
    PRNT("%s:\n",__FUNCTION__);
  }
  // save away expectData and mask
  cep_set_data(1,expData);
  cep_set_data(2,mask);
  //
  //	
  // adjust the tokens such that it matches "write" command by removing token 3 (mask)
  cep_set_token_count(cep_get_token_count()-1); // adjust total tokens
  for (i=4;i<cep_get_token_count();i++) {
    cep_copy_token(i, i-1); // token 4 -> token 3, etc...
  }
  // issue the write
  errCnt += cep_acc_write();
  // adjust the tokens such that it matches "read" command by removing token 3 (write data)
  cep_set_token_count(cep_get_token_count()-1); // adjust total tokens
  for (i=3;i<cep_get_token_count();i++) {
    cep_copy_token(i, i-1); // token 3 -> token 2, etc...
  }
  // call the read command with compare enable
  cep_set_search(1); // use to compare
  errCnt += cep_acc_read();
  cep_set_search(0); // reset
  //
  if (errCnt) {
    LOGE("%s: ERROR: Problem with executing REGTEST command\n",__FUNCTION__);
    PRNT("Check Syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_regTest));
    return 1;
  }
  return errCnt;
}


//
// ======================================
// Memory accesses
// ======================================
//
int cep_acc_memWrite(void)
{
  int errCnt = 0;
  cep_acc_info *curAcc_p;
  int matchFound;
  int l;
  if (GET_VAR_VALUE(verbose)) {
    PRNT("%s:\n",__FUNCTION__);
  }
  //<deviceName> <devId> <memType> <memId> <offset> <data0 data1...>
  // first check for proper argument types <num><num> <str>
  if (!get_token(5)->isNum) {
    LOGE("%s: ERROR: second argument %s MUST be a number\n",__FUNCTION__,get_token(5)->ascii);
    PRNT("Command syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_memWrite));
    return 1;
  }
  // FIRST!! argument must be the name of the device in the acc list so search for it
  curAcc_p = cep_search_acc(get_token(1)->ascii,&matchFound);
  if (matchFound > 1) { // multiple?
    LOGE("%s: AMBIGUOUS: found %d (multiple) matches for device %s\n",__FUNCTION__,matchFound,get_token(1)->ascii);
    cep_search_acc_and_printAll(get_token(1)->ascii);
    return 1;
  } else if (matchFound != 1) {
    LOGE("%s: ERROR: Can't find a match for device %s\n",__FUNCTION__,get_token(1)->ascii);
    cep_cmd_print(cep_get_cmd(CMD_memWrite));
    return 1;
  }
  //
  // now execute the write function if defined
  //
  if (curAcc_p->memwr_vect == NULL) {
    PRNT("%s: Sorry memwr function is not defined for this device %s\n",__FUNCTION__,curAcc_p->name);
    return 1;
  }
  // now run it
  for (l=0;l<(int)GET_VAR_VALUE(loop);l++) {
    errCnt += (*curAcc_p->memwr_vect)();
    if (errCnt >= (int)GET_VAR_VALUE(maxErr)) break;
  }
  if (errCnt) {
    LOGE("%s: ERROR: Problem with executing memWRITE command\n",__FUNCTION__);
    PRNT("Check Syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_memWrite));
    return 1;
  }
  return errCnt;
}

int cep_acc_memRead(void)
{
  int errCnt = 0;
  cep_acc_info *curAcc_p;
  int matchFound;
  int l;
  //<deviceName> <devId> <memType> <memId> <offset> [word2read]
  if (GET_VAR_VALUE(verbose)) {
    PRNT("%s:\n",__FUNCTION__);
  }
  // first check for proper argument types num <str>
  if (!get_token(5)->isNum) {
    LOGE("%s: ERROR: second argument %s MUST be a number\n",__FUNCTION__,get_token(5)->ascii);
    PRNT("Command syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_memRead));
    return 1;
  }
  // FIRST argument must be the name of the device in the acc list so search for it
  curAcc_p = cep_search_acc(get_token(1)->ascii,&matchFound);
  if (matchFound > 1) { // multiple?
    LOGE("%s: AMBIGUOUS: found %d (multiple) matches for device %s\n",__FUNCTION__,matchFound,get_token(1)->ascii);
    cep_search_acc_and_printAll(get_token(1)->ascii);
    return 1;
  } else if (matchFound != 1) {
    LOGE("%s: ERROR: Can't find a match for device %s\n",__FUNCTION__,get_token(1)->ascii);
    cep_cmd_print(cep_get_cmd(CMD_memRead));
    return 1;
  }
  //
  // now execute the read function if it is defined
  //
  if (curAcc_p->memrd_vect == NULL) {
    PRNT("%s: Sorry memrd function is not defined for this device %s\n",__FUNCTION__,curAcc_p->name);
    return 1;
  }
  // now run it thru a loop if it is 
  for (l=0;l<(int)GET_VAR_VALUE(loop);l++) {
    errCnt += (*curAcc_p->memrd_vect)();
    if (errCnt >= (int)GET_VAR_VALUE(maxErr)) break;
  }
  if (errCnt) {
    LOGE("%s: ERROR: Problem with executing memREAD command\n",__FUNCTION__);
    PRNT("Check Syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_memRead));
    return 1;
  }
  return errCnt;
}

int cep_acc_memSearch(void)
{
  int errCnt = 0;
  cep_acc_info *curAcc_p;
  int matchFound;
  //int l;
  //char tmp[64];
  //char *p;
  u_int64_t curOffset;
  int word2search ;
  //<deviceName> <devId> <memType> <memId> <offset> <word2search> <data> <mask>
  if (GET_VAR_VALUE(verbose)) {
    PRNT("%s:\n",__FUNCTION__);
  }
  // first check for proper argument types num <str>
  if (!get_token(5)->isNum) {
    LOGE("%s: ERROR: second argument %s MUST be a number\n",__FUNCTION__,get_token(5)->ascii);
    PRNT("Command syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_memRead));
    return 1;
  }
  //
  // FIRST argument must be the name of the device in the acc list so search for it
  //
  curAcc_p = cep_search_acc(get_token(1)->ascii,&matchFound);
  if (matchFound > 1) { // multiple?
    LOGE("%s: AMBIGUOUS: found %d (multiple) matches for device %s\n",__FUNCTION__,matchFound,get_token(1)->ascii);
    cep_search_acc_and_printAll(get_token(1)->ascii);
    return 1;
  } else if (matchFound != 1) {
    LOGE("%s: ERROR: Can't find a match for device %s\n",__FUNCTION__,get_token(1)->ascii);
    cep_cmd_print(cep_get_cmd(CMD_memSearch));
    return 1;
  }
  //
  // now check the read function if it is defined
  //
  if (curAcc_p->memrd_vect == NULL) {
    PRNT("%s: Sorry memrd function is not defined (need for search) for this device %s\n",__FUNCTION__,curAcc_p->name);
    return 1;
  }
  //
  // and start doing the search	
  // 
  curOffset   = get_token(5)->value;
  word2search = get_token(6)->value;
  strcpy(cep_get_searchPat(), get_token(7)->ascii);
  PRNT("%s: startOffset=0x%016lx numWord2search=%d pattern2search=%s\n",__FUNCTION__,
       curOffset,word2search,get_token(7)->ascii);
  //
  cep_set_search(1); // no printing on read unless match
  // now execute the memRead 
#if 1
  errCnt += (*curAcc_p->memrd_vect)();
  //if (errCnt >= GET_VAR_VALUE(maxErr)) break;
#else
  strcpy(tmp,"1"); // word2read = 1
  cep_set_token(6, tmp);
  for (l=0;l<word2search;l++) {
    // change offset and word2read=1
    sprintf(tmp,"0x%x",curOffset);
    cep_set_token(5, tmp);
    errCnt += (*curAcc_p->memrd_vect)();
    if (errCnt >= GET_VAR_VALUE(maxErr)) break;
    // move into the actual target to support multiple memread or *
    // do the check, if match print
    p = strstr(cep_get_dataStr(), get_token(7)->ascii);
    if (p != NULL) {
      PRNT("\t offset=0x%016lx Data=%s\n",curOffset,cep_get_dataStr());
    }
    // next
    curOffset++;
  }
#endif
  cep_set_search(0); // put it back
  if (errCnt) {
    LOGE("%s: ERROR: Problem with executing memSearch command\n",__FUNCTION__);
    PRNT("Check Syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_memSearch));
    return 1;
  }
  return errCnt;
}


//
// ================================
// DUMP
// ================================
//
int cep_acc_dump(void)
{
  int errCnt = 0;
  cep_acc_info *curAcc_p;
  int matchFound;
  if (GET_VAR_VALUE(verbose)) {
    PRNT("%s:\n",__FUNCTION__);
  }
  // first argument must be the name of the device in the acc list so search for it
  curAcc_p = cep_search_acc(get_token(1)->ascii,&matchFound);
  if (matchFound > 1) { // multiple?
    LOGE("%s: AMBIGUOUS: found %d (multiple) matches for device %s\n",__FUNCTION__,matchFound,get_token(1)->ascii);
    cep_search_acc_and_printAll(get_token(1)->ascii);
    return 1;
  } else if (matchFound != 1) {
    LOGE("%s: ERROR: Can't find a match for device %s\n",__FUNCTION__,get_token(1)->ascii);
    cep_cmd_print(cep_get_cmd(CMD_dump));
    return 1;
  }
  //
  // now execute the dump function if it is defined
  //
  if (curAcc_p->dump_vect == NULL) {
    PRNT("%s: Sorry dump function is not defined for this device %s\n",__FUNCTION__,curAcc_p->name);
    return 1;
  }
  // now run it
  errCnt = (*curAcc_p->dump_vect)();
  if (errCnt) {
    LOGE("%s: ERROR: Problem with executing DUMP command\n",__FUNCTION__);
    PRNT("Check Syntax:\n");
    cep_cmd_print(cep_get_cmd(CMD_dump));
    return 1;
  }
  return errCnt;
}


// ================================
u_int64_t cep_get_id_mask(int tokenId, u_int64_t def)
{
  u_int64_t idMask;
  int isWild;
  //
  if (cep_get_args_count() >= tokenId) { // get I2CS id
    idMask = cep_token_get_id(tokenId, &isWild);
    if (isWild) return def;
    else { // single
      idMask = 1 << idMask; // change to mask
    }
  } else {
    idMask = def;
  }
  if (VERBOSE2()) {
    PRNT("%s: token=0x%016lx (%d) def=0x%016lx idMask=0x%016lx isWild=%d\n",__FUNCTION__,
	 get_token(tokenId)->value,tokenId,def,idMask,isWild);
  }
  return idMask;
}

u_int64_t cep_get_id_mask64(int tokenId, u_int64_t def)
{
  u_int64_t idMask;
  int isWild;
  //
  if (cep_get_args_count() >= tokenId) { // get I2CS id
   idMask = cep_token_get_id(tokenId, &isWild);
    if (isWild) return def;
    else { // single
      idMask = (u_int64_t)1 << idMask; // change to mask
    }
  } else {
    idMask = def;
  }
  if (VERBOSE2()) {
    PRNT("%s: token=0x%016lx (%d) def=0x%16lx idMask=0x%16lx isWild=%d\n",__FUNCTION__,
	 get_token(tokenId)->value,tokenId,def,idMask,isWild);
  }
  return idMask;
}

u_int64_t cep_get_value_if_any(int tokenId, u_int64_t def)
{
  u_int64_t value;
  //
  if (cep_get_args_count() >= tokenId) { // get I2CS id
    value = get_token(tokenId)->value;
  } else {
    value = def;
  }
  if (VERBOSE2()) {
    PRNT("%s: token=0x%016lx (%d) def=0x%016lx value=0x%016lx\n",__FUNCTION__,
	 get_token(tokenId)->value,tokenId,def,value);
  }
  return value;
}

u_int64_t cep_get_mask_if_any(int tokenId, u_int64_t def)
{
  u_int64_t value;
  //
  if (cep_get_args_count() >= tokenId) { // get I2CS id
    if (get_token(tokenId)->ascii[0] == '*') {
      value = -1;
    } else {
      value = 1 << get_token(tokenId)->value;
    }
  } else {
    value = 1 << def;
  }
  if (VERBOSE2()) {
    PRNT("%s: token=0x%016lx (%d) def=0x%016lx value=0x%016lx\n",__FUNCTION__,
	 get_token(tokenId)->value,tokenId,def,value);
  }
  return value;
}

char *cep_get_string_if_any(int tokenId)
{
  //
  if (cep_get_args_count() >= tokenId) { // get I2CS id
    return get_token(tokenId)->ascii;
  } else {
    return NULL;
  }
}


//
// =======================
// global stuffs to support memSearch
// =======================
//
int cep_searchFlag = 0;
u_int64_t cep_return_data[16];
int cep_return_data_valid_count = 0;
char cep_datStr[1024];
char cep_searchPat[256];

char *cep_get_dataStr(void)
{
  return cep_datStr;
}
char *cep_get_searchPat(void)
{
  return cep_searchPat;
}

void cep_set_data(int idx, u_int64_t data)
{
  cep_return_data[idx & 0xf] = data;
  // also save in visible variable and its string
  SET_VAR_VALUE(lastDataRead,data);
}
u_int64_t cep_get_data(int idx)
{
  return cep_return_data[idx & 0xf];
}
void cep_set_dataValidCnt(int cnt)
{
  cep_return_data_valid_count = cnt;
}
int cep_get_dataValidCnt(void)
{
  return cep_return_data_valid_count;
}
void cep_set_search(int value)
{
  cep_searchFlag = value;
}
int cep_get_search(void)
{
  return cep_searchFlag;
}

int cep_get_access_type(int tokenId, int defaultType)
{
  int len;
  char *str;
  if (cep_get_args_count() >= tokenId) { 
    str = get_token(tokenId)->ascii;
    len = strlen(str);
    if (strncasecmp(str,"i2c",MIN(3,len)) == 0) { // found it
      return I2C_ACCESS;
    } else if (strncasecmp(str,"si2c",MIN(3,len)) == 0) { // found it
      return SECONDARY_I2C_ACCESS;
    } else if (strncasecmp(str,"pi2c",MIN(3,len)) == 0) { // 
      return PRIMARY_I2C_ACCESS;
    } else if (strncasecmp(str,"pcie",MIN(3,len)) == 0) { // 
      return PCIE_ACCESS;
    } else if (strncasecmp(str,"serdes",MIN(3,len)) == 0) { // 
      return SERDES_ACCESS;
    } else if (strncasecmp(str,"line",MIN(3,len)) == 0) { // 
      return LINE_ACCESS;
    } else if (strncasecmp(str,"pline",MIN(3,len)) == 0) { // 
      return PORT_LINE_ACCESS;
    } else if (strncasecmp(str,"system",MIN(3,len)) == 0) { // 
      return SYSTEM_ACCESS;
    } else if (strncasecmp(str,"psystem",MIN(3,len)) == 0) { // 
      return PORT_SYSTEM_ACCESS;
    } else if (strncasecmp(str,"picxo",MIN(3,len)) == 0) { // 
      return PICXO_ACCESS;
    } else if (strncasecmp(str,"qpll",MIN(3,len)) == 0) { // 
      return QPLL_ACCESS;
    } else if (strncasecmp(str,"pll",MIN(3,len)) == 0) { // 
      return PLL_ACCESS;
    } else if (strncasecmp(str,"range",MIN(3,len)) == 0) { // 
      return DUMP_RANGE;
    } else if (strncasecmp(str,"eyescan",MIN(3,len)) == 0) { // 
      return EYESCAN_ACCESS;
    } else if (strncasecmp(str,"aperture",MIN(3,len)) == 0) { // 
      return APERTURE;
    } else if (strncasecmp(str,"DDR3",MIN(3,len)) == 0) { // 
      return DDR3_ACCESS;
    } else if (strncasecmp(str,"XR2",MIN(3,len)) == 0) { // 
      return XR2_ACCESS;
    } else if (strncasecmp(str,"DMONITOR",MIN(3,len)) == 0) { // 
      return DMONITOR;
    } else if (strncasecmp(str,"QUICK",MIN(3,len)) == 0) { // 
      return QUICK_ACCESS;
    } else if (strncasecmp(str,"INDIRECT",MIN(3,len)) == 0) { // 
      return INDIRECT_ACCESS;
    } else if (strncasecmp(str,"EFUSE",MIN(3,len)) == 0) { // 
      return EFUSE_ACCESS;
    } else {
      PRNT("%s: ERROR. access type %s is not recognized..Used %d instead\n",__FUNCTION__,
	   str,defaultType);
      return   defaultType;
    }
  } else return   defaultType;
}

