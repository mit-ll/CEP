# See LICENSE for license details.

//
// Tony Duong 12/17/202
//
// Two level page table look ups with all virtual page maps to singal physical page 0xBFFFF000
// Use riscv_local.h for PTW
//	
#include "riscv_test.h"
#include "test_macros.h"

# align 12 = page size of 4K

RVTEST_RV64U
RVTEST_CODE_BEGIN

user_code_here:
//
#define EXP      a0
#define ACT      a1
#define SAVE_P   a2
#define ONES     a3
#define CPY_SRC  s2
#define CPY_DST  s3
#define RW_DST   s4	
#define CODE_DAT s5
#define COREID   s10	
#define CUR_P    t0		
#define CUR_A    t1
#define WRDAT    t2
#define RDDAT    t3	
#define CUR_CNT  t4
#define CUR_LOOP t5
#define WRN_RD	 t6

// one page away
#define STACK_TOP (_end + 4096)	
	//
	// s11 = hartID << 10 : bit 11:10 
	//
	slli COREID,s11,10
	//
	// init
	addi ONES,x0,1			; # use as constant 1
	addi CUR_LOOP,x0,4		; # loop = 4 patterns. FIXME: 2-patterns for now = virtual[31:0], [38:32] still under debug!!
	addi WRDAT,x0,1			; # shift-1 = write data pattern
	addi EXP,x0,0			; # EXP = 0
	addi ACT,x0,0			; # ACT = 0
	//

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
	la   RW_DST,SandBox		; # write/read data
	add  RW_DST,RW_DST,COREID       ; # each core has its own sandbox	
	mv   CUR_P, SAVE_P	        ; # start from top
	
sub_loop:
	ld   CUR_A,0(CUR_P)		; # get virtual address point by CUR_P
	or   CUR_A, CUR_A, COREID	; # add coreid to make unquie cache block
	beq  WRN_RD,x0,do_write		; #

do_read:
	la   CPY_SRC,rd_code_marker 	; # setup t copy
	move CPY_DST,CUR_A		
	jal  copy_code_marker		;
	nop				;
	jalr x1,0(CUR_A)		; # ----- READ! -------	
	#ld  RDDAT,0(CUR_A)		; # ----- READ! ------
	add  ACT,ACT,RDDAT		; # add to actual for compare
	j    continue			; # done?
do_write:
	la   CPY_SRC,wr_code_marker 	; # setup t copy
	move CPY_DST,CUR_A		
	jal  copy_code_marker		;
	nop				;	
	jalr x1,0(CUR_A)		; # ----- WRITE! -------
	#sd   WRDAT,0(CUR_A)		; # ----- WRITE! -------
	add  EXP,EXP,WRDAT	    	; # compute expected
	slli WRDAT,WRDAT,1		; # next WRDAT

continue:
	add  CUR_P,CUR_P,8		; # point to next virtual address
	sub  CUR_CNT,CUR_CNT,ONES   	; # decrement
	addi RW_DST,RW_DST,64           ; # next block		
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

	
	.global wr_code_marker
// write and return
	.align 12
wr_code_marker:
	sd   WRDAT,0(RW_DST)	; # ----- WRITE! -------
	fence w,r              	;		
	fence.i              	;			
	ret			;

// read and return	
	.global rd_code_marker
	.align 12	
rd_code_marker:
	ld   RDDAT,0(RW_DST)	; # ----- READ! ------ 
	ret			;	

#
# self-replication code
# copy 16 bytes from CPY_SRC to CPY_DST
#
	.global copy_code_marker
	.align 12	
copy_code_marker:
	ld   CODE_DAT,0(CPY_SRC) 	; # first 8 bytes from SRC
	sd   CODE_DAT,0(CPY_DST)	; # to DST
	addi CPY_SRC,CPY_SRC,8		; # next 8 bytes
	addi CPY_DST,CPY_DST,8		; 
	ld   CODE_DAT,0(CPY_SRC) 	; # next 8 bytes
	sd   CODE_DAT,0(CPY_DST)	; # to DST
	fence w,r              		;		
	fence.i              		;		
	ret				;
	
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
// all maps to the same page , make sure each line hits different cache line
// 11:10 = hart ID
//  9:6  = 16 cache blocks	
//  5:0	 = within cache line
//	
	.align 12
