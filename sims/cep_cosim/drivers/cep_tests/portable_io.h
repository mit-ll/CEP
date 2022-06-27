//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      portable_io.h
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          Linux support has been temporarily removed
//
//--------------------------------------------------------------------------------------


#ifndef PORTABLE_IO_H
#define PORTABLE_IO_H

#include <stdio.h>
#include <stdint.h>

#ifdef BARE_MODE
#include <kprintf.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

  uint64_t  get_physical_addr(int coreIndex, uint32_t pAddress);
  void      cep_write32(int coreIndex, uint32_t pAddress, uint32_t pData);
  uint32_t  cep_read32(int coreIndex, uint32_t pAddress);
  void      cep_write64(int coreIndex, uint32_t pAddress, uint64_t pData);
  uint64_t  cep_read64(int coreIndex, uint32_t pAddress);
  
#ifdef __cplusplus
}
#endif

// Operating in Simulation
#ifdef SIM_ENV_ONLY

  #include "simPio.h"

  #if defined(_SIM_HW_ENV)
    #define LOGD   my_io_logD
    #define LOGW   my_io_logW
    #define LOGI   my_io_logI
    #define LOGE   my_io_logE
    #define LOGF   my_io_logF

  // See from simulation/C side
  #elif defined(_SIM_SW_ENV)

    #include "v2c_sys.h"
    #define LOGI   v2cLogI
    #define LOGW   v2cLogI
    #define LOGE   v2cLogE
    #define LOGF   v2cLogF

    #define DUT_ATOMIC_RMW64(a,p,m,d)   sim_Atomic_Rdw64(a,p,m,d)
    #define DUT_WRITE32_BURST(a,s,d)    sim_Write64_BURST(a,s,d)
    #define DUT_READ32_BURST(a,s,d)     sim_Read64_BURST(a,s,d)
    #define DUT_WRITE32_64(a,d)         sim_Write32_64(a,d)
    #define DUT_READ32_64(a,d)          d = sim_Read32_64(a)
    #define DUT_WRITE32_32(a,d)         sim_Write32_32(a,d)
    #define DUT_READ32_32(a,d)          d = sim_Read32_32(a)
    #define DUT_WRITE32_16(a,d)         sim_Write32_16(a,d)
    #define DUT_READ32_16(a,d)          d = sim_Read32_16(a)
    #define DUT_WRITE32_8(a,d)          sim_Write32_8(a,d)
    #define DUT_READ32_8(a,d)           d = sim_Read32_8(a)
    #define DUT_WRITE_DVT(msb,lsb,val)  sim_WriteDvtFlag(msb,lsb,val)
    #define DUT_READ_DVT(msb,lsb)       sim_ReadDvtFlag(msb,lsb)
    #define DUT_SETINACTIVESTATUS       sim_SetInActiveStatus
    #define DUT_RUNCLK(x)               sim_RunClk(x)
    #define USEC_SLEEP(x)              

  #endif // if defined(_SIM_HW_ENV)

#else

  #define DUT_RUNCLK(x)
  #define USEC_SLEEP(x)

  #define LOGI   printf
  #define LOGW   printf
  #define LOGE   printf
  #define LOGF   printf

  #ifdef BARE_MODE
    #define DUT_WRITE32_64(a,d)     *(volatile uint64_t *)((intptr_t)a)=d
    #define DUT_READ32_64(a,d)      d = *(volatile uint64_t *)((intptr_t)a)
    #define DUT_WRITE32_8(a,d)      *reinterpret_cast<volatile uint8_t *>(a)=d
    #define DUT_READ32_8(a,d)       d = *reinterpret_cast<volatile uint8_t *>(a)
    #define DUT_WRITE32_16(a,d)     *reinterpret_cast<volatile uint16_t *>(a)=d
    #define DUT_READ32_16(a,d)      d = *reinterpret_cast<volatile uint16_t *>(a)
    #define DUT_WRITE32_32(a,d)     *reinterpret_cast<volatile uint32_t *>(a)=d
    #define DUT_READ32_32(a,d)      d = *reinterpret_cast<volatile uint32_t *>(a)
    #define USEC_SLEEP(x)              
  #else
    #define DUT_WRITE32_64(a,d)     *reinterpret_cast<volatile uint64_t *>(a)=d
    #define DUT_READ32_64(a,d)      d = *reinterpret_cast<volatile uint64_t *>(a)
    #define DUT_WRITE32_32(a,d)     *reinterpret_cast<volatile uint32_t *>(a)=d
    #define DUT_READ32_32(a,d)      d = *reinterpret_cast<volatile uint32_t *>(a)
    #define DUT_WRITE32_16(a,d)     *reinterpret_cast<volatile uint16_t *>(a)=d
    #define DUT_READ32_16(a,d)      d = *reinterpret_cast<volatile uint16_t *>(a)
    #define DUT_WRITE32_8(a,d)      *reinterpret_cast<volatile uint8_t *>(a)=d
    #define DUT_READ32_8(a,d)       d = *reinterpret_cast<volatile uint8_t *>(a)
  #endif
#endif

// common for all plaforms or derived from above
#define thread_timed_yield()
#define MAYBE_YIELD(y) if ((y++ & 0x1000) == 0) thread_timed_yield()


#endif
