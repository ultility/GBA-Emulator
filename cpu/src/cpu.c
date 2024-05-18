#include "cpu.h"

void cpu_init(struct cpu *cpu)
{
    for (int i = 0; i < register_count; i++)
    {
        cpu->registers[i] = 0;
    }
    cpu->registers[SP] = STACK_START;
    cpu->request_channel_capacity = 0;
    cpu->request_channel_count = 0;
}

void free_cpu(struct cpu *cpu)
{
    if (cpu->request_channel_capacity > 0)
    {
        free(cpu->request_channels);
    }
}

void cpu_loop(struct cpu *cpu)
{
    if ((cpu->registers[CPSR] & T_MASK) == T_MASK)
    {
        // HALF_WORD instruction = cpu_fetch_thumb_instruction(cpu);
        cpu->registers[PC] += sizeof(HALF_WORD) / sizeof(BYTE);
    }
    else
    {
        WORD instruction = cpu_fetch_arm_instruction(cpu);
        cpu_execute_arm_instruction(cpu, instruction);
    }
}

void cpu_print_registers(struct cpu *cpu)
{
    for (int i = R0; i <= R12; i++)
    {
        printf("R%d: %08x\n", i - 1, cpu->registers[i]);
    }
    printf("SP: %08x\n", cpu->registers[SP]);
    printf("LR: %08x\n", cpu->registers[LR]);
    printf("PC: %08x\n", cpu->registers[PC]);
    printf("CPSR: %08x\n", cpu->registers[CPSR]);
}

void cpu_print_memory(struct cpu *cpu)
{
    for (int i = 0; i < MEMORY_SIZE; i += 0x10)
    {
        printf("%x:", i);
        for (int j = 0; j < 4; j++)
        {
            printf("\t");
            for (int k = 0; k < 4; k++)
            {
                printf("%02x", cpu->memory[i + (j * 4) + k]);
            }
        }
        printf("\n");
    }
}

void cpu_print_instruction(struct cpu *cpu)
{
    if ((cpu->registers[CPSR] & T_MASK) == T_MASK)
    {
        printf("Thumb\n");
    }
    else
    {
        WORD instruction = cpu_fetch_arm_instruction(cpu);
        printf("ARM %032b\n", instruction);

        enum arm_instruction_set instruction_type = cpu_decode_arm_instruction(instruction);
        switch (instruction_type)
        {
        case BRANCH:
            printf("\tbranch");
            break;
        case BRANCH_AND_EXCHANGE:
            printf("\tbranch & exchange");
            break;
        case SOFTWARE_INTERRUPT:
            printf("\tsoftware interrupt");
            break;
        case UNDEFINED:
            printf("\tundefined");
            break;
        case DATA_PROCESSING:
            printf("\tdata processing");
            break;
        case MULTIPLY:
            printf("\tmultiply");
            break;
        case PSR_TRANSFER:
            printf("\tpsr transfer");
            break;
        case SINGLE_DATA_TRANSFER:
            printf("\tsingle data transfer");
            break;
        case HDS_DATA_TRANSFER:
            printf("\thds data transfer");
            break;
        case BLOCK_DATA_TRANSFER:
            printf("\tblock data transfer");
            break;
        case SINGLE_DATA_SWAP:
            printf("\tsingle data swap");
            break;
        case COPROCESSOR:
            printf("\tcoprocessor");
            break;
        }
        printf("\n");
    }
}

