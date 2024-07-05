
.globl _start
.section .text
_exit:
    MOV r7, #1
    MOV r0, #0
    SWI 0
_start:
    MOV r0, #-1
    BL _sqrt
    MOV r1, r0
    MOV r0, #4
    BL _sqrt
    MOV r2, r0
    MOV r0, #25
    BL _sqrt
    MOV r3, r0
    MOV r0, #100
    BL _sqrt
    MOV r4, r0
    MOV r0, #1000
    ADD r0, r0
    ADD r0, #500
    BL _sqrt
    MOV r5, r0
    B _exit

_softReset:
_registerRamReset:
_halt:
_stop:
_intrWait:
_vBlankIntrWait:
_div:
    STM SP, {r3,r10,r11, r12}
    ADD SP, #16
    MOV r12, #1
    MOV r10, #-2
    MOV r11, #-1
    CMP r0, #0
    MULMI r0, r11
    EORMI r12, r10
    CMP r1, #0
    MULMI r1, r11
    EORMI r12, r10
    MOV r10, #0
_div_loop:
    CMP r0, r1
    BLT _div_finish
    ADD r10, r12
    ADD r3, #1
    SUB r0, r1
    B _div_loop
_div_finish:
    MOV r1, r0 // MOD
    MOV r0, r10 // result
    SUB SP, #16
    LDM SP, {r3,r10, r11, r12}
    MOV PC, LR
_divARM:
    ADD r0, r1
    SUB r1, r0, r1
    SUB r0, r1
    B _div
_sqrt:
    STM SP, {r1,r3,r10,r11, r12}
    ADD SP, #20
    MOV r12, #0b1100000000000000000000000000
    _shifting_left:
    TST r0, r12
    LSLEQ r0, #2
    BEQ _shifting_left
    MOV r12, #1
    MOV r11, #32
    _log_main_loop:
    SUB r11, #1
    LSL r10, r12, r11
    AND r9, r0, r10
    CMP r9, #0
    BEQ _log_main_loop
    SUB r11, #1
    LSL r10, r12, r11
    TST r0, r10
    SUB r11, #1
    LSL r10, r12, r11
    ADDNE r12, #2
    TST r0, r10
    ADDNE r12, #1
    ADD r11, #2
    LDR r10, =sqrt_correction_starting_point
    SUB r0, r10
    LDR r10, =sqrt_correction_start
    LDR r9, =sqrt_correction_difference
    CMP r0, #0
    BLT _log_correction_loop_negetive
    _log_correction_loop:
    SUBS r0, r9
    ADDPL r10, #1
    BPL _log_correction_loop
    B _log_correction_loop_end
    _log_correction_loop_negetive:
    ADDS r0, r9
    ADDMI r10, #1
    BMI _log_correction_loop_negetive
    _log_correction_loop_end:
    MUL r0, r10, r12
    MUL r12, r10, r11
    MOV r1, #4
    STR LR, [SP]
    ADD SP, #4
    BL _div
    SUB SP, #4
    LDR LR, [SP]
    ADD r0, r12
    LSR r0, #6
    CMP r0, #0x10000
    LSRGT r0, #1
    SUBGT PC, #8
    SUB SP, #20
    STM SP, {r1,r3,r10,r11, r12}
    MOV PC, LR
_arctan:
_arctan2:
_cpuSet:
_cpuFastSet:
_getBiosChecksum:
_bgAffineSetSet:
_objAffineSet:
_bitUnPack:
_lz77UnCompReadNormalWrite8Bit:
_lz77UnCompReadNormalWrite16Bit:
_huffUnCompReadNormal:
_rlUnCompReadNormalWrite8Bit:
_rlUnCompReadNormalWrite16Bit:
_diff8BitUnFilterWrite8Bit:
_diff8BitUnFilterWrite16Bit:
_diff16BitUnFilter:
_soundBias:
_soundDriverInit:
_soundDriverMode:
_soundDriverMain:
_soundDriverVsync:
_soundChannelClear:
_midiKey2Freq:
_soundWhatever0:
_soundWhatever1:
_soundWhatever2:
_soundWhatever3:
_soundWhatever4:
_multiBoot:
_hardReset:
_customHalt:
_soundDriverVSyncOff:
_soundDriverVSyncOn:
_soundGetJumpList:
_crash:
# input: r0 = 32bit number
# output: r0 = 16bit number
_log:
    
.section .data
    sqrt_correction_start: #1050
    sqrt_correction_difference: #1000000
    sqrt_correction_starting_point: #10000000000
