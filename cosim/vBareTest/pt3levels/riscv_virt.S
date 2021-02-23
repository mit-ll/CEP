# See LICENSE for license details.

//
// Tony Duong 12/17/202
//
// Three level page table look ups with dynamic page maps.
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
// must obey the page table rule as specified in multi_vm.c for this to work!!!
//
// MODE: WALK1_0_3LEVELS (vpn2->vpn1->vpn0)
//
// Acceptabe User's VA addressing:
//
//  VPN[2]     VPN[1]    VPN[0]   PageOffset
//         3           2          1
// 876543210_987654321_098765432_109876543210
// 111111111 mmmmmmmmm xxxxxxxxx pppppppppppp : Kernel code (megapage)
// 000000000 000000000 000UUUUUU pppppppppppp : user code (restricted to 64 pages = 256Kbytes)
// 000000011 wwwwwwwww 1011xxxxx pppppppppppp : walk-1 VPN[1] = pattern1
// 000000011 WWWWWWWWW 1100xxxxx pppppppppppp : walk-0 VPN[1] = pattern2
// wwwwwwwww 000000011 1101xxxxx pppppppppppp : walk-1 VPN[2] = pattern3
// WWWWWWWWW 000000011 1110xxxxx pppppppppppp : walk-0 VPN[2] = pattern4
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
	addi CUR_LOOP,x0,2		; # loop = 4 patterns. FIXME: 2-patterns for now = virtual[31:0], [38:32] still under debug!!
	addi WRDAT,x0,1			; # shift-1 = write data pattern
	addi EXP,x0,0			; # EXP = 0
	addi ACT,x0,0			; # ACT = 0

get_pattern:
	addi WRN_RD,x0,0		; # start with Write
	
	la   SAVE_P,pat4          	;
	addi x1,x0,4			;
	beq  CUR_LOOP,x1,start_loop	; # test pat4
	la   SAVE_P,pat3          	;
	addi x1,x0,3			;
	beq  CUR_LOOP,x1,start_loop	; # test pat3
	la   SAVE_P,pat2          	;
	addi x1,x0,2			;
	beq  CUR_LOOP,x1,start_loop	; # test pat2	
	la   SAVE_P,pat1          	; # test pat1
	j    start_loop			;

	.align 12
start_loop:	
	addi CUR_CNT,x0,16		; # walk-1/0 for 16-bits
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
	addi WRN_RD, WRN_RD,1		; # now do read loop
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
	.dword WALK1_0_3LEVELS	

//
// Pattern 1-4
//

// Max is 16 per align12!! (128bytes)	
	.align 12
pat1:
	.dword ((0x3<<30)|(0x001<<21)|(0xb<<17)|(0x00<<12)|(0<<6))	
	.dword ((0x3<<30)|(0x002<<21)|(0xb<<17)|(0x01<<12)|(0<<6))
	.dword ((0x3<<30)|(0x004<<21)|(0xb<<17)|(0x02<<12)|(1<<6))
	.dword ((0x3<<30)|(0x008<<21)|(0xb<<17)|(0x03<<12)|(2<<6))
	.dword ((0x3<<30)|(0x010<<21)|(0xb<<17)|(0x04<<12)|(3<<6))
	.dword ((0x3<<30)|(0x020<<21)|(0xb<<17)|(0x05<<12)|(4<<6))
	.dword ((0x3<<30)|(0x040<<21)|(0xb<<17)|(0x06<<12)|(5<<6))
	.dword ((0x3<<30)|(0x080<<21)|(0xb<<17)|(0x07<<12)|(6<<6))
	.dword ((0x3<<30)|(0x100<<21)|(0xb<<17)|(0x08<<12)|(7<<6))
	//
	.dword ((0x3<<30)|(0x001<<21)|(0xb<<17)|(0x09<<12)|(8<<6))	
	.dword ((0x3<<30)|(0x002<<21)|(0xb<<17)|(0x0a<<12)|(0<<6))
	.dword ((0x3<<30)|(0x004<<21)|(0xb<<17)|(0x0b<<12)|(1<<6))
	.dword ((0x3<<30)|(0x008<<21)|(0xb<<17)|(0x0c<<12)|(2<<6))
	.dword ((0x3<<30)|(0x010<<21)|(0xb<<17)|(0x0d<<12)|(3<<6))
	.dword ((0x3<<30)|(0x020<<21)|(0xb<<17)|(0x0e<<12)|(4<<6))
	.dword ((0x3<<30)|(0x040<<21)|(0xb<<17)|(0x0f<<12)|(5<<6))

	.align 12
