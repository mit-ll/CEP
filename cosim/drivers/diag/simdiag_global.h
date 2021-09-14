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

// ------------------
#ifndef simdiag_global_H
#define simdiag_global_H
// ---------------------------
#ifndef _MICROKERNEL
#include <sys/types.h>
#endif

//#include "supercon_io.h"
#include "portable_io.h"

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

//
//
//
#define HYPERION_BOARD_TYPE   0
#define SNORKEL_BOARD_TYPE    1
#define DPC2X100_BOARD_TYPE   2
#define WINDSURF_BOARD_TYPE   3
#define SCUBA_BOARD_TYPE      4
#define LOAD_MPC1_BOARD_TYPE  5
#define LOAD_MPC2_BOARD_TYPE  6 
// stout, alias
#define ST40X10_BOARD_TYPE    7
#define STOUT_MX_MRATE_BOARD_TYPE  7
#define STOUT_MX_SFPP_BOARD_TYPE   8
//
#define STOUT_TINY_BOARD_TYPE 9
#define ST4X100_BOARD_TYPE    9
// Aloha
#define ALOHA_BOARD_TYPE      10
//
//
// For log stuffs
//
#define TELNET_LOG_LEVEL   0x40
#define DUMP_LOG_LEVEL     0x20
#define INIT_LOG_LEVEL     0x10
#define ACCESS_LOG_LEVEL   0x08
#define MEM_LOG_LEVEL      0x04
#define ENTRY_LOG_LEVEL    0x04
#define DRIVER_LOG_LEVEL   0x04
#define BASE_LOG_LEVEL     0x02
#define TEST_LOG_LEVEL     0x01


//
// from ~/junos/aduong/Snorkel092711/src/pfe/include/adpc/adpc_hw.h
// to control the I2C SCL speed via divider 
// used "void pq3_i2c_init_ctlr ( int bus, u_int8_t slv_addr, u_int8_t fdr)" 
// or npc_i2c_init(0, ADPC_I2C_CTLR_BUS_ADDR, ADPC_I2C_FDR);
//
#define ADPC_I2C_FDR_X4            0x28   /* CCB/2/640,  4x faster */
#define ADPC_I2C_FDR_X2            0x2C   /* CCB/2/1280, 2x faster */
#define ADPC_I2C_FDR_DEF           0x30   /* CCB/2/2560. 65KHz@333MHz, 78KHz@400MHz, 104KHz@533MHz: default */
#define ADPC_I2C_FDR_DIV2          0x34   /* CCB/2/5120,  2x slower */
#define ADPC_I2C_FDR_DIV4          0x38   /* CCB/2/10240, 4x slower */
#define ADPC_I2C_FDR_DIV8          0x3C   /* CCB/2/20480, 8x slower */

#define GET_REVCHECK()  simdiag_revCheck_get()
#define SET_REVCHECK(e) simdiag_revCheck_set(e)

#define SET_VERBOSE(v)   simdiag_SetVerbose(v)

  #ifdef BARE_MODE
#define GET_VERBOSE(me)  0
  #else
#define GET_VERBOSE(me)  (simdiag_GetVerbose() & me->mLogLevelMask)
#endif
  
#define VERBOSE6() (simdiag_GetVerbose()&TELNET_LOG_LEVEL)
#define VERBOSE5() (simdiag_GetVerbose()&DUMP_LOG_LEVEL)
#define VERBOSE4() (simdiag_GetVerbose()&ACCESS_LOG_LEVEL)

#define VERBOSE3() (simdiag_GetVerbose()&DRIVER_LOG_LEVEL)
#define VERBOSE2() (simdiag_GetVerbose()&BASE_LOG_LEVEL)
#define VERBOSE1() (simdiag_GetVerbose()&TEST_LOG_LEVEL)

#define SET_STANDALONE(f) simdiag_SetStandalone(f)
#define GET_STANDALONE()  simdiag_GetStandalone()
  

  //void DELAY_N_Y(int msec);
  
  //
  //
//



#define RETURN_IF_ERROR(func)						\
  do {									\
    if (func) {								\
      LOGE("Failed in function %s while calling from %s..file=%s line=%d\n", #func, __FUNCTION__,__FILE__,__LINE__); \
      return 1;								\
    }									\
  } while (0)

#ifndef RIP
#define RIP RETURN_IF_ERROR
#endif

// return-if-problem
//#define RIP(func) RETURN_IF_ERROR(func)

#ifdef SIM_ENV_ONLY
#define DELAY_N_YIELD(x)    time_delay_us(100)			       
#else
#define DELAY_N_YIELD(x)			\
  do {						\
    time_delay_us(x);				\
    thread_timed_yield();			\
  } while (0)
#endif

#define DELAY_N_Y DELAY_N_YIELD

#endif

#define _FIXME() do { printf("%s: FIXME!!!\n",__FUNCTION__); } while (0)

#ifdef __cplusplus
extern "C" {
#endif
  
  //
//
//
  int simdiag_GetVerbose(void);
  void simdiag_SetVerbose(int verbose);
  
  void simdiag_revCheck_set(int enable);
  int simdiag_revCheck_get(void);


  // for standalone debug
  int simdiag_GetStandalone(void);
  void simdiag_SetStandalone(int standalone);
  
  //
  //
  //
  void set_board_type(int type);
  int get_board_type(void);
  
  // idprom number!!
  void set_board_subtype(int type);
  int get_board_subtype(void);

#ifdef __cplusplus
}
#endif


