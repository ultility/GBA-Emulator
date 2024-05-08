#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "data_sizes.h"
#include "instructions.h"

#define STACK_SIZE 1 * KB
#define MEMORY_SIZE STACK_SIZE

#define STACK_START 0

struct request_data
{
    enum {input, output} request_type;
    enum {word, half_word, byte} data_type;
    uint32_t address;
    union
    {
        WORD word;
        HALF_WORD half_word;
        BYTE byte;
    } data;
};

struct request_channel
{
    const char* name;
    int id;
    int memory_address;
    int memory_range;
    void (*push_to_channel)(struct request_data*);
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
};

enum cpsr_masks
{
    MODE_MASK = 0b11111 << MODE_POS,
    T_MASK = 0b1 << T_POS,
    F_MASK = 0b1 << F_POS,
    I_MASK = 0b1 << I_POS,
    A_MASK = 0b1 << A_POS,
    E_MASK = 0b1 << E_POS,
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
    PC,
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
    CPSR,
    R8_FIQ,
    R9_FIQ,
    R10_FIQ,
    R11_FIQ,
    R12_FIQ,
    SP_FIQ,
    LR_FIQ,
    CPSR_FIQ,
    SP_SVC,
    LR_SVC,
    CPSR_SVC,
    SP_ABT,
    LR_ABT,
    CPSR_ABT,
    SP_IRQ,
    LR_IRQ,
    CPSR_IRQ,
    SP_UND,
    LR_UND,
    CPSR_UND,
    register_count
};

struct cpu
{
    uint32_t registers[register_count];
    BYTE memory[MEMORY_SIZE];
    struct request_channel *request_channels;
    int request_channel_count;
    int request_channel_capacity;
};

void cpu_init(struct cpu *cpu);

void free_cpu(struct cpu *cpu);

void cpu_loop(struct cpu *cpu);

void cpu_print_registers(struct cpu *cpu);

void cpu_print_memory(struct cpu *cpu);

void cpu_print_instruction(struct cpu *cpu);

enum arm_instruction_set cpu_decode_arm_instruction(WORD instruction);

WORD cpu_fetch_arm_instruction(struct cpu *cpu);

void cpu_execute_arm_instruction(struct cpu *cpu);

HALF_WORD cpu_fetch_thumb_instruction(struct cpu *cpu);

void cpu_execute_thumb_instruction(struct cpu *cpu);

void arm_single_data_transfer(struct cpu *cpu, WORD instruction);

void add_request_channel(struct cpu *cpu, struct request_channel channel);

void remove_request_channel(struct cpu *cpu, struct request_channel channel);

