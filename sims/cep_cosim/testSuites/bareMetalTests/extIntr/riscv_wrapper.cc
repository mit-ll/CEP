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
#include "cep_adrMap.h"
#include "cep_apis.h"
#include "portable_io.h"
#include "CEP.h"

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
trap_handler_t trap_handler[NHARTS] = {NULL,NULL,NULL,NULL}; 

// GPIO starts as Interrupt[3] (1 based)
int gpioIntrStart = 3;

//
// Get here either from external M-mode or S-mode interrupt
//
void *extIntr(unsigned hartid, unsigned long long mcause, void *mepc, void *sp)
{
    uint32_t offS;
    set_csr(mie, 0);
    //
    interrupt_count[hartid] |= 1 << (mcause & 0x1f); // set the bit
    // clear interrupt, do both since we dont know which one..
    for (int ctx=0;ctx<2;ctx++) {
        offS = plic_base_addr + plic_hart0_m_mode_ien_start + (plic_mode_ien_size_offset*(hartid*2 + ctx));              
        DUT_WRITE32_32(offS, 0);
    }
    //
    // save mcause to my sratch register for debug
    //
    DUT_WRITE32_64(CEPREGS_BASE_ADDR + cep_scratch0_reg + (hartid * 8), mcause);
    //
    return mepc;
}

int set_trap_handler(trap_handler_t handler)
{
    int errCnt = 0;
    unsigned hartid = read_csr(mhartid);
    errCnt = trap_handler[hartid] == NULL ? 0 : 1;
    trap_handler[hartid] = handler;
    return errCnt;
}

void enable_external_interrupts(unsigned hartid, int ctx)
{
    uint32_t offS;
    //
    // Get the lock befoe can pulse the GPIO
    //
    // enable my interrupt
    offS = plic_base_addr + plic_hart0_m_mode_ien_start + (plic_mode_ien_size_offset*(hartid*2 + ctx));        
    DUT_WRITE32_32(offS, 1 << (gpioIntrStart + (hartid*2 + ctx)));
    //
    // get the lock to create a pulse on targeted GPIO
    //
    if (cep_get_lock(hartid, 1, 5000)) {
        DUT_WRITE32_32(gpio_base_addr + gpio_low_ie, 1 << (hartid*2 + ctx)); // one of gpio
        DUT_WRITE32_32(gpio_base_addr + gpio_low_ie, 0x0);
        cep_release_lock(hartid, 1);        
    }
    set_csr(mie, MIP_MEIP | MIP_SEIP); // either M or S modes
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
    // should never get pass this...
    while (1) ;
}
//
// Main
//
void thread_entry(int cid, int nc)
{
  //
  int errCnt = 0;
  uint32_t offS;  
  //
  set_printf(0);
  set_cur_status(CEP_RUNNING_STATUS);
  //
  unsigned hartid = read_csr(mhartid);
  MTIMECMP[hartid] = (uint64_t)(-1);
  interrupt_count[hartid] = 0;
  //
  local[hartid] += set_trap_handler((trap_handler_t)extIntr);  
  //
  // Use GPIO as external interrupt
  //
  for (int ctx=0;ctx<2;ctx++) {
      DUT_WRITE32_32(plic_base_addr + plic_source0_prio_offset +  (gpioIntrStart+(hartid*2)+ctx)*4, 7); // prio 7
      //
      offS = plic_base_addr + plic_hart0_m_mode_prio_thresh + (plic_prio_thresh_size_offset*((hartid*2)+ctx));
      DUT_WRITE32_32(offS, 6); // threshold = 6
  }
  //
  // each hart has M/S-mode interrupts, test one at a time
  //
  int expCount = 0;
  for (int ctx=0;ctx<2;ctx++) {
      enable_external_interrupts(hartid, ctx);
      //
      expCount |= (ctx == 0) ? (1 << IRQ_M_EXT) : (1 << IRQ_S_EXT);
      //
      // stuck here until got it via interrupt
      //
      while ((interrupt_count[hartid] & (1 << IRQ_M_EXT)) == 0) {
          local[hartid]++;
          if (local[hartid] & 0x8000) {
              errCnt++;
              break;
          }
      }
  }
  //
  // Done
  //
  set_status(errCnt,hartid);

  //
  // Stuck here forever...
  //

  exit(errCnt);
}

#endif
