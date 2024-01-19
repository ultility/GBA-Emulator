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
    if (address >= VRAM_START && address < VRAM_START + VRAM_SIZE)
    {
        cpu->drawn = false;
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

void cpu_change_mode(struct cpu *cpu, enum cpsr_mode mode)
{ /*
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
     }*/
}

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
    if (check_condition(cpu, instruction))
    {
        cpu->registers[pc] += sizeof(WORD);
        return;
    }
    BYTE opcode = instruction & 0b11110000;
    BYTE reg = instruction & 0b1111;
    switch (opcode >> 4)
    {
    case 0b0011:
        cpu->registers[lr] = cpu->registers[pc];
    case 0b0001:
        cpu->registers[pc] = cpu->registers[reg];
        cpu->registers[cpsr] |= THUMB_MASK;
        break;
    case 0b0010:
        cpu->registers[pc] = cpu->registers[reg];
        cpu->registers[cpsr] |= JAZZELE_MASK;
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
    if (check_condition(cpu, instruction))
    {
        cpu->registers[pc] += sizeof(WORD) / sizeof(BYTE);
    }
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

void arm_block_data_transfer(struct cpu *cpu, WORD instruction)
{
    enum opt
    {
        P = 0b10000,
        U = 0b01000,
        S = 0b00100,
        W = 0b00010,
        L = 0b00001,
    };
    if (check_condition(cpu, instruction))
    {
        cpu->registers[pc] += sizeof(WORD);
        return;
    }
    BYTE opt = instruction >> 20 & 0b11111;
    BYTE base_reg = instruction >> 16 & 0b1111;
    WORD base = cpu->registers[base_reg];
    int16_t reg_list = instruction & UINT16_MAX;
    int reg_count = 0;
    for (int i = 0, regs = reg_list; i < sizeof(reg_list) * __CHAR_BIT__; i++)
    {
        if ((regs & 0b1) == 0b1)
        {
            reg_count += 1;
        }
        regs >>= 1;
    }
    int offset = reg_count * sizeof(WORD);
    base += offset;
    if ((opt & S) == S)
    {
    }
    if ((opt & L) == L)
    {
        // load
        for (int i = 0, current_reg = 0, regs = reg_list; i < reg_count;)
        {
            WORD value = cpu_load_word(cpu, base - i * sizeof(WORD));
            while ((regs & 0b1) != 0b1 && regs != 0)
            {
                regs >>= 1;
                current_reg += 1;
            }
            cpu->registers[current_reg] = value;
            i++;
            regs >>= 1;
            current_reg += 1;
        }
    }
    else
    {
        // store
        for (int i = 0, current_reg = 0, regs = reg_list; i < reg_count;)
        {
            while ((regs & 0b1) != 0b1 && regs != 0)
            {
                regs >>= 1;
            }
            WORD value = cpu->registers[current_reg];
            cpu_save_word(cpu, base - i * sizeof(WORD), value);
            i++;
            current_reg += 1;
            regs >>= 1;
        }
    }
    if ((opt & W) == W)
    {
        cpu->registers[base_reg] = base;
    }
}

void arm_single_data_transfer(struct cpu *cpu, WORD instruction)
{
    enum opt
    {
        I = 0b1 << 25,
        B = 0b1 << 22,
        W = 0b1 << 21,
        L = 0b1 << 20,
        RN = 0b1111 << 16,
        RD = 0b1111 << 12,
        IMM_OFFSET = 0b11111111111,
        IS = 0b11111 << 7,
        ST = 0b11 << 5,
        RM = 0b1111,
    };
}
void arm_data_processing(struct cpu *cpu, WORD instruction)
{
    if (!check_condition(cpu, (instruction & 0b1111) << 28))
    {
        cpu->registers[pc] += sizeof(WORD);
        return;
    }
    enum opt
    {
        I = 0b1 << 25,
        OP = 0b1111 << 21,
        S = 0b1 << 20,
        RN = 0b1111 << 16,
        RD = 0b1111 << 12,
        RIS = 0b11111 << 7,
        RS = 0b1111 << 8,
        ST = 0b11 << 5,
        R = 0b1 << 4,
        RM = 0b1111,
        IIS = 0b1111 << 8,
        NN = 0b11111111,
    };
    BYTE opcode = (instruction & OP) >> 21;
    int Rd = (instruction & RD) >> 12;
    int op1 = cpu->registers[(instruction & RN) >> 16];
    int op2 = 0;
    BYTE status = 0;
    printf("\nI: %d\n", instruction & I);
    if ((instruction & I) == I)
    {
        op2 = instruction & NN;
        for (int i = (instruction & IIS) >> 8; i > 0; i--)
        {
            BYTE rotated = op2 & 0b11;
            op2 >>= 2;
            op2 &= ~(0b11 << (sizeof(op2) * __CHAR_BIT__ - 2));
            op2 |= rotated << (sizeof(op2) * __CHAR_BIT__ - 2);
        }
        printf("\n op2: %d\n", op2);
    }
    else
    {
        int shift_amount = 0;
        if ((instruction & R) == R)
        {
            shift_amount = cpu->registers[(instruction & RS) >> 8];
        }
        else
        {
            shift_amount = (instruction & RIS) >> 7;
        }
        switch ((instruction & ST) >> 5)
        {
        case 0b00: // LSL
            op2 <<= shift_amount;
            for (int i = 0; i < shift_amount; i++)
            {
                if ((op2 & (0b1 << (sizeof(op2) * __CHAR_BIT__ - i))) > 0)
                {
                    status |= C_POS >> STATUS_POS;
                }
            }
            break;
        case 0b01: // LSR
            op2 >>= shift_amount;
            for (int i = 0; i < shift_amount; i++)
            {
                op2 &= ~(1 << (sizeof(op2) * __CHAR_BIT__ - i - 1));
            }
            break;
        case 0b10: // ASR
            int sign_bit = op2 & (0b1 << 31);
            op2 >>= shift_amount;
            for (int i = 0; i < shift_amount; i++)
            {
                op2 &= ~(1 << (sizeof(op2) * __CHAR_BIT__ - i - 1));
                op2 |= sign_bit >> i;
            }
            break;
        case 0b11: // ROR
            for (int i = 0; i < shift_amount; i++)
            {
                BYTE rotated = op2 & 0b1;
                op2 >>= 1;
                op2 &= ~(1 << (sizeof(op2) * __CHAR_BIT__ - 1));
                op2 |= rotated << (sizeof(op2) * __CHAR_BIT__ - 1);
            }
            break;
        }
    }
    switch (opcode)
    {
    case 0x0: // AND
        cpu->registers[Rd] = op1 & op2;
        break;
    case 0x1: // EOR
        cpu->registers[Rd] = op1 ^ op2;
        break;
    case 0x2: // SUB
        cpu->registers[Rd] = op1 - op2;
        if (op2 > 0 && op1 < INT32_MIN + op2)
        {
            status |= V_POS >> STATUS_POS;
        }
        break;
    case 0x3: // RSB
        cpu->registers[Rd] = op2 - op1;
        if (op1 > 0 && op2 < INT32_MIN + op1)
        {
            status |= V_POS >> STATUS_POS;
        }
        break;
    case 0x4: // ADD
        cpu->registers[Rd] = op1 + op2;
        if (op2 > 0 && op1 > INT32_MAX - op2)
        {
            status |= V_POS >> STATUS_POS;
        }
        break;
    case 0x5: // ADC
        cpu->registers[Rd] = op1 + op2 + ((cpu->registers[cpsr] >> C_POS) & 0b1);
        if (op2 > 0 && op1 > INT32_MAX - 1 - op2)
        {
            status |= V_POS >> STATUS_POS;
        }
        break;
    case 0x6: // SBC
        cpu->registers[Rd] = op1 - op2 - 1 + ((cpu->registers[cpsr] >> C_POS) & 0b1);
        if (op2 > 0 && op1 < INT32_MIN + op2 + 1 - ((cpu->registers[cpsr] >> C_POS) & 0b1))
        {
            status |= V_POS >> STATUS_POS;
        }
        break;
    case 0x7: // RSC
        cpu->registers[Rd] = op2 - op1 - 1 + ((cpu->registers[cpsr] >> C_POS) & 0b1);
        if (op1 > 0 && op2 < INT32_MIN + op1 + 1 - ((cpu->registers[cpsr] >> C_POS) & 0b1))
        {
            status |= V_POS >> STATUS_POS;
        }
        break;
    case 0x8: // TST
        if ((op1 & op2) == 0)
        {
            status |= Z_POS >> STATUS_POS;
        }
        if ((op1 & op2) < 0)
        {
            status |= N_POS >> STATUS_POS;
        }
        break;
    case 0x9: // TEQ
        if ((op1 ^ op2) == 0)
        {
            status |= Z_POS >> STATUS_POS;
        }
        if ((op1 ^ op2) < 0)
        {
            status |= N_POS >> STATUS_POS;
        }
        break;
    case 0xa: // CMP
        if (op1 == op2)
        {
            status |= Z_POS >> STATUS_POS;
        }
        else if (op1 > op2)
        {
            status |= N_POS >> STATUS_POS;
        }
        break;
    case 0xb: // CMN
        if (op1 == -op2)
        {
            status |= Z_POS >> STATUS_POS;
        }
        if (op1 < -op2)
        {
            status |= N_POS >> STATUS_POS;
        }
        break;
    case 0xc: // ORR
        cpu->registers[Rd] = op1 | op2;
        break;
    case 0xd: // MOV
        cpu->registers[Rd] = op2;
        break;
    case 0xe: // BIC
        cpu->registers[Rd] = op1 & ~op2;
        break;
    case 0xf: // MVN
        cpu->registers[Rd] = ~op2;
        break;
    }
    if ((opcode >= 0x0 && opcode <= 0x1) || (opcode >= 0xc && opcode <= 0xf))
    {
        status |= (cpu->registers[cpsr] & FLAG_V) >> (V_POS - STATUS_POS);
        if (cpu->registers[Rd] == 0)
        {
            status |= Z_POS >> STATUS_POS;
        }
        if (((cpu->registers[Rd] >> 31) & 0b1) == 0b1)
        {
            status |= N_POS >> STATUS_POS;
        }
    }
}
bool check_condition(struct cpu *cpu, uint8_t cond)
{
    uint8_t state = (cpu->registers[cpsr] >> STATUS_POS) & STATUS_MASK;
    switch (cond)
    {
    case EQ:
        return (state & FLAG_Z) == FLAG_Z;
    case NE:
        return (state & FLAG_Z) != FLAG_Z;
    case BEQ:
        return (state & FLAG_C) == FLAG_C;
    case LO:
        return (state & FLAG_C) != FLAG_C;
    case MI:
        return (state & FLAG_N) == FLAG_N;
    case PL:
        return (state & FLAG_N) != FLAG_N;
    case VS:
        return (state & FLAG_V) == FLAG_V;
    case VC:
        return (state & FLAG_V) != FLAG_V;
    case HI:
        return (state & (FLAG_C | FLAG_Z)) == (FLAG_C | ~FLAG_Z);
    case LS:
        return (state & (FLAG_C | FLAG_Z)) == (~FLAG_C | FLAG_Z);
    case GE:
        return ((state & FLAG_N) >> (N_POS - STATUS_POS)) == ((state & FLAG_V) >> (V_POS - STATUS_POS));
    case LT:
        return ((state & FLAG_N) >> (N_POS - STATUS_POS)) != ((state & FLAG_V) >> (V_POS - STATUS_POS));
    case GT:
        return ((state & FLAG_N) >> (N_POS - STATUS_POS)) == ((state & FLAG_V) >> (V_POS - STATUS_POS)) && ((state & FLAG_Z) != FLAG_Z);
    case LE:
        return ((state & FLAG_N) >> (N_POS - STATUS_POS)) != ((state & FLAG_V) >> (V_POS - STATUS_POS)) || ((state & FLAG_Z) == FLAG_Z);
    case AL:
        return true;
    case NV:
    default:
        return false;
    }
}
