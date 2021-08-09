# See LICENSE for license details.

//
// Tony Duong 12/17/202
//
// Two level page table look ups with statically direct map	
//
// Use 1 G of physical space to support 39-bit virtual so there are some rules MUST be followed
// See below
//	
#include "riscv_test.h"
#include "test_macros.h"

# align 12 = page size of 4K

	
RVTEST_RV64U
RVTEST_CODE_BEGIN

user_code_here:
//
// to cause misAligned access fault
// use default PTW from drivers/virtual/multi_vm.c
//
//
#define EXP      a0
#define ACT      a1
#define SAVE_P   a2
#define ONES     a3
#define CUR_P    t0		
#define CUR_A    t1
#define WRDAT    t2
#define RDDAT    t3	
#define CUR_CNT  t4
#define CUR_LOOP t5
#define WRN_RD	 t6

	// init
	addi ONES,x0,1			; # use as constant 1
	addi CUR_LOOP,x0,1		; # loop = 4 patterns. FIXME: 2-patterns for now = virtual[31:0], [38:32] still under debug!!
	addi WRDAT,x0,1			; # shift-1 = write data pattern
	addi EXP,x0,0			; # EXP = 0
	addi ACT,x0,0			; # ACT = 0

get_pattern:
	addi WRN_RD,x0,0		; # start with Write
	
	la   SAVE_P,pat2          	;
	addi x1,x0,2			;
	beq  CUR_LOOP,x1,start_loop	; # test pat2	
	la   SAVE_P,pat1          	; # test pat1
	j    start_loop			;

	.align 12
start_loop:	
	addi CUR_CNT,x0,4		; # walk-1/0 for 17-bits
	mv   CUR_P, SAVE_P	        ; # start from top
	
sub_loop:
	ld   CUR_A,0(CUR_P)		; # get virtual address point by CUR_P	
	beq  WRN_RD,x0,do_write		; #

do_read:	
	ld   RDDAT,0(CUR_A)		; # ----- READ! ------
	add  ACT,ACT,RDDAT		; # add to actual for compare
	j    continue			; # done?
do_write:
	sd   WRDAT,0(CUR_A)		; # ----- WRITE! -------
	add  EXP,EXP,WRDAT	    	; # compute expected
	slli WRDAT,WRDAT,1		; # next WRDAT

continue:
	add  CUR_P,CUR_P,8		; # point to next virtual address
	sub  CUR_CNT,CUR_CNT,ONES   	; # decrement	
	bne  CUR_CNT,x0,sub_loop	; # until done
	
	beq  WRN_RD, ONES,next_loop	; # next outter loop
	addi WRN_RD,WRN_RD,1		; # now do read loop
	j    start_loop			; #

	.align 12
next_loop:
	sub  CUR_LOOP,CUR_LOOP,ONES	; # back to top	if not done
	beq  CUR_LOOP,x0,compare	; # compare
	j    get_pattern		; #

compare:
	bne EXP,ACT, test_fail	   	; # not equal -> fail
	j   test_pass		   	; # else test pass

#
# Either pass or timeout
#
	.global  test_pass
test_pass:	
RVTEST_PASS

	.global  test_fail
test_fail:
RVTEST_FAIL	

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN
	.align 3
	.global	vm_flags	
vm_flags:
	.dword WALK1_0_2LEVELS
//
// Pattern 1-4
//
	// cross page load/store
	.align 12
pat1:
  	.dword ((0x1<<20)|(0x00002<<12)|(0xFFE))
  	.dword ((0x1<<20)|(0x00004<<12)|(0xFFE))
  	.dword ((0x1<<20)|(0x00008<<12)|(0xFFE))
  	.dword ((0x1<<20)|(0x00010<<12)|(0xFFE))
