#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#ifndef __USE_MISC
# define __USE_MISC
#endif
#include "data_sizes.h"
#include "instructions.h"
#include "syscall.h"
#include "requests.h"
#ifndef NULL
    #define NULL 0
#endif

enum memory_sections_size
{
    BIOS_SIZE = 16 * KB,
    STACK_SIZE = 1 * KB,
    MEMORY_SIZE = STACK_SIZE + STACK_SIZE + BIOS_SIZE
};

enum memory_sections
{
    BIOS_START = 0x00000000,
    BIOS_END = BIOS_START + BIOS_SIZE,
    STACK_START = BIOS_END,
    STACK_END = STACK_START + STACK_SIZE,
};

enum virtual_memory_sections
{
    // internal memory
    VIRTUAL_BIOS_START =        0x00000000,
    VIRTUAL_WRAM_BOARD_START =  0x02000000,
    VIRTUAL_WRAM_CHIP_START =   0x03000000,
    VIRTUAL_IO_REGISTERS =      0x04000000,
    // internal display memory
    VIRTUAL_PALLETTE_RAM =      0x05000000,
    VIRTUAL_VRAM =              0x06000000,
    VIRTUAL_OAM =               0x07000000,
    // external memory
    VIRUTAL_ROM_WAIT_STATE_1 =  0x08000000,
    VIRTUAL_ROM_WAIT_STATE_2 =  0x0A000000,
    VIRTUAL_ROM_WAIT_STATE_3 =  0x0C000000,
    VIRUTAL_ROM_SRAM =          0x0E000000,
};

enum shift_type
{
    LSL,
    LSR,
    ASR,
    ROR,
    RCR
};

enum cpsr_bit_positions
{
    MODE_POS = 0,
    T_POS = 5,
    F_POS = 6,
    I_POS = 7,
    A_POS = 8,
    E_POS = 9,
    IT7_2_POS = 10,
    GE_POS = 16,
    J_POS = 24,
    IT1_0_POS = 25,
    Q_POS = 27,
    V_POS = 28,
    C_POS = 29,
    Z_POS = 30,
    N_POS = 31,
    CPSR_CONTROL = 0,
    CPSR_EXTENTION = 8,
    CPSR_STATUS = 16,
    CPSR_FLAGS = 24,
};

enum cpu_mode
{
    USER = 0x10,
    SYS = 0x1F,
    UND = 0x1B,
    SVC = 0x13,
    IRQ = 0x12,
    FIQ = 0x11,
    ABT = 0x17
};

enum cpsr_masks
{
    MODE_MASK = 0b11111 << MODE_POS,
    T_MASK = 0b1 << T_POS,
    F_MASK = 0b1 << F_POS,
    I_MASK = 0b1 << I_POS,
    A_MASK = 0b1 << A_POS,
    E_MASK = 0b1 << E_POS, // 1 == big endian
    IT7_2_MASK = 0b111111 << IT7_2_POS,
    GE_MASK = 0b1111 << GE_POS,
    J_MASK = 0b1 << J_POS,
    IT1_0_MASK = 0b11 << IT1_0_POS,
    Q_MASK = 0b1 << Q_POS,
    V_MASK = 0b1 << V_POS,
    C_MASK = 0b1 << C_POS,
    Z_MASK = 0b1 << Z_POS,
    N_MASK = 0b1 << N_POS,
};

enum cpu_registers
{
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    R12,
    SP,
    LR,
    PC,
    CPSR,
    SP_SYS,
    SPSR_SYS,
    R8_FIQ,
    R9_FIQ,
    R10_FIQ,
    R11_FIQ,
    R12_FIQ,
    SP_FIQ,
    LR_FIQ,
    SPSR_FIQ,
    SP_SVC,
    LR_SVC,
    SPSR_SVC,
    SP_ABT,
    LR_ABT,
    SPSR_ABT,
    SP_IRQ,
    LR_IRQ,
    SPSR_IRQ,
    SP_UND,
    LR_UND,
    SPSR_UND,
    register_count
};

struct cpu
{
    uint32_t registers[register_count];
    BYTE memory[MEMORY_SIZE];
    struct request_channel *request_channels;
    int request_channel_count;
    int request_channel_capacity;
    bool isOn;
};

enum arc_tab_taylor_series
{
    Order_1 = 0xA2F9,
    Order_3 = 0x3651,
    Order_5 = 0x2081,
    Order_7 = 0x16AA,
    Order_9 = 0x0FB6,
    Order_11 = 0x091C,
    Order_13 = 0x0390,
    Order_15 = 0x00A9,
};