enum arm_instruction_set cpu_decode_arm_instruction(WORD instruction)
{
    if ((instruction & BRANCH_AND_EXCHANGE_OPCODE_MASK) == BRANCH_AND_EXCHANGE_OPCODE)
    {
        return BRANCH_AND_EXCHANGE;
    }
    if ((instruction & BLOCK_DATA_TRANSFER_OPCODE_MASK) == BLOCK_DATA_TRANSFER_OPCODE)
    {
        return BLOCK_DATA_TRANSFER;
    }
    if ((instruction & BRANCH_OPCODE_MASK) == BRANCH_OPCODE)
    {
        return BRANCH;
    }
    if ((instruction & SOFTWARE_INTERRUPT_OPCODE_MASK) == SOFTWARE_INTERRUPT_OPCODE)
    {
        return SOFTWARE_INTERRUPT;
    }
    if ((instruction & UNDEFINED_OPCODE_MASK) == UNDEFINED_OPCODE)
    {
        return UNDEFINED;
    }
    if ((instruction & SINGLE_DATA_TRANSFER_OPCODE_MASK) == SINGLE_DATA_TRANSFER_OPCODE)
    {
        return SINGLE_DATA_TRANSFER;
    }
    if ((instruction & SINGLE_DATA_SWAP_OPCODE_MASK) == SINGLE_DATA_SWAP_OPCODE)
    {
        return SINGLE_DATA_SWAP;
    }
    if ((instruction & MULTIPLY_OPCODE_MASK) == MULTIPLY_OPCODE)
    {
        return MULTIPLY;
    }
    if ((instruction & HDS_DATA_TRANSFER_OPCODE_MASK) == HDS_DATA_TRANSFER_OPCODE)
    {
        return HDS_DATA_TRANSFER;
    }
    if ((instruction & PSR_TRANSFER_OPCODE_MASK) == PSR_TRANSFER_OPCODE)
    {
        return PSR_TRANSFER;
    }
    if ((instruction & DATA_PROCESSING_OPCODE_MASK) == DATA_PROCESSING_OPCODE)
    {
        return DATA_PROCESSING;
    }
    if ((instruction & COPROCESSOR_OPCODE_MASK) == COPROCESSOR_OPCODE)
    {
        return COPROCESSOR;
    }
    return UNDEFINED;
}

WORD cpu_fetch_arm_instruction(struct cpu *cpu)
{
    WORD instruction = 0;
    if ((cpu->registers[CPSR] & E_MASK) == E_MASK)
    {
        for (int i = 0; i < 4; i++)
        {
            BYTE byte = cpu->memory[cpu->registers[PC] + i];
            instruction |= byte << (i * 8);
        }
    }
    else
    {
        for (int i = 3; i >= 0; i--)
        {
            BYTE byte = cpu->memory[cpu->registers[PC] + i];
            instruction |= byte << (i * 8);
        }
    }
    return instruction;
}

void cpu_execute_arm_instruction(struct cpu *cpu, WORD instruction)
{
    switch (cpu_decode_arm_instruction(instruction))
    {
    case BRANCH:
        arm_branch(cpu, instruction);
        break;
    case BRANCH_AND_EXCHANGE:
        arm_branch_and_exchange(cpu, instruction);
        break;
    case SOFTWARE_INTERRUPT:
        arm_software_interrupt(cpu, instruction);
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case UNDEFINED:
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case DATA_PROCESSING:
        arm_data_processing(cpu, instruction);
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case MULTIPLY:
        arm_multiply(cpu, instruction);
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case PSR_TRANSFER:
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case SINGLE_DATA_TRANSFER:
        arm_single_data_transfer(cpu, instruction);
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case HDS_DATA_TRANSFER:
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case BLOCK_DATA_TRANSFER:
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case SINGLE_DATA_SWAP:
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case COPROCESSOR:
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    }
}

