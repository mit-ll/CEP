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
#ifndef riscv_wrapper_H
#define riscv_wrapper_H

#define NHARTS   4

#define MTIME           (*(volatile long long *)(0x02000000 + 0xbff8))
#define MTIMECMP        ((volatile long long *)(0x02000000 + 0x4000))

// Write 32-bit ONLY
#define MSIP_OFFS       ((volatile uint32_t *)(0x02000000))


#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*trap_handler_t)(unsigned hartid, unsigned long long mcause, void *mepc,
				void *sp);

  void thread_entry(int cid, int nc);
  int set_trap_handler(trap_handler_t handler);
  void enable_timer_interrupts();
  void enable_sw_interrupts();
  void handle_trap(unsigned long long mcause, void *mepc, void *sp);
  //
  // Timerupt handler
  //
  void *timerIntr(unsigned hartid, unsigned long long mcause, void *mepc, void *sp);
  void *swIntr(unsigned hartid, unsigned long long mcause, void *mepc, void *sp);

#ifdef __cplusplus
}
#endif

#endif
