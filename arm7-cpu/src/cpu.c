#include "include/cpu.h"

void cpu_init(struct cpu *cpu)
{
    for (int i = 0; i < total_reg_count; i++)
    {
        cpu->registers[i] = 0;
    }
    cpu->drawn = false;
}

void cpu_loop(struct cpu *cpu)
{
    if ((cpu->registers[cpsr] & THUMB_MASK) == THUMB_MASK)
    {
        HALFWORD instruction = cpu_load_halfword(cpu, cpu->registers[pc]);

        cpu_execute_thumb_instruction(cpu, instruction);
    }
    else
    {
        WORD instruction = cpu_load_word(cpu, cpu->registers[pc]);

        cpu_execute_arm_instruction(cpu, instruction);
    }
}

WORD cpu_load_word(struct cpu *cpu, WORD address)
{
    WORD memory = cpu->memory[address + 3];
    memory <<= 8;
    memory |= cpu->memory[address + 2];
    memory <<= 8;
    memory |= cpu->memory[address + 1];
    memory <<= 8;
    memory |= cpu->memory[address];
    return memory;
}

void cpu_save_word(struct cpu *cpu, WORD address, WORD value)
{
    for (int i = 0; i < sizeof(value); i++)
    {
        cpu->memory[address + i] = value >> sizeof(BYTE) * i * __CHAR_BIT__;
    }
}

HALFWORD cpu_load_halfword(struct cpu *cpu, HALFWORD address)
{
    HALFWORD memory = cpu->memory[address + 1];
    memory <<= sizeof(BYTE) * __CHAR_BIT__;
    memory |= cpu->memory[address];
    return memory;
}

void cpu_save_halfword(struct cpu *cpu, WORD address, HALFWORD value)
{
    for (int i = 0; i < 2; i++)
    {
        cpu->memory[address + i] = value >> sizeof(BYTE) * i * __CHAR_BIT__;
        printf("byte %d saved as %u\n", i, cpu->memory[address + i]);
    }
}

/*void cpu_change_mode(struct cpu *cpu, enum cpsr_mode mode)
{
    enum cpsr_mode current_mode = cpu->registers[cpsr] & MODE_MASK;
    if (current_mode != MODE_USER)
    {
        cpu_change_mode(cpu, MODE_USER);
    }
    switch (mode)
    {
    case MODE_USER:
    {
        int reg_start = 0;
        int reg_end = 0;
        switch (current_mode)
        {
        case MODE_SVC:
            reg_start = SVC_REG_START;
            reg_end = SVC_REG_END;
            break;
        }
        for (int i = 0; i < reg_end - reg_start; i++)
        {
            cpu->registers[USER_REG_END - i] =
        }
    }
    }
}*/

void cpu_execute_arm_instruction(struct cpu *cpu, WORD instruction)
{
    switch (decode_arm_instruction(instruction))
    {
    case BRANCH_AND_EXCHANGE:
        arm_branch_and_exchange(cpu, instruction);
        break;
    case BLOCK_DATA_TRANSFER:
        arm_block_data_transfer(cpu, instruction);
        break;
    case BRANCH:
        arm_branch(cpu, instruction);
        break;
    }
}

int decode_arm_instruction(WORD instruction)
{
    if ((instruction & BRANCH_AND_EXCHANGE_MASK) == BRANCH_AND_EXCHANGE_FORMAT)
    {
        return BRANCH_AND_EXCHANGE;
    }
    if ((instruction & BLOCK_DATA_TRANSFER_MASK) == BLOCK_DATA_TRANSFER_FORMAT)
    {
        return BLOCK_DATA_TRANSFER;
    }
    if ((instruction & BRANCH_MASK) == BRANCH_FORMAT)
    {
        return BRANCH;
    }
    if ((instruction & SOFTWARE_INTERRUPT_MASK) == SOFTWARE_INTERRUPT_FORMAT)
    {
        return SOFTWARE_INTERRUPT;
    }
    if ((instruction & UNDEFINED_MASK) == UNDEFINED_FORMAT)
    {
        return UNDEFINED;
    }
    if ((instruction & SINGLE_DATA_TRANSFER_MASK) ==
        SINGLE_DATA_TRANSFER_FORMAT)
    {
        return SINGLE_DATA_TRANSFER;
    }
    if ((instruction & SINGLE_DATA_SWAP_MASK) == SINGLE_DATA_SWAP_FORMAT)
    {
        return SINGLE_DATA_SWAP;
    }
    if ((instruction & MULTIPLY_MASK) == MULTIPLY_FORMAT)
    {
        return MULTIPLY;
    }
    if ((instruction & HALFWORD_DATA_TRANSFER_MASK) ==
        HALFWORD_DATA_TRANSFER_FORMAT)
    {
        return HALFWORD_DATA_TRANSFER;
    }
    if ((instruction & PSR_TRANSFER_MASK) == PSR_TRANSFER_FORMAT)
    {
        return PSR_TRANSFER;
    }
    if ((instruction & DATA_PROCCESSING_MASK) == DATA_PROCCESSING_FORMAT)
    {
        return DATA_PROCCESSING;
    }
    return UNIMPLEMENTED_INSTRUCTION;
}