pat1:
	//      Level2     Level1=0 PageAdr   CacheBlock
	.dword ((0x01<<30)|(0x1fe<<21)|(0x1ff<<12)|(0x00<<6)|(0x0<<0))	
	.dword ((0x02<<30)|(0x1fd<<21)|(0x1ff<<12)|(0x01<<6)|(0x0<<0))
	.dword ((0x03<<30)|(0x1fb<<21)|(0x1ff<<12)|(0x02<<6)|(0x0<<0))
	.dword ((0x04<<30)|(0x1f7<<21)|(0x1ff<<12)|(0x03<<6)|(0x0<<0))
	.dword ((0x05<<30)|(0x1ef<<21)|(0x1ff<<12)|(0x04<<6)|(0x0<<0))
	.dword ((0x06<<30)|(0x1df<<21)|(0x1ff<<12)|(0x05<<6)|(0x0<<0))
	.dword ((0x07<<30)|(0x1bf<<21)|(0x1ff<<12)|(0x06<<6)|(0x0<<0))
	.dword ((0x08<<30)|(0x17f<<21)|(0x1ff<<12)|(0x07<<6)|(0x0<<0))
	.dword ((0x09<<30)|(0x1ef<<21)|(0x1ff<<12)|(0x08<<6)|(0x0<<0))
	.dword ((0x0a<<30)|(0x1df<<21)|(0x1ff<<12)|(0x09<<6)|(0x0<<0))	
	.dword ((0x0b<<30)|(0x1bf<<21)|(0x1ff<<12)|(0x0a<<6)|(0x0<<0))
	.dword ((0x0c<<30)|(0x17f<<21)|(0x1ff<<12)|(0x0b<<6)|(0x0<<0))
	.dword ((0x0d<<30)|(0x1ef<<21)|(0x1ff<<12)|(0x0c<<6)|(0x0<<0))
	.dword ((0x0e<<30)|(0x1df<<21)|(0x1ff<<12)|(0x0d<<6)|(0x0<<0))
	.dword ((0x0f<<30)|(0x1bf<<21)|(0x1ff<<12)|(0x0e<<6)|(0x0<<0))
	.dword ((0x10<<30)|(0x17f<<21)|(0x1ff<<12)|(0x0f<<6)|(0x0<<0))

	.align 12
pat2:
	//         L2=1   Level1     PageAdr   CacheBlock	
	.dword ((0xfe<<30)|(0x001<<21)|(0x1ff<<12)|(0x00<<6)|(0x10<<0))	
	.dword ((0xfd<<30)|(0x002<<21)|(0x1ff<<12)|(0x01<<6)|(0x10<<0))
	.dword ((0xfb<<30)|(0x004<<21)|(0x1ff<<12)|(0x02<<6)|(0x10<<0))
	.dword ((0xf7<<30)|(0x008<<21)|(0x1ff<<12)|(0x03<<6)|(0x10<<0))
	.dword ((0xef<<30)|(0x010<<21)|(0x1ff<<12)|(0x04<<6)|(0x10<<0))
	.dword ((0xdf<<30)|(0x020<<21)|(0x1ff<<12)|(0x05<<6)|(0x10<<0))
	.dword ((0xbf<<30)|(0x040<<21)|(0x1ff<<12)|(0x06<<6)|(0x10<<0))
	.dword ((0x7f<<30)|(0x080<<21)|(0x1ff<<12)|(0x07<<6)|(0x10<<0))
	.dword ((0xfe<<30)|(0x100<<21)|(0x1ff<<12)|(0x08<<6)|(0x10<<0))
	.dword ((0xfd<<30)|(0x001<<21)|(0x1ff<<12)|(0x09<<6)|(0x10<<0))	
	.dword ((0xfb<<30)|(0x002<<21)|(0x1ff<<12)|(0x0a<<6)|(0x10<<0))
	.dword ((0xf7<<30)|(0x004<<21)|(0x1ff<<12)|(0x0b<<6)|(0x10<<0))
	.dword ((0xef<<30)|(0x008<<21)|(0x1ff<<12)|(0x0c<<6)|(0x10<<0))
	.dword ((0xdf<<30)|(0x010<<21)|(0x1ff<<12)|(0x0d<<6)|(0x10<<0))
	.dword ((0xbf<<30)|(0x020<<21)|(0x1ff<<12)|(0x0e<<6)|(0x10<<0))
	.dword ((0x7f<<30)|(0x040<<21)|(0x1ff<<12)|(0x0f<<6)|(0x10<<0))


//
// Still under debug
//	
	.align 12
