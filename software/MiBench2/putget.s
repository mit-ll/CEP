
;@-----------------------
.cpu cortex-m0
.thumb
;@-----------------------
.thumb_func
.globl putchar
putchar:
    push {r7, lr}
    add r7, sp, #0
    movs r2, #0xE0
    lsl r2, r2, #24
    str r0, [r2, #0]
    mov sp, r7
    pop {r7, pc}
;@-----------------------
.thumb_func
.globl PUT32
PUT32:
    str r1,[r0]
    bx lr
;@-----------------------
.thumb_func
.globl GET32
GET32:
    ldr r0,[r0]
    bx lr
;@-----------------------
.thumb_func
.globl PUTGETCLR
PUTGETCLR:
    ldr r2,[r0]
    bic r2,r1
    str r2,[r0]
    bx lr
;; @-----------------------
.thumb_func
.globl atest
atest:
    add r2,r0,r1
    mov r0,#0
    bvc atest1
    mov r3,#1
    orr r0,r3
atest1:
    bcc atest2
    mov r3,#2
    orr r0,r3
atest2:
    bx lr
;; @-----------------------
.thumb_func
.globl stest
stest:
    sub r2,r0,r1
    mov r0,#0
    bvc stest1
    mov r3,#1
    orr r0,r3
stest1:
    bcc stest2
    mov r3,#2
    orr r0,r3
stest2:
    bx lr
.thumb_func
returnvcflags:
    @swi 0xCC
    @bx lr

    bvc 1f
    bcc 2f
    ;@ v set   c set
    mov r0,#3
    lsl r0,#28
    bx lr
1:
    ;@ v clear
    bcc 3f
    ;@ v clear c set
    mov r0,#2
    lsl r0,#28
    bx lr

2:  ;@ v set   c clear
    mov r0,#1
    lsl r0,#28
    bx lr

3:  ;@ v clear c clear
    mov r0,#0
    lsl r0,#28
    bx lr
.thumb_func
.globl testfun1
testfun1:
    ;@ clear carry flag
    mov r2,#0
    ror r2,r2
    adc r0,r1
    b returnvcflags
.thumb_func
.globl testfun2
testfun2:
    ;@ set carry flag
    mov r2,#1
    ror r2,r2
    adc r0,r1
    b returnvcflags
.thumb_func
.globl testfun3
testfun3:
    ;@ clear carry flag
    mov r2,#0
    ror r2,r2
    sbc r0,r1
    b returnvcflags
.thumb_func
.globl testfun4
testfun4:
    ;@ set carry flag
    mov r2,#1
    ror r2,r2
    sbc r0,r1
    b returnvcflags
;;@-----------------------
;.thumb_func
;.globl PUTGETSET
;PUTGETSET:
;    ldr r2,[r0]
;    orr r2,r1
;    str r2,[r0]
;    bx lr
;;@-----------------------
.end
;@-----------------------