void arm_single_data_transfer(struct cpu *cpu, WORD instruction)
{
    enum
    {
        I = 0b1 << 25,
        P = 0b1 << 24,
        U = 0b1 << 23,
        B = 0b1 << 22,
        T = 0b1 << 21, // when P = 0
        W = 0b1 << 21, // when P = 1
        L = 0b1 << 20,
        Rn = 0b1111 << 16,
        Rd = 0b1111 << 12,
        // when I = 0
        offset = 0b111111111111,
        // when I = 1
        Is = 0b1111 << 7,
        ST = 0b11 << 5,
        Rm = 0b1111 << 0,
    };
    uint32_t sd_reg = (instruction & Rd) >> 12;
    if (sd_reg == 0xF)
    {
        sd_reg = PC;
    }
    else
    {
        sd_reg++;
    }

    uint8_t address_reg = (instruction & Rn) >> 16;
    if (address_reg == 0xF)
    {
        address_reg = PC;
    }
    else
    {
        address_reg++;
    }
    int32_t address_offset = 0;
    uint32_t value = 0;
    uint32_t address = cpu->registers[address_reg];
    struct request_data data = {.data_type = word, .data = 0, .request_type = input};
    if ((instruction & I) != I)
    {
        address_offset = instruction & offset;
    }
    else
    {
        // to be built after alu implementation
        int offset_reg = instruction & Rm;
        int shift_type = instruction & ST;
        shift_type >>= 5;
        int shift_amount = instruction & Is;
        shift_amount >>= 7;
        address_offset = cpu->registers[offset_reg];
        address_offset = shift_immediate(cpu, shift_type, shift_amount, address_offset);
    }
    if ((instruction & U) != U)
    {
        address_offset = -address_offset;
    }
    if ((instruction & P) == 1)
    {
        address += address_offset;
        if ((instruction & W) == W)
        {
            cpu->registers[address_reg] = address;
        }
    }
    if ((instruction & L) == L)
    {
        // LOAD
        if (address >= MEMORY_SIZE)
        {
            for (int i = 0; i < cpu->request_channel_count; i++)
            {
                if (cpu->request_channels[i].memory_address <= address && cpu->request_channels[i].memory_address + cpu->request_channels[i].memory_range > address)
                {
                    data.address = address - cpu->request_channels[i].memory_address;
                    (*(cpu->request_channels[i].push_to_channel))(&data);
                }
            }
            if ((instruction & B) == B)
            {
                cpu->registers[sd_reg] = data.data.byte;
            }
            else
            {
                cpu->registers[sd_reg] = data.data.word;
            }
        }
        else
        {
            if ((instruction & B) == B)
            {
                // only 1 byte
                if ((cpu->registers[CPSR] & E_MASK) == E_MASK)
                {
                    value = cpu->memory[address + (sizeof(WORD) / sizeof(BYTE)) - 1] & 0xFF;
                }
                else
                {
                    value = cpu->memory[address] & 0xFF;
                }
            }
            else
            {
                if ((cpu->registers[CPSR] & E_MASK) == E_MASK)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        value |= cpu->memory[address + i] << (i * 8);
                    }
                }
                else
                {
                    for (int i = 4; i > 0; i--)
                    {
                        value |= cpu->memory[address + i - 1] << ((i - 1) * 8);
                    }
                }
            }
            cpu->registers[sd_reg] = value;
        }
    }
    else
    {
        // STORE
        data.request_type = output;
        if (address >= MEMORY_SIZE)
        {
            if ((instruction & B) == B)
            {
                data.data_type = byte;
                data.data.byte = cpu->registers[sd_reg] & 0xFF;
            }
            else
            {
                data.data.word = cpu->registers[sd_reg];
            }
            for (int i = 0; i < cpu->request_channel_count; i++)
            {
                if (cpu->request_channels[i].memory_address <= address && cpu->request_channels[i].memory_address + cpu->request_channels[i].memory_range > address)
                {
                    data.address = address - cpu->request_channels[i].memory_address;
                    (*(cpu->request_channels[i].push_to_channel))(&data);
                }
            }
        }
        else
        {
            if ((instruction & B) == B)
            {
                // only 1 byte
                if ((cpu->registers[CPSR] & E_MASK) == E_MASK)
                {
                    cpu->memory[address + (sizeof(WORD) / sizeof(BYTE)) - 1] = cpu->registers[sd_reg] & 0xFF;
                }
                else
                {
                    cpu->memory[address] = cpu->registers[sd_reg] & 0xFF;
                }
            }
            else
            {
                value = cpu->registers[sd_reg];
                if ((cpu->registers[CPSR] & E_MASK) == E_MASK)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        cpu->memory[address + i] = value & 0xFF;
                        value >>= 8;
                    }
                }
                else
                {
                    for (int i = 4; i > 0; i--)
                    {
                        cpu->memory[address + i - 1] = value & 0xFF;
                        value >>= 8;
                    }
                }
            }
        }
    }
}

void add_request_channel(struct cpu *cpu, struct request_channel channel)
{
    if (channel.push_to_channel == NULL)
    {
        return;
    }
    if (cpu->request_channel_capacity == 0)
    {
        cpu->request_channels = malloc(sizeof(struct request_channel));
        cpu->request_channel_capacity = 1;
    }
    for (int i = 0; i < cpu->request_channel_count; i++)
    {
        if (cpu->request_channels[i].id == channel.id)
        {
            return;
        }
    }
    if (cpu->request_channel_count == cpu->request_channel_capacity)
    {
        cpu->request_channels = realloc(cpu->request_channels, sizeof(struct request_channel) * (cpu->request_channel_count + 1));
        cpu->request_channel_capacity++;
    }
    cpu->request_channels[cpu->request_channel_count] = channel;
    cpu->request_channel_count++;
}

