#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "instructions.h"

#define MEMORY_SEPERATOR_PADDING sizeof(WORD)
#define BIOS_START 0
#define BIOS_SIZE 16 * KB
#define STACK_START BIOS_START + BIOS_SIZE + MEMORY_SEPERATOR_PADDING
#define STACK_SIZE 1 * KB
#define VRAM_START STACK_START + STACK_SIZE + MEMORY_SEPERATOR_PADDING
#define DISPLAY_LAYERS 1
#define VRAM_SIZE SCREEN_WIDTH *SCREEN_HEIGHT * sizeof(PIXEL) * DISPLAY_LAYERS
#define MAX_CLOCK_SPEED 16.78 * MHz

enum cpsr_mode
{
    MODE_USER = 0b10000,
    MODE_FIQ = 0b10001,
    MODE_IRQ = 0b10010,
    MODE_SVC = 0b10011,
    MODE_ABT = 0b10111,
    MODE_UND = 0b11011,
    MODE_SYS = 0b11111,
};

enum cpsr_bit_pos
{
    THUMB_POS = 5,
    FIQ_POS,
    IRQ_POS,

    V_POS,
    C_POS,
    Z_POS,
    N_POS,
};

enum cspr_masks
{
    MODE_MASK = 0b11111,
    THUMB_MASK = 0b1 << 4,
    FIQ_MASK = 0b1 << 5,
    IRQ_MASK = 0b1 << 6,
    ABORT_MASK = 0b1 << 7,
    ENDIAN_MASK = 0b1 << 8,
    IT_MASK = 0b111111 << 9,
    GE_MASK = 0b1111 << 15,
    JAZZELE_MASK = 0b1 << 23,
    THUMBV2_IT_MASK = 0b11 << 24,
    SATURATION_MASK = 0b1 << 26,
    STATUS_MASK = 0b1111 << 27
};

enum cpsr_alu_flag
{
    FLAG_V = 0b0001,
    FLAG_C = 0b0010,
    FLAG_Z = 0b0100,
    FLAG_N = 0b1000
};

enum registers
{
    r0 = 0,
    r1,
    r2,
    r3,
    r4,
    r5,
    r6,
    r7,
    r8,
    r9,
    r10,
    r11,
    r12,
    sp,
    ra,
    lr = ra,
    pc,
    cpsr,
    sp_svc,
    lr_svc,
    spsr_svc,
    r8_fiq,
    r9_fiq,
    r10_fiq,
    r11_fiq,
    r12_fiq,
    sp_fiq,
    lr_fiq,
    spsr_fiq,
    sp_abt,
    lr_abt,
    spsr_abt,
    sp_irq,
    lr_irq,
    spsr_irq,
    sp_und,
    lr_und,
    spsr_und,
    total_reg_count
};

enum mode_registers
{
    USER_REG_START = r0,
    USER_REG_END = pc,
    SVC_REG_START = sp_svc,
    SVC_REG_END = spsr_svc,
    ABT_REG_START = sp_abt,
    ABT_REG_END = spsr_abt,
    IRQ_REG_START = sp_irq,
    IRQ_REG_END = spsr_irq,
    FIQ_REG_START = r8_fiq,
    FIQ_REG_END = spsr_fiq,
    UND_REG_START = sp_und,
    UND_REG_END = spsr_und,
};

struct cpu
{
    uint32_t registers[total_reg_count];
    BYTE memory[BIOS_SIZE + STACK_SIZE + VRAM_SIZE + MEMORY_SEPERATOR_PADDING * 2];
    bool drawn;
};

void cpu_init(struct cpu *cpu);

void cpu_loop(struct cpu *cpu);

WORD cpu_load_word(struct cpu *cpu, WORD address);

void cpu_save_word(struct cpu *cpu, WORD address, WORD value);

void cpu_save_halfword(struct cpu *cpu, WORD address, HALFWORD value);

HALFWORD cpu_load_halfword(struct cpu *cpu, HALFWORD address);

void cpu_change_mode(struct cpu *cpu, enum cpsr_mode mode);

int decode_arm_instruction(WORD Instruction);

void cpu_execute_thumb_instruction(struct cpu *cpu, HALFWORD instruction);

void cpu_execute_arm_instruction(struct cpu *cpu, WORD instruction);

void arm_branch_and_exchange(struct cpu *cpu, WORD instruction);

void arm_branch(struct cpu *cpu, WORD instruction);

void arm_block_data_transfer(struct cpu *cpu, WORD instruction);

bool check_condition(struct cpu *cpu, uint8_t condition);