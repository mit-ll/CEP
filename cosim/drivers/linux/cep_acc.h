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

#ifndef cep_acc__H_
#define cep_acc__H_

#include <stddef.h>
#include <stdlib.h>

#ifndef _MICROKERNEL
#include <sys/types.h>
#endif

//
// ==========================
// Erase general
// ==========================
//
typedef int (*erase_block_fn_t)(u_int64_t);
typedef int (*erase_all_fn_t)(void);


typedef struct erase_handle_ {
  char name[64];
  erase_block_fn_t block_erase;
  erase_all_fn_t chip_erase;
  struct erase_handle_ *next; // point to next one
} erase_handle_t;

//
// ==========================
// Attach/detach
// ==========================
//
typedef int (*attach_fn_t)(int device_id);

typedef struct attach_handle_ {
  char name[64];
  int id;
  attach_fn_t device_attach;
  attach_fn_t device_detach;
  int attach_done;
  struct attach_handle_ *next; // point to next one
} attach_handle_t;

//
// ==========================
// MDIO general
// ==========================
//
typedef int (*mdio_wr_fn_t)(int, int, int, u_int8_t , u_int8_t , u_int16_t , u_int16_t, int);
typedef int (*mdio_rd_fn_t)(int, int, int, u_int8_t , u_int8_t , u_int16_t , u_int16_t *, int);


typedef struct mdio_handle_ {
  char name[64];
  mdio_wr_fn_t wr;
  mdio_rd_fn_t rd;
  struct mdio_handle_ *next; // point to next one
} mdio_handle_t;

//
// ==========================
// TRIM general
// ==========================
//
typedef int (*trim_fn_t)(int, int, int, int);
typedef double (*trim_get_fn_t)(int);

typedef struct trim_handle_ {
  char name[64];
  char type[32];
  int  id;
  int nominal_value;
  int percentage;
  int margin;
  trim_fn_t trim;
  trim_get_fn_t getVOUT;
  trim_get_fn_t getIOUT;
  struct trim_handle_ *next; // point to next one
} trim_handle_t;

//
// ==========================
// RELOAD (force <device> to reload its image from flash
// ==========================
//
typedef int (*reload_fn_t)(void);

typedef struct reload_handle_ {
  char name[64];
  reload_fn_t func;
  struct reload_handle_ *next; // point to next one
} reload_handle_t;

//
// ==========================
// CONFIG device
// ==========================
//
typedef int (*config_fn_t)(void);

typedef struct config_handle_ {
  char name[64];
  config_fn_t func;
  struct config_handle_ *next; // point to next one
} config_handle_t;


//
// ==========================
// access type: si2c, pi2c, pcie
// ==========================
//
#define PCIE_ACCESS          0x0
#define SECONDARY_I2C_ACCESS 0x1
#define PRIMARY_I2C_ACCESS   0x2
#define SERDES_ACCESS        0x3
#define DUMP_RANGE           0x4
#define APERTURE             0x5
#define LINE_ACCESS          0x6
#define SYSTEM_ACCESS        0x7
#define QPLL_ACCESS          0x8
#define PLL_ACCESS           0x9
#define PICXO_ACCESS         0xA
#define PORT_LINE_ACCESS     0xB
#define PORT_SYSTEM_ACCESS   0xC
#define EYESCAN_ACCESS       0xD
#define DDR3_ACCESS          0xE
#define XR2_ACCESS           0xF
#define I2C_ACCESS           0x10
#define DMONITOR             0x11
#define QUICK_ACCESS         0x12
#define INDIRECT_ACCESS      0x13
#define EFUSE_ACCESS         0x14

// Trim
#ifndef TRIM_MARGIN_HIGH 
#define TRIM_MARGIN_HIGH       1
#define TRIM_MARGIN_NOMINAL    0
#define TRIM_MARGIN_LOW       -1
#endif
//
// function pointer point to API to execute the command
//
typedef int (*cep_acc_vect_t)(void);
//
// to hold command name and value
//
typedef struct cep_acc_s cep_acc_info;
typedef struct cep_acc_s {
  int id;
  int valid;
  cep_acc_vect_t write_vect; // to write internal register
  cep_acc_vect_t read_vect; // to read internal register
  cep_acc_vect_t dump_vect; // to dump internal register
  cep_acc_vect_t reset_vect; // to reset the device
  cep_acc_vect_t power_vect; // to power on the device
  cep_acc_vect_t program_vect; // to program the thing!!
  // for memory supports
  cep_acc_vect_t memwr_vect; // to write its external/internal memory
  cep_acc_vect_t memrd_vect; // to read its external/internal memory
  // non-popular access method
  cep_acc_vect_t bringup_vect; // to take device out of reset and possible PCIe
  cep_acc_vect_t reload_vect; // to take device out of reset and possible PCIe
  cep_acc_vect_t eyescan_vect; // to read internal register
  //
  char name[64]; // max
  char help[256]; // access description
  cep_acc_info *next;
} cep_acc_s_t ;