void remove_request_channel(struct cpu *cpu, struct request_channel channel)
{
    int i = 0;
    for (; i < cpu->request_channel_count; i++)
    {
        if (cpu->request_channels[i].id == channel.id)
        {
            break;
        }
    }
    while (i < cpu->request_channel_count)
    {
        cpu->request_channels[i] = cpu->request_channels[i + 1];
        i++;
    }
    cpu->request_channels[cpu->request_channel_count] = (struct request_channel){0};
    cpu->request_channel_count--;
}

void arm_branch(struct cpu *cpu, WORD instruction)
{
    int32_t opcode = (instruction & 0b1) >> 24;
    int32_t offset = instruction & 0xFFFFFF;
    if (((offset >> 23) & 0b1) == 0b1) // checks if offset should be negative
    {
        offset |= 0xFF000000;
    }
    if (opcode == 1)
    {
        cpu->registers[LR] = cpu->registers[PC] + 4;
    }
    cpu->registers[PC] += 8 + (offset * 4);
}

void arm_branch_and_exchange(struct cpu *cpu, WORD instruction)
{
    int opcode = (instruction >> 4) & 0xF;
    int reg = instruction & 0xF;
    bool thumb = (cpu->registers[CPSR] & T_MASK) == T_MASK;
    if (opcode == 0b0001)
    {
        if (thumb)
        {
            cpu->registers[reg] |= 0b1;
            cpu->registers[PC] = cpu->registers[reg] - 1;
            cpu->registers[CPSR] |= T_MASK;
        }
        else
        {
            cpu->registers[PC] = cpu->registers[reg];
            cpu->registers[CPSR] &= ~T_MASK;
        }
    }
}

void arm_data_processing(struct cpu *cpu, WORD instruction)
{
    enum
    {
        I = 0b1 << 25,
        OPCODE = 0b1111 << 21,
        S = 0b1 << 20,
        Rn = 0b1111 << 16,
        Rd = 0b1111 << 12,
        // if I isnt set
        // if R isnt set
        IsR = 0b11111 << 7,
        // if R is set
        Rs = 0b1111 << 8,
        ST = 0b11 << 5,
        R = 0b1 << 4,
        Rm = 0b1111 << 0,
        // if I is set
        ISI = 0b1111 << 8,
        nn = 0xFF,
    };
    int opcode = instruction & OPCODE;
    bool s = (instruction & S) == S;
    int32_t rn = (instruction & Rn) >> 16;
    int32_t rd = (instruction & Rd) >> 12;
    int op1 = cpu->registers[rn];
    int op2 = 0;
    if (rd == 0xF)
    {
        rd = PC;
    }
    else
    {
        rd++;
    }
    if (rn == 0xF)
    {
        rn = PC;
    }
    else
    {
        rn++;
    }
    if ((instruction & I) == I)
    {
        int shift_amount = instruction & ISI;
        shift_amount >>= 8;
        op2 = instruction & nn;
        op2 = shift_immediate(cpu, ROR, shift_amount, op2);
    }
    switch (opcode >> 21)
    {
    case 0x0: // AND
        cpu->registers[rd] = op1 & op2;
        break;
    case 0x1: // EOR
        cpu->registers[rd] = op1 ^ op2;
        break;
    case 0x2: // SUB
        cpu->registers[rd] = op1 - op2;
        break;
    case 0x3: // RSB
        cpu->registers[rd] = op2 - op1;
        break;
    case 0x4: // ADD
        cpu->registers[rd] = op1 + op2;
        break;
    case 0x5: // ADC
        cpu->registers[rd] = op1 + op2 + ((cpu->registers[CPSR] & C_MASK) == C_MASK);
        break;
    case 0x6: // SBC
        cpu->registers[rd] = op1 - op2 + ((cpu->registers[CPSR] & C_MASK) == C_MASK) - 1;
        break;
    case 0x7: // RSC
        cpu->registers[rd] = op2 - op1 + ((cpu->registers[CPSR] & C_MASK) == C_MASK) - 1;
        break;
    case 0x8: // TST
        if ((op1 & op2) == 0)
        {
            cpu->registers[CPSR] |= Z_MASK;
        }
        else
        {
            cpu->registers[CPSR] &= ~Z_MASK;
            if ((op1 & op2) < 0)
            {
                cpu->registers[CPSR] |= N_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~N_MASK;
            }
        }
        break;
    case 0x9: // TEQ
        if ((op1 ^ op2) == 0)
        {
            cpu->registers[CPSR] |= Z_MASK;
        }
        else
        {
            cpu->registers[CPSR] &= ~Z_MASK;
            if ((op1 ^ op2) < 0)
            {
                cpu->registers[CPSR] |= N_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~N_MASK;
            }
        }
        break;
    case 0xA: // CMP
        if ((op1 & op2) == 0)
        {
            cpu->registers[CPSR] |= Z_MASK;
        }
        else
        {
            cpu->registers[CPSR] &= ~Z_MASK;
            if ((op1 & op2) < 0)
            {
                cpu->registers[CPSR] |= N_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~N_MASK;
            }
        }
        break;
    case 0xB: // CMN
        break;
    case 0xC: // ORR
        cpu->registers[rd] = op1 | op2;
        break;
    case 0xD: // MOV
        cpu->registers[rd] = op2;
        break;
    case 0xE: // BIC
        cpu->registers[rd] = op1 & (~op2);
        break;
    case 0xF: // MVN
        cpu->registers[rd] = ~op2;
        break;
    }
}