void cpu_execute_thumb_instruction(struct cpu *cpu, HALFWORD instruction)
{
    if ((instruction & STATUS_MASK) != (cpu->registers[cpsr] & STATUS_MASK))
    {
        return;
    }
}

void arm_branch_and_exchange(struct cpu *cpu, WORD instruction)
{
    if ((instruction & STATUS_MASK) != (cpu->registers[cpsr] & STATUS_MASK))
    {
        cpu->registers[pc] += sizeof(WORD);
        return;
    }
    BYTE opcode = instruction & 0b11110000;
    BYTE reg = instruction & 0b1111;
    switch (opcode >> 4)
    {
    case 0b0011:
        cpu->registers[lr] = cpu->registers[pc] + (sizeof(WORD) * 2);
    case 0b0001:
        cpu->registers[pc] = cpu->registers[reg];
        cpu->registers[cpsr] |= THUMB_MASK;
        break;
    case 0b0010:
        cpu->registers[pc] = cpu->registers[reg];
        cpu->registers[cpsr] &= JAZZELE_MASK;
        break;
    }
}

void arm_software_interrupt(struct cpu *cpu, WORD instruction)
{
    BYTE opcode = instruction >> 24;
    opcode &= 0b1111;
    switch (opcode)
    {
    case 0b1111: // SWI
    {
        if ((instruction & STATUS_MASK) != (cpu->registers[cpsr] & STATUS_MASK))
        {
            cpu->registers[pc] += sizeof(WORD);
            return;
        }
        cpu_change_mode(cpu, MODE_SVC);
        // implement software interrupt mode
    }
    case 0b0001: // BKPT
        if ((instruction >> 20 & 0b1111) == 0b0010 &&
            (instruction >> 4 & 0b1111) == 0b0111)
        {
            // implement arm cpu debugging
        }
    }
}

void arm_branch(struct cpu *cpu, WORD instruction)
{
    BYTE opcode = instruction >> 24 & 0b1;
    // getting rid of the top 8 bits
    int32_t offset = instruction << 8;
    offset >>= 8;
    switch (opcode)
    {
    case 0b0: // no link
        cpu->registers[pc] += offset;
        break;
    case 0b1: // link
        cpu->registers[lr] = cpu->registers[pc];
        cpu->registers[pc] += offset;
        break;
    }
}

bool check_condition(struct cpu *cpu, uint8_t cond)
{
    uint8_t state = cpu->registers[cpsr] & MODE_MASK;
    switch (cond)
    {
    case EQ:
        return state & FLAG_Z;
    case NE:
        return !(state & FLAG_Z);
    case BEQ:
        return state & FLAG_C;
    case LO:
        return !(state & FLAG_C);
    case MI:
        return state & FLAG_N;
    case PL:
        return !(state & FLAG_N);
    case VS:
        return state & FLAG_V;
    case VC:
        return !(state & FLAG_V);
    case HI:
        return state & (FLAG_C | FLAG_Z);
    case LS:
        return state & (FLAG_C | FLAG_Z);
    case GE:
        return (state & FLAG_N) == (state & FLAG_V);
    case LT:
        return (state & FLAG_N) != (state & FLAG_V);
    case GT:
        return (state & FLAG_N) == (state & FLAG_V) && !(state & FLAG_Z);
    case LE:
        return (state & FLAG_N) != (state & FLAG_V) && (state & FLAG_Z);
    case AL:
        return true;
    case NV:
    default:
        return false;
    }
}