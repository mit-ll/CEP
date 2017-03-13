
/* vectors.s */
/*
	
The first interrupt vector position (at 0) should always point to the
startup stack top. When the reset interrupt is reached, it also loads
the stack pointer. (As far as I can say, the latter one is
unnecessary as the HW anyway reloads the SP from the 0th
vector before calling the reset handler.)

The Cortex-M stack pointer should always point to the last item in the
stack. At startup there are no items in the stack and thus the
pointer should point to the first address above the actual
memory, 0x020010000 in this case. With the original linker
script the stack pointer is set to 0x0200ffff, which actually
results in sp = 0x0200fffc (the hardware forces word-aligned
stack). After this the stack is misaligned by 4.

*/
	
;@ .cpu cortex-m0
.thumb

.word   _stacktop   /* stack top address */
.word   _start      /* 1 Reset */
.word   hang        /* 2 NMI */
.word   hang        /* 3 HardFault */
.word   hang        /* 4 MemManage */
.word   hang        /* 5 BusFault */
.word   hang        /* 6 UsageFault */
.word   hang        /* 7 RESERVED */
.word   hang        /* 8 RESERVED */
.word   hang        /* 9 RESERVED*/
.word   hang        /* 10 RESERVED */
.word   hang        /* 11 SVCall */
.word   hang        /* 12 Debug Monitor */
.word   hang        /* 13 RESERVED */
.word   hang        /* 14 PendSV */
.word   hang        /* 15 SysTick */
.word   hang        /* 16 External Interrupt(0) */
.word   hang        /* 17 External Interrupt(1) */
.word   hang        /* 18 External Interrupt(2) */
.word   hang        /* 19 ...   */

.thumb_func
hang:   b .

.thumb_func
.global _start
_start:
    bl main
.global exit
exit:
    swi 1
    @ldr r0,=0xF0000000 ;@ halt
    @str r0,[r0]
    b .
.end