/*	
  	.dword ((0x1<<29)|(0x00010<<12)|(4<<6))
  	.dword ((0x1<<29)|(0x00020<<12)|(5<<6))
  	.dword ((0x1<<29)|(0x00040<<12)|(6<<6))
  	.dword ((0x1<<29)|(0x00080<<12)|(7<<6))
  	.dword ((0x1<<29)|(0x00100<<12)|(8<<6))
  	.dword ((0x1<<29)|(0x00200<<12)|(9<<6))
  	.dword ((0x1<<29)|(0x00400<<12)|(0<<6))
  	.dword ((0x1<<29)|(0x00800<<12)|(1<<6))
  	.dword ((0x1<<29)|(0x01000<<12)|(2<<6))
  	.dword ((0x1<<29)|(0x02000<<12)|(3<<6))
  	.dword ((0x1<<29)|(0x04000<<12)|(4<<6))
  	.dword ((0x1<<29)|(0x08000<<12)|(5<<6))
  	.dword ((0x1<<29)|(0x10000<<12)|(6<<6))
	// Somehow the pte's PPN[0] should be dont care but NOT. perhpas a bug
  	.dword ((0x1<<29)|(0x003<<21)|(0x0ff<<12)|(0<<6))
  	.dword ((0x1<<29)|(0x006<<21)|(0x0ff<<12)|(0<<6))
  	.dword ((0x1<<29)|(0x00c<<21)|(0x0ff<<12)|(0<<6))
  	.dword ((0x1<<29)|(0x018<<21)|(0x0ff<<12)|(0<<6))
  	.dword ((0x1<<29)|(0x030<<21)|(0x0ff<<12)|(0<<6))
  	.dword ((0x1<<29)|(0x060<<21)|(0x0ff<<12)|(0<<6))					
  	.dword ((0x1<<29)|(0x0c0<<21)|(0x0ff<<12)|(0<<6))
  	.dword ((0x1<<29)|(0x180<<21)|(0x0ff<<12)|(0<<6))						
*/
	.align 12
pat2:	
  	.dword ((0x1<<28)|(0x1fffe<<12)|(0xFFC))
  	.dword ((0x1<<28)|(0x1fffd<<12)|(0xFFC))
  	.dword ((0x1<<28)|(0x1fffb<<12)|(0xFFC))
  	.dword ((0x1<<28)|(0x1fff7<<12)|(0xFFC))
	/*
  	.dword ((0x0<<29)|(0x1ffef<<12)|(4<<6))
  	.dword ((0x0<<29)|(0x1ffdf<<12)|(5<<6))
  	.dword ((0x0<<29)|(0x1ffbf<<12)|(6<<6))
  	.dword ((0x0<<29)|(0x1ff7f<<12)|(7<<6))
  	.dword ((0x0<<29)|(0x1feff<<12)|(8<<6))
  	.dword ((0x0<<29)|(0x1fdff<<12)|(9<<6))
  	.dword ((0x0<<29)|(0x1fbff<<12)|(0<<6))
  	.dword ((0x0<<29)|(0x1f7ff<<12)|(1<<6))
  	.dword ((0x0<<29)|(0x1efff<<12)|(2<<6))
  	.dword ((0x0<<29)|(0x1dfff<<12)|(3<<6))
  	.dword ((0x0<<29)|(0x1bfff<<12)|(4<<6))
  	.dword ((0x0<<29)|(0x17fff<<12)|(5<<6))
  	.dword ((0x0<<29)|(0x0ffff<<12)|(6<<6))
	//
	// Somehow the pte's PPN[0] should be dont care but NOT. perhpas a bug
  	.dword ((0x3<<29)|(0x003<<21)|(0x0ff<<12)|(0<<6))
  	.dword ((0x3<<29)|(0x006<<21)|(0x0ff<<12)|(0<<6))
  	.dword ((0x3<<29)|(0x00c<<21)|(0x0ff<<12)|(0<<6))
  	.dword ((0x3<<29)|(0x018<<21)|(0x0ff<<12)|(0<<6))
  	.dword ((0x3<<29)|(0x030<<21)|(0x0ff<<12)|(0<<6))
  	.dword ((0x3<<29)|(0x060<<21)|(0x0ff<<12)|(0<<6))					
  	.dword ((0x3<<29)|(0x0c0<<21)|(0x0ff<<12)|(0<<6))
  	.dword ((0x3<<29)|(0x180<<21)|(0x0ff<<12)|(0<<6))						
*/
	
RVTEST_DATA_END

