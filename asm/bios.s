
.globl _start
.section .text

_start:
    MOV r1, #2
    MOV r1, #1
    MOV r1, #1
    mov r1, #1
    mov r1, #1
    mov r1, #1
    mov r1, #1
    mov r1, #1
    mov r1, #1
    b _exit
_exit:
    MOV r7, #1
    MOV r7, #0
    SWI 0
.section .data
