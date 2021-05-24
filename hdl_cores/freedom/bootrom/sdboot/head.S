#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#//
#// File Name:      
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************

// See LICENSE for license details.
#include <platform.h>
#include <smp.h>
#include "common.h"

  .section .text.init
  .option norvc
  .globl _prog_start
_prog_start:
  smp_pause(s1, s2)
#ifdef _USE_xEXE
	#
	# -----------------------
	# jump to xExeInit init (core0 ONLY)
	# -----------------------	
	#
  li sp, (SCRATCH_DEST + 0xf000)
  call xExeInit
#endif	
	# -----------------------	
	# back to main
	# -----------------------	
  li sp, (PAYLOAD_DEST + 0x7fff000)
  call main
  smp_resume(s1, s2)
  csrr a0, mhartid
  la a1, dtb
  li s1, PAYLOAD_DEST
  jr s1

  .section .rodata
dtb:
  .incbin DEVICE_TREE