int shift_immediate(struct cpu *cpu, enum shift_type shift_type, int shift_amount, WORD value)
{
    /*if (shift_amount == 0)
    {
        switch (shift_type)
        {
        case LSL:
            return value;
        case LSR:
            if (((value >> 31) & 0b1) == 0b1)
            {
                cpu->registers[CPSR] |= C_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~C_MASK;
            }
            return 0;
        case ASR:
            if (((value >> 31) & 0b1) == 0b1)
            {
                cpu->registers[CPSR] |= C_MASK;
                return -1;
            }
            else
            {
                cpu->registers[CPSR] &= ~C_MASK;
                return 0;
            }
        case ROR:
            shift_amount = 1;
            shift_type = RCR;
            break;
        }
    }*/
    switch (shift_type)
    {
    case LSL:
        return value << shift_amount;
    case LSR:
        return value >> shift_amount; // does trigger overflow
    case ASR:
        bool negetive = value < 0;
        value = value >> shift_amount;
        if (negetive)
        {
            for (int i = 0; i < shift_amount; i++)
            {
                value |= 0b1 << (32 - i);
            }
        }
        return value;
    case ROR:
        for (int i = 0; i < shift_amount; i++)
        {
            if ((value & 0b1) == 0b1)
            {
                value >>= 1;
                value |= 0b1 << 31;
            }
            else
            {
                value >>= 1;
                value &= ~(0b1 << 31);
            }
        }
        return value;
    case RCR:
        for (int i = 0; i < shift_amount; i++)
        {
            int c = (cpu->registers[CPSR] & C_MASK) << (31 - C_POS);
            if ((value & 0b1) == 0b1)
            {
                cpu->registers[CPSR] |= C_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~C_MASK;
            }
            value >>= 1;
            value |= c;
        }
        return value;
    }
}

bool test_overflow(int32_t op1, int32_t op2)
{
    // addition overflow
    if (op1 > 0 && op2 > 0 && op1 > (INT32_MAX - op2))
    {
        return true;
    }
    // addition underflow
    else if (op1 < 0 && op2 < 0 && op1 < (INT32_MIN - op2))
    {
        return true;
    }
    return false;
}

void arm_software_interrupt(struct cpu *cpu, WORD instruction)
{
    enum syscall_number syscall = cpu->registers[R7];
    switch (syscall)
    {
    case RESTART_SYSCALL:
        break;
    case EXIT:
        exit(cpu->registers[R0]);
    case FORK:
        break;
    case WRITE:
        break;
    case READ:
        break;
    case OPEN:
    case CLOSE:
    default:
        break;
    }
}

