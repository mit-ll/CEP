//-------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      head.S
// Program:        Common Evaluation Platform (CEP)
// Description:    Bootrom entry code
// Notes:          
//
//--------------------------------------------------------------------------------------
// See LICENSE.Sifive for license details.
#include <platform.h>
#include <smp.h>

  .section .text.init
  .option norvc
  .globl _prog_start
_prog_start:

  // Initialize the stack pointer
  li sp, (MEMORY_MEM_ADDR + MEMORY_MEM_SP_OFFSET)

  // Call the main function witha single core
  smp_pause(s1, s2)
  call main
  smp_resume(s1, s2)

  csrr a0, mhartid // hartid for next level bootloader
  la a1, dtb // dtb address for next level bootloader
  
  // Jump to main memory
  li s1, MEMORY_MEM_ADDR
  jr s1

  .section .dtb
  .align 3
dtb:
