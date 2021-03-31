# See LICENSE for license details.

//
// Tony Duong 12/17/2020
//
// One level page table look ups with statically direct map
//	
// See below
//	
#include "riscv_test.h"
#include "test_macros.h"

# align 12 = page size of 4K

	
RVTEST_RV64U
RVTEST_CODE_BEGIN

user_code_here:
//	
// must obey the page table rule as specified in multi_vm.c for this to work!!!
//
// MODE: WALK1_0_2LEVELS (vpn2->vpn0)
//
// Acceptabe User's VA addressing:
//
//  VPN[2]     VPN[1]    VPN[0]   PageOffset
//         3           2          1
// 876543210_987654321_098765432_109876543210
// 111111111 mmmmmmmmm xxxxxxxxx pppppppppppp : Kernel code (megapage)
// 000000000 000000000 00UUUUUUU pppppppppppp : user code (restricted to 64 pages = 256Kbytes)
// 00000wwww 0xxxxxxxx xxxxxxxxx pppppppppppp : walk-1 VPN[1] = pattern1
// 00000WWWW 0xxxxxxxx xxxxxxxxx pppppppppppp : walk-0 VPN[1] = pattern2
//
//
#define EXP      a0
#define ACT      a1
#define SAVE_P   a2
#define ONES     a3
#define PAGE     a4	
#define CUR_P    t0		
#define CUR_A    t1
#define WRDAT    t2
#define RDDAT    t3	
#define CUR_CNT  t4
#define CUR_LOOP t5
#define WRN_RD	 t6

	// init
	addi ONES,x0,1			; # use as constant 1
	slli PAGE,ONES,12		; # page size    
	addi CUR_LOOP,x0,2		; # loop = 4 patterns. FIXME: 2-patterns for now = virtual[31:0], [38:32] still under debug!!
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
	addi CUR_CNT,x0,8 		; # walk-1/0 for 17-bits
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
	add  CUR_P,CUR_P,8		; # point to next page of virtual address
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
	.align 12
pat1:
  	.dword ((0x000fe<<30)|(0x00001<<12)|(1<<6))
  	.dword ((0x000fd<<30)|(0x00002<<12)|(2<<6))
  	.dword ((0x000fb<<30)|(0x00004<<12)|(3<<6))
  	.dword ((0x000f7<<30)|(0x00008<<12)|(4<<6))
  	.dword ((0x000ef<<30)|(0x00010<<12)|(5<<6))
  	.dword ((0x000df<<30)|(0x00020<<12)|(6<<6))
  	.dword ((0x000bf<<30)|(0x00040<<12)|(7<<6))
  	.dword ((0x0007f<<30)|(0x00080<<12)|(8<<6))
	
	.align 12
pat2:
  	.dword ((0x00001<<30)|(0x3fffe<<12)|(0<<6))
  	.dword ((0x00002<<30)|(0x3fffd<<12)|(1<<6))
  	.dword ((0x00004<<30)|(0x3fffb<<12)|(2<<6))
  	.dword ((0x00008<<30)|(0x3fff7<<12)|(3<<6))
  	.dword ((0x00010<<30)|(0x3ffef<<12)|(4<<6))
  	.dword ((0x00020<<30)|(0x3ffdf<<12)|(5<<6))
  	.dword ((0x00040<<30)|(0x3ffbf<<12)|(6<<6))
  	.dword ((0x00080<<30)|(0x3ff7f<<12)|(7<<6))

	
RVTEST_DATA_END

