
.globl _start
.section .text
_exit:
    MOV r7, #1
    MOV r0, #0
    SWI 0
_start:
    MOV r1, #1
    ADD r4, r1, #5
    ADD r4, r1, r2
    ADD r4, r1, r1
    b _exit

.section .data
