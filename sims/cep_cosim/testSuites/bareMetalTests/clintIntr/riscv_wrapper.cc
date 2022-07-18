//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
//
// For bareMetal mode ONLY
//
#ifdef BARE_MODE
#include "CEP.h"
#include "cep_adrMap.h"
#include "cep_apis.h"
#include "portable_io.h"

#include "cepRegTest.h"
#include "portable_io.h"

#include "riscv_wrapper.h"

//
// =================================
// Interrupt stuffs
// =================================
//
volatile unsigned interrupt_count[NHARTS];
volatile unsigned local[NHARTS];

void *timerIntr(unsigned hartid, unsigned long long mcause, void *mepc, void *sp)
{
  //unsigned delta = 0x100;
  set_csr(mie, 0);
  //
  interrupt_count[hartid] |= 1 << (mcause & 0x1f); // set the bit
  // wait til clear
  while (read_csr(mip) & MIP_MTIP) {
    MTIMECMP[hartid] = (uint64_t)(-1); // clear
  }
  // save mcause to my sratch register
  DUT_WRITE32_64(CEPREGS_BASE_ADDR + cep_scratch0_reg + (hartid * 8), mcause);
  //
  return mepc;
}

void *swIntr(unsigned hartid, unsigned long long mcause, void *mepc, void *sp)
{
  set_csr(mie, 0);
  MSIP_OFFS[hartid] = 0; // clear??
  //
  interrupt_count[hartid] |= 1 << (mcause & 0x1f); // set the bit
  //
  // Clear the interrup
  //
  //
  // save mcause to my sratch register
  //
  DUT_WRITE32_64(CEPREGS_BASE_ADDR + cep_scratch0_reg + (hartid * 8), mcause);
  //
  // jump pass ecall!!!
  //
  //return (mepc + 4); // if use ecall
  return mepc;
}

trap_handler_t trap_handler[NHARTS] = {NULL,NULL,NULL,NULL}; // timerIntr,timerIntr,timerIntr,timerIntr}; 

int set_trap_handler(trap_handler_t handler)
{
  int errCnt = 0;
  unsigned hartid = read_csr(mhartid);
  errCnt = trap_handler[hartid] == NULL ? 0 : 1;
  trap_handler[hartid] = handler;
  return errCnt;
}

void enable_timer_interrupts()
{
    set_csr(mie, MIP_MTIP);
    set_csr(mstatus, MSTATUS_MIE);
    return;
}

void enable_sw_interrupts()
{
    set_csr(mie, MIP_MSIP);
    set_csr(mstatus, MSTATUS_MIE);
    return;
}

void handle_trap(unsigned long long mcause, void *mepc, void *sp)
{
    unsigned hartid = read_csr(mhartid);
    if (trap_handler[hartid]) {
      (*trap_handler[hartid])(hartid, mcause, mepc, sp);
        return;
    }

    while (1)
        ;
}

//
// Main
//
void thread_entry(int cid, int nc)
{
  //
  int errCnt = 0;
  //
  set_cur_status(CEP_RUNNING_STATUS);
  //
  unsigned hartid = read_csr(mhartid);
  MTIMECMP[hartid] = (uint64_t)(-1);
  interrupt_count[hartid] = 0;
  local[hartid] = 0;

  //
  // ==================================
  // First do the timer interrupt
  // ==================================
  //
  local[hartid] += set_trap_handler((trap_handler_t)timerIntr);
  // one less then current => interrupt
  MTIMECMP[hartid] = MTIME - 1;
  enable_timer_interrupts();
  //
  // stuck until got it
  //
  while ((interrupt_count[hartid] & (1 << IRQ_M_TIMER)) == 0) {
    local[hartid]++;
    if (local[hartid] & 0x8000) {
      errCnt++;
      break;
    }
  }
  if (errCnt) goto done;
  //
  // ==================================
  // Next is SW interrupt via ecall 
  // ==================================
  //
  local[hartid] += set_trap_handler((trap_handler_t)swIntr);
  enable_sw_interrupts();
  // execute an ecall to cause an SW interrupt = IRQ_M_EXT
  //asm volatile ("ecall;"); // cause IRQ_M_EXT
  MSIP_OFFS[hartid] = 1; // to create a software interrupt
  //
  // stuck here until interrupt
  //
  //while ((interrupt_count[hartid] & (1 << IRQ_M_EXT)) == 0) {
  while ((interrupt_count[hartid] & (1 << IRQ_M_SOFT)) == 0) {
    local[hartid]++;
    if (local[hartid] & 0x8000) {
      errCnt++;
      break;
    }
  }
  //
  // Done
  //
 done:
  set_status(errCnt,hartid);

  //
  // Stuck here forever...
  //

  exit(errCnt);
}

#endif
