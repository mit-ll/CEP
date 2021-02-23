# See LICENSE for license details.

#include "riscv_test.h"
#include "test_macros.h"

# align 12 = page size of 4K
	
	
#define ADD_T_DATA(num) \
	.align 12 		; \
tdata ## num: 	.dword (1 << num)

#define ADD_T_DATA2(num) \
	.align 12 		; \
	.dword 0		; \
tdata ## num: 	.dword (1 << num)	

#define ADD_T_EXP(num) \
	.align 12 		; \
texp: 	.dword ((1 << num)-1)

#define ADD_T_ACT(num) \
	.align 12 		; \
tact: 	.dword num


#define ADD2ACT(num) \
	la a0, tdata ## num	; \
	add a1,a0,t6		; \
	sd t6,0(a1)		; \
	ld t1,0(a0)		; \
	sd x0,0(a0)		; \
	add t0,t1,t0		; 


RVTEST_RV64U
RVTEST_CODE_BEGIN

user_code_here:
	# try to restrict to 128 bytes per page due to cut down on memcpy per page
	# preload
	//csrr t6,scounteren	; # t6=hartID
	addi  t6,x0,0x3f	; # bottom of page
	slli t6,t6,6		; # move to cache line
	#
	mv a0, x0		; # clear A0
	la a0, tact		; # load address of act into a0
	lw t0,0(a0)		; # t0 = act
	# add all
	j 1f
	.align 12
1:	
ADD2ACT(0)		;
ADD2ACT(1)		;
ADD2ACT(2)		;
ADD2ACT(3)		;
	j 2f		;
	.align 12
2:
ADD2ACT(4)		;	
ADD2ACT(5)		;
ADD2ACT(6)		;
ADD2ACT(7)		;
	j 3f		;
	.align 12
3:
ADD2ACT(8)		;
ADD2ACT(9)		;	
ADD2ACT(10)		;
ADD2ACT(11)		;
	j 4f		;
	.align 12
4:
ADD2ACT(12)		;
ADD2ACT(13)		;
ADD2ACT(14)		;	
ADD2ACT(15)		;
	j 5f		;
	.align 12
5:	
ADD2ACT(16)		;
ADD2ACT(17)		;
ADD2ACT(18)		;
ADD2ACT(19)		;
	j 6f		;
	.align 12
6:	
ADD2ACT(20)		;
ADD2ACT(21)		;
ADD2ACT(22)		;
ADD2ACT(23)		;
	j 7f		;
	.align 12
7:
ADD2ACT(24)		;	
ADD2ACT(25)		;
ADD2ACT(26)		;
ADD2ACT(27)		;
	j 8f		;
	.align 12
8:	
ADD2ACT(28)		;
ADD2ACT(29)		;
ADD2ACT(30)		;
ADD2ACT(31)		;	
	j 9f		;
	.align 12
9:
ADD2ACT(32) 		;
ADD2ACT(33)		;
ADD2ACT(34)		;
ADD2ACT(35)		;	
	j 10f		;
	.align 12
10:
ADD2ACT(36)		;
ADD2ACT(37)		;
ADD2ACT(38)		;
ADD2ACT(39)		;	
	j 11f		;
	.align 12
11:
ADD2ACT(40)		;
ADD2ACT(41)		;
ADD2ACT(42)		;
ADD2ACT(43)		;	
	j 12f		;
	.align 12
12:
ADD2ACT(44)		;
ADD2ACT(45)		;
ADD2ACT(46)		;
ADD2ACT(47)		;	
	j 13f		;
	.align 12
13:
ADD2ACT(48)		;
ADD2ACT(49)		;
ADD2ACT(50)		;
ADD2ACT(51)		;	
	j 14f		;
	.align 12
14:
ADD2ACT(52)		;
ADD2ACT(53)		;
ADD2ACT(54) 		;
ADD2ACT(55)		;	
	j 15f		;
	.align 12
15:	
	# check
	la a0, texp		; # load address of exp into a0
	ld t1,0(a0)		; # t1 = exp
	# compare t0 and t1
	bne t0,t1, test_fail	; # fail if not equal

#
# Either pass or timeout
#
	.global  test_pass
	.align 2			
test_pass:	
RVTEST_PASS

	.global  test_fail
	.align 2
test_fail:
RVTEST_FAIL	

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN
	.align 3
	.global	vm_flags	
vm_flags:
	.dword 0
//	
ADD_T_DATA2(0)		;
ADD_T_DATA2(1)		;
ADD_T_DATA2(2)		;
ADD_T_DATA(3)		;
ADD_T_DATA(4)		;
ADD_T_DATA(5)		;
ADD_T_DATA(6)		;
ADD_T_DATA(7)		;
ADD_T_DATA(8)		;
ADD_T_DATA(9)		;
ADD_T_DATA(10)		;
ADD_T_DATA(11)		;
ADD_T_DATA(12)		;
ADD_T_DATA(13)		;
ADD_T_DATA(14)		;
ADD_T_DATA(15)		;
ADD_T_DATA(16)		;
ADD_T_DATA(17)		;
ADD_T_DATA(18)		;
ADD_T_DATA(19)		;
ADD_T_DATA(20)		;
ADD_T_DATA(21)		;
ADD_T_DATA(22)		;
ADD_T_DATA(23)		;
ADD_T_DATA(24)		;
ADD_T_DATA(25)		;
ADD_T_DATA(26)		;
ADD_T_DATA(27)		;
ADD_T_DATA(28)		;
ADD_T_DATA(29)		;
ADD_T_DATA2(30)		;
ADD_T_DATA2(31)		;
ADD_T_DATA2(32)		;
ADD_T_DATA(33)		;
ADD_T_DATA(34)		;
ADD_T_DATA(35)		;
ADD_T_DATA(36)		;
ADD_T_DATA(37)		;
ADD_T_DATA(38)		;
ADD_T_DATA(39)		;
ADD_T_DATA(40)		;
ADD_T_DATA(41)		;
ADD_T_DATA(42)		;
ADD_T_DATA(43)		;
ADD_T_DATA(44)		;
ADD_T_DATA(45)		;
ADD_T_DATA(46)		;
ADD_T_DATA(47)		;
ADD_T_DATA(48)		;
ADD_T_DATA(49)		;
ADD_T_DATA(50)		;
ADD_T_DATA(51)		;
ADD_T_DATA(52)		;
ADD_T_DATA(53)		;
ADD_T_DATA(54)		;
ADD_T_DATA(55)		;
#
#
#
ADD_T_EXP(56)		;
ADD_T_ACT(0)		;
	
RVTEST_DATA_END