pat2:
	.dword ((0x3<<30)|(0x1fe<<21)|(0xc<<17)|(0x01<<12)|(0<<6))
	.dword ((0x3<<30)|(0x1fd<<21)|(0xc<<17)|(0x02<<12)|(1<<6))
	.dword ((0x3<<30)|(0x1fb<<21)|(0xc<<17)|(0x03<<12)|(2<<6))
	.dword ((0x3<<30)|(0x1f7<<21)|(0xc<<17)|(0x04<<12)|(3<<6))
	.dword ((0x3<<30)|(0x1ef<<21)|(0xc<<17)|(0x05<<12)|(4<<6))
	.dword ((0x3<<30)|(0x1df<<21)|(0xc<<17)|(0x06<<12)|(5<<6))
	.dword ((0x3<<30)|(0x1bf<<21)|(0xc<<17)|(0x07<<12)|(6<<6))
	.dword ((0x3<<30)|(0x17f<<21)|(0xc<<17)|(0x08<<12)|(7<<6))
	.dword ((0x3<<30)|(0x0ff<<21)|(0xc<<17)|(0x09<<12)|(8<<6))
	//
	.dword ((0x3<<30)|(0x1fe<<21)|(0xc<<17)|(0x0a<<12)|(0<<6))
	.dword ((0x3<<30)|(0x1fd<<21)|(0xc<<17)|(0x0b<<12)|(1<<6))
	.dword ((0x3<<30)|(0x1fb<<21)|(0xc<<17)|(0x0c<<12)|(2<<6))
	.dword ((0x3<<30)|(0x1f7<<21)|(0xc<<17)|(0x0d<<12)|(3<<6))
	.dword ((0x3<<30)|(0x1ef<<21)|(0xc<<17)|(0x0e<<12)|(4<<6))
	.dword ((0x3<<30)|(0x1df<<21)|(0xc<<17)|(0x0f<<12)|(5<<6))
	.dword ((0x3<<30)|(0x1bf<<21)|(0xc<<17)|(0x10<<12)|(6<<6))	

//
// Still under debug
//	
	.align 12
pat3:
	.dword ((0x3<<21)|(0x001<<30)|(0xd<<17)|(0x00<<12)|(0<<6))	
	.dword ((0x3<<21)|(0x002<<30)|(0xd<<17)|(0x01<<12)|(0<<6))
	.dword ((0x3<<21)|(0x004<<30)|(0xd<<17)|(0x02<<12)|(1<<6))
	.dword ((0x3<<21)|(0x008<<30)|(0xd<<17)|(0x03<<12)|(2<<6))
	.dword ((0x3<<21)|(0x010<<30)|(0xd<<17)|(0x04<<12)|(3<<6))
	.dword ((0x3<<21)|(0x020<<30)|(0xd<<17)|(0x05<<12)|(4<<6))
	.dword ((0x3<<21)|(0x040<<30)|(0xd<<17)|(0x06<<12)|(5<<6))
	.dword ((0x3<<21)|(0x080<<30)|(0xd<<17)|(0x07<<12)|(6<<6))
	.dword ((0x3<<21)|(0x100<<30)|(0xd<<17)|(0x08<<12)|(7<<6))
	//
	.dword ((0x3<<21)|(0x001<<30)|(0xd<<17)|(0x09<<12)|(8<<6))	
	.dword ((0x3<<21)|(0x002<<30)|(0xd<<17)|(0x0a<<12)|(0<<6))
	.dword ((0x3<<21)|(0x004<<30)|(0xd<<17)|(0x0b<<12)|(1<<6))
	.dword ((0x3<<21)|(0x008<<30)|(0xd<<17)|(0x0c<<12)|(2<<6))
	.dword ((0x3<<21)|(0x010<<30)|(0xd<<17)|(0x0d<<12)|(3<<6))
	.dword ((0x3<<21)|(0x020<<30)|(0xd<<17)|(0x0e<<12)|(4<<6))
	.dword ((0x3<<21)|(0x040<<30)|(0xd<<17)|(0x0f<<12)|(5<<6))

	.align 12
pat4:
	.dword ((0x3<<21)|(0x1fe<<30)|(0xe<<17)|(0x01<<12)|(0<<6))
	.dword ((0x3<<21)|(0x1fd<<30)|(0xe<<17)|(0x02<<12)|(1<<6))
	.dword ((0x3<<21)|(0x1fb<<30)|(0xe<<17)|(0x03<<12)|(2<<6))
	.dword ((0x3<<21)|(0x1f7<<30)|(0xe<<17)|(0x04<<12)|(3<<6))
	.dword ((0x3<<21)|(0x1ef<<30)|(0xe<<17)|(0x05<<12)|(4<<6))
	.dword ((0x3<<21)|(0x1df<<30)|(0xe<<17)|(0x06<<12)|(5<<6))
	.dword ((0x3<<21)|(0x1bf<<30)|(0xe<<17)|(0x07<<12)|(6<<6))
	.dword ((0x3<<21)|(0x17f<<30)|(0xe<<17)|(0x08<<12)|(7<<6))
	.dword ((0x3<<21)|(0x0ff<<30)|(0xe<<17)|(0x09<<12)|(8<<6))
	//
	.dword ((0x3<<21)|(0x1fe<<30)|(0xe<<17)|(0x0a<<12)|(0<<6))
	.dword ((0x3<<21)|(0x1fd<<30)|(0xe<<17)|(0x0b<<12)|(1<<6))
	.dword ((0x3<<21)|(0x1fb<<30)|(0xe<<17)|(0x0c<<12)|(2<<6))
	.dword ((0x3<<21)|(0x1f7<<30)|(0xe<<17)|(0x0d<<12)|(3<<6))
	.dword ((0x3<<21)|(0x1ef<<30)|(0xe<<17)|(0x0e<<12)|(4<<6))
	.dword ((0x3<<21)|(0x1df<<30)|(0xe<<17)|(0x0f<<12)|(5<<6))
	.dword ((0x3<<21)|(0x1bf<<30)|(0xe<<17)|(0x10<<12)|(6<<6))	
	
RVTEST_DATA_END