enum exception_vector_table{
    RESET_VECTOR = 0x00,
    UNDEFINED_INSTRUCTION_VECTOR = 0x04,
    SOFTWARE_INTERRUPT_VECTOR = 0x08,
    PREFATCH_ABORT_VECTOR = 0x0C,
    DATA_ABORT_VECTOR = 0x10,
    ADDRESS_EXCEEDS_26_BIT_VECTOR = 0x14,
    NORMAL_INTERRUPT_VECTOR = 0x18,
    FAST_INTERRUT_VECTOR = 0x1C,
};

void cpu_init(struct cpu *cpu);

void free_cpu(struct cpu *cpu);

void cpu_loop(struct cpu *cpu);

void cpu_print_registers(struct cpu *cpu);

void cpu_print_memory(struct cpu *cpu);

void cpu_print_instruction(struct cpu *cpu);

enum arm_instruction_set cpu_decode_arm_instruction(WORD instruction);

WORD cpu_fetch_arm_instruction(struct cpu *cpu);

void cpu_execute_arm_instruction(struct cpu *cpu, WORD instruction);

HALF_WORD cpu_fetch_thumb_instruction(struct cpu *cpu);

void cpu_execute_thumb_instruction(struct cpu *cpu);

void arm_branch(struct cpu *cpu, WORD instruction);

void arm_branch_and_exchange(struct cpu *cpu, WORD instruction);

void arm_data_processing(struct cpu *cpu, WORD instruction);

void arm_single_data_transfer(struct cpu *cpu, WORD instruction);

void arm_software_interrupt(struct cpu *cpu, WORD instruction);

void arm_multiply(struct cpu *cpu, WORD instruction);

void arm_psr_transfer(struct cpu *cpu, WORD instruction);

void arm_hds_data_transfer(struct cpu *cpu, WORD instruction);

void arm_block_data_transfer(struct cpu *cpu, WORD instruction);

void arm_single_data_swap(struct cpu *cpu, WORD instruction);

void thumb_software_interrupt(struct cpu *cpu, HALF_WORD instruction);

void thumb_unconditional_branch(struct cpu *cpu, HALF_WORD instruction);

void thumb_conditional_branch(struct cpu *cpu, HALF_WORD instruction);

void thumb_multiple_load_store(struct cpu *cpu, HALF_WORD instruction);

void thumb_long_branch_and_link(struct cpu *cpu, HALF_WORD instruction);

void thumb_offset_stackpointer(struct cpu *cpu, HALF_WORD instruction);

void thumb_push_pop_registers(struct cpu *cpu, HALF_WORD instruction);

void thumb_load_store_halfword(struct cpu *cpu, HALF_WORD instruction);

void thumb_sp_relative_load_store(struct cpu *cpu, HALF_WORD instruction);

void thumb_load_address(struct cpu *cpu, HALF_WORD instruction);

void thumb_load_store_with_offset(struct cpu *cpu, HALF_WORD instruction);

void thumb_load_store_with_reg_offset(struct cpu *cpu, HALF_WORD instruction);

void thumb_load_store_sign_extended_byte_halfword(struct cpu *cpu, HALF_WORD instruction);

void thumb_pc_relative_load(struct cpu *cpu, HALF_WORD instruction);

void thumb_hi_reg_operation_branch_exchange(struct cpu *cpu, HALF_WORD instruction);

void thumb_alu_operations(struct cpu *cpu, HALF_WORD instruction);

void thumb_mov_cmp_add_sub_imm(struct cpu *cpu, HALF_WORD instruction);

void thumb_add_sub(struct cpu *cpu, HALF_WORD instruction);

void thumb_move_shifted_register(struct cpu *cpu, HALF_WORD instruction);

void cpu_switch_mode(struct cpu *cpu, enum cpu_mode mode);

void add_request_channel(struct cpu *cpu, struct request_channel channel);

void remove_request_channel(struct cpu *cpu, struct request_channel channel);

int shift_immediate(struct cpu *cpu, enum shift_type shift_type, int shift_amount, WORD value);

bool test_overflow(int32_t op1, int32_t op2);

bool check_condition(struct cpu *cpu, WORD instruction);

WORD read_word_from_memory(struct cpu *cpu, WORD address);

HALF_WORD read_half_word_from_memory(struct cpu *cpu, WORD address);

void write_word_to_memory(struct cpu *cpu, WORD address, WORD value);

void write_half_word_to_memory(struct cpu *cpu, WORD address, HALF_WORD value);

int update_register(int r1, struct cpu *cpu);