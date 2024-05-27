
.globl _start
.section .text
_exit:
    MOV r7, #1
    MOV r0, #0
    SWI 0
_start:
    MOVS r0, #1
    MVNS r1, #0
    ORR r0, r1, #1
    EOR r0, r0, #1
    AND r0, r0, #1
    BIC r0, r0, #1
    TST r0, #1
    TEQ r0, #1
    ADDS r0, r0, #1
    ADCS r0, r0, #1
    SUBS r0, r0, #1
    RSBS r0, r0, #1
    RSCS r0, r0, #1
    CMP r0, #1
    CMN r0, #1
    MUL r0, r1, r0
    MLA r0, r1, r0, r0
    UMULL r0, r1, r2, r0
    UMLAL r0, r1, r2, r0
    SMULL r0, r1, r2, r0
    LDR r0, [r8]
    LDRH r0, [r8, #4]
    LDR r0, [r8, #2]
    LDRSB r0, [r8]
    LDRSH r0, [r8, #2]
    LDM r8, {r1, r2}
    STR r0, [r8]
    STRH r0, [r8, #4]
    STR r0, [r8, #2]
    STM r8, {r1, r2}
    SWP r0, r1, [r8]
    BL _link
    MRS r0, CPSR
    MSR CPSR_f, r0
    LDR r0, =_test_thumb
    LDR r12, =_test_conditions
    BX r0

_link:
    mov pc, lr

_test_thumb:
    BX r12

_test_conditions:
    cmp r0, #0
    addEQ r0, r0, #1
    addNE r0, r0, #1
    addCS r0, r0, #1
    addCC r0, r0, #1
    addMI r0, r0, #1
    addPL r0, r0, #1
    addVS r0, r0, #1
    addVC r0, r0, #1
    addHI r0, r0, #1
    addLS r0, r0, #1
    addGE r0, r0, #1
    addLT r0, r0, #1
    addGT r0, r0, #1
    addLE r0, r0, #1
    b _exit
.section .data
