
.global _start
.section .data
    .equ screen_data, 0x5000000
.section .text
_exit:
    MOV r7, #1
    MOV r0, #0
    SWI 0
_start:
    MRS r2, CPSR
    B _exit
_softReset:
_registerRamReset:
_halt:
_stop:
_intrWait:
_vBlankIntrWait:
_div:
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
# output: r0 = 16bit number
_log:
