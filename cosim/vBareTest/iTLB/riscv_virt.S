# See LICENSE for license details.

#include "riscv_test.h"
#include "test_macros.h"

# align 12 = page size of 4K
	
#define DO_NXT_PAGE(p,n) \
	.align 12 \
p:	j nf		;

	

RVTEST_RV64U
RVTEST_CODE_BEGIN

user_code_here:	
	j 1f			;
	.align 12
	nop			;
1:	j 2f			;
	.align 12
	nop			;	
2:	j 3f			;
	.align 12
	nop			;	
3:	j 4f			;
	.align 12
4:	j 5f			;
	.align 12
5:	j 6f			;
	.align 12
6:	j 7f			;
	.align 12
7:	j 8f			;
	.align 12
8:	j 9f			;
	.align 12
9:	j 10f			;
	.align 12
10:	j 11f			;
	.align 12
11:	j 12f			;
	.align 12
12:	j 13f			;
	.align 12
13:	j 14f			;
	.align 12
14:	j 15f			;
	.align 12
15:	j 16f			;
	.align 12
16:	j 17f			;
	.align 12
17:	j 18f			;
	.align 12
18:	j 19f			;
	.align 12
19:	j 20f			;
	.align 12
20:	j 21f			;
	.align 12
21:	j 22f			;
	.align 12
22:	j 23f			;
	.align 12
23:	j 24f			;
	.align 12
24:	j 25f			;
	.align 12
25:	j 26f			;
	.align 12
26:	j 27f			;
	.align 12
27:	j 28f			;
	.align 12
28:	j 29f			;
	.align 12
29:	j 30f			;
	.align 12
30:	j 31f			;
	.align 12
	nop			;		
31:	j 32f			;
	.align 12
	nop			;		
32:	j 33f			;
	.align 12
	nop			;			
33:	j 34f			;
	.align 12
34:	j 35f			;
	.align 12
35:	j 36f			;
	.align 12
36:	j 37f			;
	.align 12
37:	j 38f			;
	.align 12
38:	j 39f			;
	.align 12
39:	j 40f			;
	.align 12
40:	j 41f			;
	.align 12
41:	j 42f			;
	.align 12
42:	j 43f			;
	.align 12
43:	j 44f			;
	.align 12
44:	j 45f			;
	.align 12
45:	j 46f			;
	.align 12
46:	j 47f			;
	.align 12
47:	j 48f			;
	.align 12
48:	j 49f			;
	.align 12
49:	j 50f			;
	.align 12
50:	j 51f			;
	.align 12
51:	j 52f			;
	.align 12
52:	j 53f			;
	.align 12
53:	j 54f			;
	.align 12
	nop			;
54:	
# Either pass or timeout
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
	.dword INVERT_MASK
	
RVTEST_DATA_END