void arm_multiply(struct cpu *cpu, WORD instruction)
{
    enum
    {
        OPCODE = 0b1111 << 21,
        S = 0b1 << 20,
        RD = 0b1111 << 16,
        RN = 0b1111 << 12,
        RS = 0b1111 << 8,
        // for HALFWORD multipliers
        Y = 0b1 << 6,
        X = 0b1 << 5,
        //
        RM = 0b1111
    };
    int opcode = instruction & opcode;
    opcode >>= 21;
    int rd = (instruction & RD) >> 16;
    int rn = (instruction & RN) >> 12;
    int rs = (instruction & RS) >> 8;
    int rm = (instruction & RM);
    int64_t op1 = cpu->registers[rm];
    int64_t op2 = cpu->registers[rs];
    int64_t rd_hi_low = cpu->registers[rd];
    rd_hi_low <<= sizeof(cpu->registers[rd] / sizeof(BYTE) * __CHAR_BIT__);
    rd_hi_low |= cpu->registers[rn];
    int64_t result = 0;
    uint64_t uop1 = cpu->registers[rm];
    uint64_t uop2 = cpu->registers[rs];
    uint64_t urd_hi_low = cpu->registers[rd];
    urd_hi_low <<= sizeof(cpu->registers[rd] / sizeof(BYTE) * __CHAR_BIT__);
    urd_hi_low |= cpu->registers[rn];
    uint64_t uresult = 0;
    if (opcode >= 0b1000)
    {
        if (instruction & Y)
        {
            op1 >>= 16;
        }
        if (instruction & X)
        {
            op2 >>= 16;
        }
    }
    switch (opcode)
    {
    case 0b0000: // MUL
        cpu->registers[rd] = op1 * op2;
        break;
    case 0b0001: // MLA
        cpu->registers[rd] = op1 * op2 + cpu->registers[rn];
        break;
    case 0b0010: // UMAAL
        uresult = uop1 * uop2 + urd_hi_low;
        cpu->registers[rn] = uresult & UINT32_MAX;
        cpu->registers[rd] = (uresult >> 32) & UINT32_MAX;
        break;
    case 0b0100: // UMULL
        uresult = uop1 * uop2;
        cpu->registers[rn] = uresult & UINT32_MAX;
        cpu->registers[rd] = (uresult >> 32) & UINT32_MAX;
        break;
    case 0b0101: // UMLAL
        uresult = uop1 * uop2 + rd_hi_low;
        cpu->registers[rn] = uresult & UINT32_MAX;
        cpu->registers[rd] = (uresult >> 32) & UINT32_MAX;
        break;
    case 0b0110: // SMULL
        result = op1 * op2;
        cpu->registers[rn] = result & UINT32_MAX;
        cpu->registers[rd] = (result >> 32) & UINT32_MAX;
        break;
    case 0b0111: // SMLAL
        result = op1 * op2 + rd_hi_low;
        cpu->registers[rn] = result & UINT32_MAX;
        cpu->registers[rd] = (result >> 32) & UINT32_MAX;
        break;
    case 0b1000: // SMLAL
        result = (op1 & UINT16_MAX) * (op2 & UINT16_MAX) + (rd_hi_low & UINT32_MAX);
        cpu->registers[rn] = result & UINT32_MAX;
        cpu->registers[rd] = (result >> 32) & UINT32_MAX;
        break;
    case 0b1001: // SMLAWx
        result = ((op1 * (op2 & UINT16_MAX)) / 0x10000) + (rd_hi_low & UINT32_MAX);
        cpu->registers[rn] = result & UINT32_MAX;
        cpu->registers[rd] = (result >> 32) & UINT32_MAX;
        break;
    case 0b1010: // SMLALxy
        result = (op1 & UINT16_MAX) * (op2 & UINT16_MAX) + (int32_t)cpu->registers[rn];
        cpu->registers[rn] = result & UINT32_MAX;
        cpu->registers[rd] = (result >> 32) & UINT32_MAX;
        break;
    case 0b1011: // SMULxy
        cpu->registers[rd] = (op1 & UINT16_MAX) * (op2 & UINT16_MAX);
        break;
    }
    if (instruction & S)
    {
        if (result == 0 && uresult == 0)
        {
            cpu->registers[CPSR] |= Z_MASK;
        }
        else
        {
            cpu->registers[CPSR] &= ~Z_MASK;
        }
        if (opcode >= 0b0110)
        {
            if (result < 0)
            {
                cpu->registers[CPSR] |= N_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~N_MASK;
            }
        }
        cpu->registers[CPSR] &= ~C_MASK;
    }
}