pat3:
	//      Level2     Level1=0 PageAdr   CacheBlock
	.dword ((0x01<<30)|(0x001<<21)|(0x1ff<<12)|(0x00<<6)|(0x20<<0))	
	.dword ((0x02<<30)|(0x002<<21)|(0x1ff<<12)|(0x01<<6)|(0x20<<0))
	.dword ((0x04<<30)|(0x004<<21)|(0x1ff<<12)|(0x02<<6)|(0x20<<0))
	.dword ((0x08<<30)|(0x008<<21)|(0x1ff<<12)|(0x03<<6)|(0x20<<0))
	.dword ((0x10<<30)|(0x010<<21)|(0x1ff<<12)|(0x04<<6)|(0x20<<0))
	.dword ((0x20<<30)|(0x020<<21)|(0x1ff<<12)|(0x05<<6)|(0x20<<0))
	.dword ((0x40<<30)|(0x040<<21)|(0x1ff<<12)|(0x06<<6)|(0x20<<0))
	.dword ((0x80<<30)|(0x080<<21)|(0x1ff<<12)|(0x07<<6)|(0x20<<0))
	.dword ((0x01<<30)|(0x100<<21)|(0x1ff<<12)|(0x08<<6)|(0x20<<0))
	.dword ((0x02<<30)|(0x001<<21)|(0x1ff<<12)|(0x09<<6)|(0x20<<0))	
	.dword ((0x04<<30)|(0x002<<21)|(0x1ff<<12)|(0x0a<<6)|(0x20<<0))
	.dword ((0x08<<30)|(0x004<<21)|(0x1ff<<12)|(0x0b<<6)|(0x20<<0))
	.dword ((0x10<<30)|(0x008<<21)|(0x1ff<<12)|(0x0c<<6)|(0x20<<0))
	.dword ((0x20<<30)|(0x010<<21)|(0x1ff<<12)|(0x0d<<6)|(0x20<<0))
	.dword ((0x40<<30)|(0x020<<21)|(0x1ff<<12)|(0x0e<<6)|(0x20<<0))
	.dword ((0x80<<30)|(0x040<<21)|(0x1ff<<12)|(0x0f<<6)|(0x20<<0))

	.align 12
pat4:
	//        L2=1   Level1     PageAdr   CacheBlock	
	.dword ((0xfe<<30)|(0x1fe<<21)|(0x1ff<<12)|(0x00<<6)|(0x30<<0))	
	.dword ((0xfd<<30)|(0x1fd<<21)|(0x1ff<<12)|(0x01<<6)|(0x30<<0))
	.dword ((0xfb<<30)|(0x1fb<<21)|(0x1ff<<12)|(0x02<<6)|(0x30<<0))
	.dword ((0xf7<<30)|(0x1f7<<21)|(0x1ff<<12)|(0x03<<6)|(0x30<<0))
	.dword ((0xef<<30)|(0x1ef<<21)|(0x1ff<<12)|(0x04<<6)|(0x30<<0))
	.dword ((0xdf<<30)|(0x1df<<21)|(0x1ff<<12)|(0x05<<6)|(0x30<<0))
	.dword ((0xbf<<30)|(0x1bf<<21)|(0x1ff<<12)|(0x06<<6)|(0x30<<0))
	.dword ((0x7f<<30)|(0x17f<<21)|(0x1ff<<12)|(0x07<<6)|(0x30<<0))
	.dword ((0xfe<<30)|(0x1ef<<21)|(0x1ff<<12)|(0x08<<6)|(0x30<<0))
	.dword ((0xfd<<30)|(0x1df<<21)|(0x1ff<<12)|(0x09<<6)|(0x30<<0))	
	.dword ((0xfb<<30)|(0x1bf<<21)|(0x1ff<<12)|(0x0a<<6)|(0x30<<0))
	.dword ((0xf7<<30)|(0x17f<<21)|(0x1ff<<12)|(0x0b<<6)|(0x30<<0))
	.dword ((0xef<<30)|(0x1ef<<21)|(0x1ff<<12)|(0x0c<<6)|(0x30<<0))
	.dword ((0xdf<<30)|(0x1df<<21)|(0x1ff<<12)|(0x0d<<6)|(0x30<<0))
	.dword ((0xbf<<30)|(0x1bf<<21)|(0x1ff<<12)|(0x0e<<6)|(0x30<<0))
	.dword ((0x7f<<30)|(0x17f<<21)|(0x1ff<<12)|(0x0f<<6)|(0x30<<0))
//
// Pattern 1-4
//
	.align 12
SandBox:
	.dword 0
	
RVTEST_DATA_END

