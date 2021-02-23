//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

//
// =============================
// This is the file where all the overload codes should be 
// =============================
//
#ifndef PORTABLE_IO_H
#define PORTABLE_IO_H

#include <stdio.h>

// =============================
// See from simulation/verilog/HW side
// =============================
#if defined(_SIM_HW_ENV)

#define LOGD   my_io_logD
#define LOGW   my_io_logW
#define LOGI   my_io_logI
#define LOGE   my_io_logE
#define LOGF   my_io_logF

// =============================
// See from simulation/C side
// =============================
#elif defined(_SIM_SW_ENV)

#include "v2c_sys.h"

#if 1
#define LOGI   v2cLogI
#define LOGW   v2cLogI
#define LOGE   v2cLogE
#define LOGF   v2cLogF
#else
#define LOGI   printf
#define LOGW   printf
#define LOGE   printf
#define LOGF   printf
#endif

#ifdef USE_DPI
#define DUT_ATOMIC_RMW64(a,p,m,d) sim_Atomic_Rdw64(a,p,m,d)
#define DUT_WRITE32_BURST(a,s,d) sim_Write64_BURST(a,s,d)
#define DUT_READ32_BURST(a,s,d)  sim_Read64_BURST(a,s,d)

#define DUT_WRITE32_64(a,d) sim_Write64_64(a,d)
#define DUT_READ32_64(a,d)  d=sim_Read64_64(a)

#define DUT_WRITE32_32(a,d) sim_Write32_32(a,d)
#define DUT_READ32_32(a,d)  d=sim_Read32_32(a)

#define DUT_WRITE32_16(a,d) sim_Write32_16(a,d)
#define DUT_READ32_16(a,d)  d=sim_Read32_16(a)

#define DUT_WRITE32_8(a,d) sim_Write32_8(a,d)
#define DUT_READ32_8(a,d)  d=sim_Read32_8(a)


#define DDR3_WRITE(a,d) sim_Write64_64(a,d)
#define DDR3_READ(a,d)  d=sim_Read64_64(a)

#else
// no more VPI
#endif

#define DUT_WRITE_DVT(msb,lsb,val) sim_WriteDvtFlag(msb,lsb,val)
#define DUT_READ_DVT(msb,lsb)      sim_ReadDvtFlag(msb,lsb)

#define DUT_SetInActiveStatus      sim_SetInActiveStatus

#define DUT_RUNCLK(x)              sim_RunClk(x)

// framer
#define DUT_FRAMER_RDWR(a,wd,rd) sim_Framer_RdWr(a,wd,rd)
#define DUT_SAMPLE_RDWR(a,wd,rd) sim_Sample_RdWr(a,wd,rd)

// =============================
// See from other enviroment (lab/Linux/bareMetal/etc..)
// =============================
#elif defined(LINUX_MODE)

#include "cep_diag.h"
#include "cep_io.h"
#define LOGI   THR_LOGI
#define LOGW   THR_LOGW
#define LOGE   THR_LOGE
#define LOGF   THR_LOGF
// so core display
#define PRNT   printf

#define DUT_WRITE32_64(a,d) lnx_cep_write(a,d)
#define DUT_READ32_64(a,d)  { d = lnx_cep_read(a); }

#define DUT_WRITE32_32(a,d) lnx_cep_write32(a,(int)d)
#define DUT_READ32_32(a,d)  { d = (int)lnx_cep_read32(a); }

#define DUT_WRITE32_16(a,d) lnx_cep_write16(a,(uint16_t)d)
#define DUT_READ32_16(a,d)  { d = (uint16_t)lnx_cep_read16(a); }

#define DUT_WRITE32_8(a,d) lnx_cep_write8(a,(uint8_t)d)
#define DUT_READ32_8(a,d)  { d = (uint8_t)lnx_cep_read8(a); }

#define DDR3_WRITE(a,d) *(volatile uint64_t *)((intptr_t)a)=d
#define DDR3_READ(a,d)  d=*(volatile uint64_t *)((intptr_t)a)

#define DUT_RUNCLK(x)              
//
// =============================
// See from other enviroment (lab/Linux/bareMetal/etc..)
// =============================
#else

#define DUT_RUNCLK(x)

#define LOGI   printf
#define LOGW   printf
#define LOGE   printf
#define LOGF   printf

#ifdef BARE_MODE
#define DUT_WRITE32_64(a,d) *(volatile uint64_t *)((intptr_t)a)=d
#define DUT_READ32_64(a,d)  d=*(volatile uint64_t *)((intptr_t)a)

#define DDR3_WRITE(a,d) *(volatile uint64_t *)((intptr_t)a)=d
#define DDR3_READ(a,d)  d=*(volatile uint64_t *)((intptr_t)a)

// ONLY in bare metal MODE
#define DUT_WRITE32_8(a,d) *reinterpret_cast<volatile uint8_t *>(a)=d
#define DUT_READ32_8(a,d)  d=*reinterpret_cast<volatile uint8_t *>(a)

#define DUT_WRITE32_16(a,d) *reinterpret_cast<volatile uint16_t *>(a)=d
#define DUT_READ32_16(a,d)  d=*reinterpret_cast<volatile uint16_t *>(a)

#define DUT_WRITE32_32(a,d) *reinterpret_cast<volatile uint32_t *>(a)=d
#define DUT_READ32_32(a,d)  d=*reinterpret_cast<volatile uint32_t *>(a)

#else
#define DUT_WRITE32_64(a,d) *reinterpret_cast<volatile uint64_t *>(a)=d
#define DUT_READ32_64(a,d)  d=*reinterpret_cast<volatile uint64_t *>(a)

#define DUT_WRITE32_32(a,d) *reinterpret_cast<volatile uint32_t *>(a)=d
#define DUT_READ32_32(a,d)  d=*reinterpret_cast<volatile uint32_t *>(a)

#define DUT_WRITE32_16(a,d) *reinterpret_cast<volatile uint16_t *>(a)=d
#define DUT_READ32_16(a,d)  d=*reinterpret_cast<volatile uint16_t *>(a)

#define DUT_WRITE32_8(a,d) *reinterpret_cast<volatile uint8_t *>(a)=d
#define DUT_READ32_8(a,d)  d=*reinterpret_cast<volatile uint8_t *>(a)

#define DDR3_WRITE(a,d) *reinterpret_cast<volatile uint64_t *>(a)=d
#define DDR3_READ(a,d)  d=*reinterpret_cast<volatile uint64_t *>(a)

#endif

#endif

//
// =============================
// common for all plaforms or derived from above
// =============================
//
#define thread_timed_yield()
#define MAYBE_YIELD(y) if ((y++ & 0x1000) == 0) thread_timed_yield()


// -----------------------------
#endif