//
// macros
//
#define CEP_ADD_ACC(name,w,r,d,x,p,h)  do {				\
    tmp_p = cep_acc_add2idx(ACC_ ## name, #name,w,r,d,x,p,h);	\
    if (tmp_p == NULL) return 1;					\
  } while (0)

#define CEP_DELETE_ACC(name)  do {				\
    cep_acc_delete(ACC_ ## name);				\
  } while (0)

//
// for memory type and must already defined
//
#define CEP_ADD_MEM_ACC(name,mw,mr)  do {				\
    tmp_p = cep_acc_addmem2idx(ACC_ ## name, mw,mr);		\
    if (tmp_p == NULL) return 1;					\
  } while (0)

//
// Add access handle for non-popular command
//
extern cep_acc_info __cep_acc_array[];
#define ADD_ACC_METHOD(dev,cmd,func)  do {				\
    __cep_acc_array[ACC_ ## dev].cmd ## _vect = func;		\
  } while (0)

//
// ==========================================
// These are for quick index referencing (avoid the search every time it is used)
// And the name MUST begin with ACC_<commandName>
//
// ==========================================
// should be sorted for better display look up
// 
typedef enum {
  ACC_CEP_MARKER=-1,
  // ======== BEGIN (must start at 0) ===========
  ACC_ddr3      ,
  ACC_cep       ,
  // =======================================
  ACC_zzMAX_CEP_COUNT       // end marker
} cep_acc_enum;

//
//================================
// Some macros
//================================
//
#define SEARCH_ACC(n,m)        cep_search_acc(n,m)
#define GET_ACC_NAME(n)        cep_get_acc_name(ACC_ ## n)
#define GET_ACC_HELP(n)        cep_get_acc_help(ACC_ ## n)

//
// ============================
// Function prototypes
// ============================
#ifdef __cplusplus
extern "C" {
#endif
  // =============================
  int cep_init_acc(void);
  void cep_sort_acc(void);

 int cep_acc_write(void);
  int cep_acc_wrStr(void);
  int cep_acc_read(void);
  int cep_acc_compare(void);
  int cep_acc_regTest(void);
  int cep_acc_memWrite(void);
  int cep_acc_memRead(void);
  int cep_acc_memSearch(void);
  int cep_acc_dump(void);
  int cep_acc_bringup(void);
  int cep_acc_reload(void);
  int cep_acc_program(void);
  int cep_acc_reset(void);
  int cep_acc_power(void);
  int cep_acc_eyescan(void);
  void cep_search_acc_and_printAll(char *accName);
  cep_acc_info *cep_search_acc(char *accName, int *matchFound);
  cep_acc_info *cep_acc_add2idx(int idx, const char *name, 
					cep_acc_vect_t write_func_t, 
					cep_acc_vect_t read_func_t, 
					cep_acc_vect_t dump_func_t, 
					cep_acc_vect_t reset_func_t, 
					cep_acc_vect_t program_func_t, 
					const char *help);
  void cep_acc_delete(int idx);
  cep_acc_info *cep_acc_addmem2idx(int idx, cep_acc_vect_t memwr_func_t, cep_acc_vect_t memrd_func_t);
  void cep_acc_print(cep_acc_info *acc_p);
  int cep_acc_dump_all(void);
  cep_acc_info *cep_get_acc(int idx);
  char *cep_get_acc_name(int idx);
  char *cep_get_acc_help(int idx) ;
  int cep_get_max_acc_count(void);
  void cep_set_max_acc_count(int value);

  u_int64_t cep_get_id_mask(int tokenId, u_int64_t def);
  u_int64_t cep_get_id_mask64(int tokenId, u_int64_t def);
  u_int64_t cep_get_value_if_any(int tokenId, u_int64_t def);
  u_int64_t cep_get_mask_if_any(int tokenId, u_int64_t def);
  char *cep_get_string_if_any(int tokenId);
  u_int64_t cep_get_device_mask_if_any(int def);
  u_int64_t cep_get_device_mask64_if_any(u_int64_t def);
  void cep_set_device_mask(u_int64_t def);

  //
  // need for search
  //
  void cep_set_data(int idx, u_int64_t data);
  u_int64_t cep_get_data(int idx);
  void cep_set_dataValidCnt(int cnt);
  int cep_get_dataValidCnt(void);
  void cep_set_search(int value);
  int cep_get_search(void);
  char *cep_get_dataStr(void);
  char *cep_get_searchPat(void);
  int cep_get_access_type(int tokenId, int defaultType);
  //
  //
  //
  void cep_add_mdio_ops(mdio_handle_t *newOp);
  //
  void cep_add_trim_ops(trim_handle_t *newOp);
  //
  //
  void cep_add_config_ops(config_handle_t *newOp);
  int cep_acc_config(void);
  //
  // erase
  //
  void cep_add_erase_ops(erase_handle_t *newOp);
  int cep_acc_erase(void);
  //
  // attach/detach
  //
  void cep_add_attach_ops(attach_handle_t *newOp);
  int cep_acc_attach(void);
  int cep_acc_detach(void);
  int cep_acc_trim(void);
  //int cep_acc_v2p(void);

  // =============================
#ifdef __cplusplus
}
#endif
// ===========================
#endif
