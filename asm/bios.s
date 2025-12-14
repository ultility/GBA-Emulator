
.global _start
.section .data
    .equ screen_data, 0x5000000
    .equ sys_stack_start, 0x3007F00
    .equ irq_stack_start, 0x3007FA0
    .equ svc_stack_start, 0x3007FE0
    .equ stack_emptying_start, 0x3007E00
    .equ USER_MODE, 0x10
    .equ SYS_MODE, 0x1F
    .equ UND_MODE, 0x1B
    .equ SVC_MODE, 0x13
    .equ IRQ_MODE, 0x12
    .equ FIQ_MODE, 0x11
    .equ ABT_MODE, 0x17

.section .text
resetException:
    B _resetVector
undefinedInstructionException:
    B _undefinedInstructionVector
softwareInterruptException:
    B _softwareInterruptVector
prefetchAbortException:
    B _prefetchAbortVector
dataAbortException:
    B _dataAbortVector
addressExceeds26BitException:
    B _addressExceeds26BitVector
normalInterruptException:
    B _normalInterruptVector
fastInterruptException:
    B _fastInterruptVector
exit:
    MOV r7, #1
    MOV r0, #0
    SWI 0
_start:
_softReset:
    MSR CPSR_c, #SVC_MODE
    MOV sp, #0x30000
    ORR sp, #0x70
    LSL sp, #8
    ORR sp, #0xFE0
    MOV lr, #0
    MSR SPSR, #0
    MSR CPSR_c, #IRQ_MODE
    MOV sp, #0x30000
    ORR sp, #0x70
    LSL sp, #8
    ORR sp, #0xFA0
    MSR CPSR_c, #SYS_MODE
    MOV sp, #0x30000
    ORR sp, #0x70
    LSL sp, #8
    ORR sp, #0xF00
    MOV r0, #0
    MOV r1, #0x200
    MOV r2, #0x30000
    ORR r2, #0x7E
    LSL r2, #8
_softResetRamClearLoop:
    STR r0, [r2, r1]
    subs r1, #1
    BNE _softResetRamClearLoop
    MOV r1, #0
    MOV r2, #0
    MOV r3, #0
    MOV r4, #0
    MOV r5, #0
    MOV r6, #0
    MOV r7, #0
    MOV r8, #0
    MOV r9, #0
    MOV r10, #0
    MOV r11, #0
    MOV r12, #0
    MOV LR, #0x30000
    ORR LR, #0x70
    LSL LR, #4
    ORR LR, #0xFF
    LSL LR, #4
    ORR LR, #0xA
    LDRB LR, [LR]
    CMP LR, #0x0
    BEQ =0x8000000
    B =0x2000000
_registerRamReset:
_halt:
_stop:
_intrWait:
_vBlankIntrWait:
# input: r0 numerator, r1 denomenator
# output: r0 quotient, r1 reminder, r3 |quotient|
_div:
    STMFD sp!, {r4-r5, lr}
    CMP r1, #0
    BNE div_not_zero
    MOV r0, #-1
    MOV r1, #-1
    MOV r3, #-1

div_not_zero:
_divARM:
_sqrt:
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
# output: r0 = 16bit number`
_log:
_resetVector:
    NOP
_undefinedInstructionVector:
    NOP
_softwareInterruptVector:
    STM SP!, {r7}
    LDR r7, [lr, #-4]
    LSL r7, #8
    LSRS r7, #6
    SUBNE r7, #4
    ADD pc, r7
SWI0:
    B _softReset
    B _registerRamReset
    B _halt
    B _stop
    B _intrWait
    B _vBlankIntrWait
    B _div
    B _divARM
    B _sqrt
    B _arctan
    B _arctan2
    B _cpuSet
    B _cpuFastSet
    B _getBiosChecksum
    B _bgAffineSetSet
    B _objAffineSet
    B _bitUnPack
    B _lz77UnCompReadNormalWrite8Bit
    B _lz77UnCompReadNormalWrite16Bit
    B _huffUnCompReadNormal
    B _rlUnCompReadNormalWrite8Bit
    B _rlUnCompReadNormalWrite16Bit
    B _diff8BitUnFilterWrite8Bit
    B _diff8BitUnFilterWrite16Bit
    B _diff16BitUnFilter
    B _soundBias
    B _soundDriverInit
    B _soundDriverMode
    B _soundDriverMain
    B _soundDriverVsync
    B _soundChannelClear
    B _midiKey2Freq
    B _soundWhatever0
    B _soundWhatever1
    B _soundWhatever2
    B _soundWhatever3
    B _soundWhatever4
    B _multiBoot
    B _hardReset
    B _customHalt
    B _soundDriverVSyncOff
    B _soundDriverVSyncOn
    B _soundGetJumpList
    B _crash
    B _log
_prefetchAbortVector:
    NOP
_dataAbortVector:
    NOP
_addressExceeds26BitVector:
    NOP
_normalInterruptVector:
    NOP
_fastInterruptVector:
    NOP
color_screen_loop:
    MOV r3, #0b11111
    LSL r3, #5
    ADD r3, #0b11111
    LSL r3, #5
    ADD r3, #0b11111
    full_loop:
    MOV r0, #screen_data
    MOV r1, #480
    width_loop:
    MOV r2, #680
    color_loop:
    STR r3, [r0]
    ADD r0, #1 @0X50
    SUBS r2, #1 
    BNE color_loop
    SUBS r1, #1
    BNE width_loop @0X60
    SUBS r3, #1
    BNE full_loop
    B color_screen_loop


_softwareInterruptReturnVector:
    
    MSR spsr_c, 
    LDM SP!, {r7}
    B lr
    