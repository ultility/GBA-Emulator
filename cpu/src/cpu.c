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
        if (check_condition(cpu, instruction))
        {
            cpu_execute_arm_instruction(cpu, instruction);
        }
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
        if (((instruction & (0b1001 << 4)) == (0b1001 << 4)) ||
            ((((instruction >> 21) & 0b1111) > 0b1000) && (((instruction >> 4) & 0b1001) & 0b1000) == 0b1000))
        {
            return MULTIPLY;
        }
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
        arm_psr_transfer(cpu, instruction);
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case SINGLE_DATA_TRANSFER:
        arm_single_data_transfer(cpu, instruction);
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case HDS_DATA_TRANSFER:
        arm_hds_data_transfer(cpu, instruction);
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case BLOCK_DATA_TRANSFER:
        arm_block_data_transfer(cpu, instruction);
        cpu->registers[PC] += sizeof(WORD) / sizeof(BYTE);
        break;
    case SINGLE_DATA_SWAP:
        arm_single_data_swap(cpu, instruction);
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
    uint32_t updated_cpsr = cpu->registers[CPSR];
    int opcode = instruction & OPCODE;
    bool s = (instruction & S) == S;
    int32_t rn = (instruction & Rn) >> 16;
    int32_t rd = (instruction & Rd) >> 12;
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
    int op1 = cpu->registers[rn];
    if ((instruction & I) == I)
    {
        int shift_amount = instruction & ISI;
        shift_amount >>= 8;
        op2 = instruction & nn;
        op2 = shift_immediate(cpu, ROR, shift_amount, op2);
    }
    else
    {
        int rm = instruction & Rm;
        if (rm == 0xF)
        {
            rm = 0;
        }
        else
        {
            rm++;
        }
        op2 = cpu->registers[rm];
        int shift_amount = 0;
        if (instruction & R)
        {
            int rs = instruction & Rs;
            rs >>= 8;
            if (rs == 0xF)
            {
                rs = 0;
            }
            else
            {
                rs++;
            }

            shift_amount = cpu->registers[rs];
        }
        else
        {
            shift_amount = instruction & IsR;
            shift_amount >>= 7;
        }
        int shift_type = (instruction >> 5) & 0b11;
        shift_amount &= 0xFF;
        op2 = shift_immediate(cpu, shift_type, shift_amount, op2);
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
    if (instruction & S)
    {
        if ((int32_t)cpu->registers[rd] < 0)
        {
            updated_cpsr |= N_MASK;
            updated_cpsr &= ~Z_MASK;
        }
        else if (cpu->registers[rd] == 0)
        {
            updated_cpsr |= Z_MASK;
            updated_cpsr &= ~N_MASK;
        }
        cpu->registers[CPSR] = updated_cpsr;
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
    int opcode = instruction & OPCODE;
    opcode >>= 21;
    int rd = (instruction & RD) >> 16;
    int rn = (instruction & RN) >> 12;
    int rs = (instruction & RS) >> 8;
    int rm = (instruction & RM);
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
    if (rm == 0xF)
    {
        rm = PC;
    }
    else
    {
        rm++;
    }
    if (rs == 0xF)
    {
        rs = PC;
    }
    else
    {
        rs++;
    }
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

void arm_psr_transfer(struct cpu *cpu, WORD instruction)
{
    enum
    {
        I = 0b1 << 25,
        PSR = 0b1 << 22,
        OPCODE = 0b1 << 21,
        // For MRS
        RD = 0b1111 << 12,
        // for MSR
        F = 0b1 << 19,
        S = 0b1 << 18,
        X = 0b1 << 17,
        C = 0b1 << 16,
        // I = 0
        RM = 0b1111,
        // I = 1
        SHIFT = 0b1111 << 8,
        IMM = 0b11111111
    };
    uint32_t dst = CPSR;
    if (instruction & PSR)
    {
        switch (cpu->registers[CPSR] & MODE_MASK)
        {
        case USER:
        case SYS:
            break;
        case SVC:
            dst = SPSR_SVC;
            break;
        case ABT:
            dst = SPSR_ABT;
            break;
        case UND:
            dst = SPSR_UND;
            break;
        case FIQ:
            dst = SPSR_FIQ;
            break;
        case IRQ:
            dst = SPSR_IRQ;
            break;
        }
    }
    if (instruction & OPCODE) // MSR
    {
        int op = 0;
        if (instruction & I)
        {
            int shift_amount = instruction & SHIFT;
            shift_amount >>= 8;
            op = shift_immediate(cpu, ROR, shift_amount, instruction & IMM);
        }
        else
        {
            int rm = instruction & RM;
            if (rm == 0xFF)
            {
                op = cpu->registers[PC];
            }
            else
            {
                op = cpu->registers[rm + 1];
            }
        }
        if (instruction & F)
        {
            cpu->registers[dst] &= 0x00FFFFFF;
            cpu->registers[dst] |= op;
        }
        else if (instruction & S)
        {
            cpu->registers[dst] &= 0xFF00FFFF;
            cpu->registers[dst] |= op;
        }
        else if (instruction & X)
        {
            cpu->registers[dst] &= 0xFFFF00FF;
            cpu->registers[dst] |= op;
        }
        else if (instruction & C)
        {
            cpu->registers[dst] &= 0xFFFFFF00;
            cpu->registers[dst] |= op;
        }
    }
    else // MRS
    {
        int rd = instruction & RD;
        rd >>= 12;
        if (rd == 0xFF)
        {
            rd = PC;
        }
        else
        {
            rd++;
        }
        cpu->registers[rd] = cpu->registers[dst];
    }
}

void arm_hds_data_transfer(struct cpu *cpu, WORD instruction)
{
    enum
    {
        P = 0b1 << 24,
        U = 0b1 << 23,
        I = 0b1 << 22,
        // P = 1
        W = 0b1 << 21,
        L = 0b1 << 20,
        RN = 0b1111 << 16,
        RD = 0b1111 << 12,
        UPPER_IMM = 0b1111 << 8,
        OPCODE = 0b11 << 6,
        // I = 0
        RM = 0b1111,
        // I = 1
        LOWER_IMM = 0b1111
    };
    uint32_t rn = instruction & RN;
    uint32_t rd = instruction & RD;
    rn >>= 16;
    rd >>= 12;
    int base = 0;
    if (rn == 0xFF)
    {
        rn = PC;
    }
    else
    {
        rn++;
    }
    if (rd == 0xFF)
    {
        rd = PC;
    }
    else
    {
        rd++;
    }
    base = cpu->registers[rn];
    int offset = 0;
    if (instruction & I)
    {
        offset += (instruction & UPPER_IMM) >> 4;
        offset += instruction & LOWER_IMM;
    }
    else
    {
        int rm = instruction & RM;
        if (rm == 0xFF)
        {
            rm = PC;
        }
        else
        {
            rm++;
        }
        offset = cpu->registers[rm];
    }
    if ((instruction & U) != U)
    {
        offset = -offset;
    }
    if (instruction & P)
    {
        base += offset;
        if (instruction & W)
        {
            cpu->registers[rn] = base;
        }
    }
    if (instruction & L)
    {
        // STR halfword
        cpu->memory[base] = cpu->registers[rd];
    }
    else
    {
        switch ((instruction & OPCODE) >> 5)
        {
        case 0b01: // load unsigned halfword
            cpu->registers[rd] = 0;
            cpu->registers[rd] |= cpu->memory[base];
            cpu->registers[rd] <<= 8;
            cpu->registers[rd] |= cpu->memory[base + 1];
            break;
        case 0b10: // load signed byte
            cpu->registers[rd] = cpu->memory[base];
            break;
        case 0b11: // load signed halfword
            cpu->registers[rd] = cpu->memory[base];
            cpu->registers[rd] <<= 8;
            cpu->registers[rd] |= cpu->memory[base + 1];
            break;
        }
    }
    if ((instruction & P) != P)
    {
        base += offset;
        cpu->registers[rn] = base;
    }
}

void arm_block_data_transfer(struct cpu *cpu, WORD instruction)
{
    enum
    {
        P = 0b1 << 24,
        U = 0b1 << 23,
        S = 0b1 << 22,
        W = 0b1 << 21,
        L = 0b1 << 20,
        RN = 0b1111 << 16,
        RLIST = 0b1111111111111111
    };
    int rn = instruction & RN;
    rn >>= 16;
    rn++;
    int base = cpu->registers[rn];
    int rlist = instruction & RLIST;
    rlist <<= 1;
    rlist |= (rlist >> 16) & 0b1;
    int offset = 0;
    for (int i = rlist; i > 0; i >>= 1)
    {
        if (i & 0b1)
        {
            offset += sizeof(WORD);
        }
    }
    if ((instruction & U) != U)
    {
        offset = -offset;
    }
    if (instruction & P)
    {
        base += offset;
    }
    int count = 0;
    if (instruction & L)
    {
        // LDM
        for (int i = 1; rlist > 0; i++)
        {
            if (rlist & 0b1)
            {
                cpu->registers[i] = 0;
                for (int b = 0; b < sizeof(WORD); b++)
                {
                    cpu->registers[i] <<= __CHAR_BIT__;
                    cpu->registers[i] |= cpu->memory[base + (count * sizeof(WORD)) + b];
                }
                count++;
            }
        }
        if (instruction & S)
        {
            enum cpu_mode mode = cpu->registers[CPSR] & MODE_MASK;
            bool thumb = (cpu->registers[CPSR] & T_MASK) >> T_POS;
            switch (mode)
            {
            case USER:
            case SYS:
                break;
            case SVC:
                cpu->registers[CPSR] = cpu->registers[SPSR_SVC];
                break;
            case ABT:
                cpu->registers[CPSR] = cpu->registers[SPSR_ABT];
                break;
            case UND:
                cpu->registers[CPSR] = cpu->registers[SPSR_UND];
                break;
            case IRQ:
                cpu->registers[CPSR] = cpu->registers[SPSR_IRQ];
                break;
            case FIQ:
                cpu->registers[CPSR] = cpu->registers[SPSR_FIQ];
                break;
            }
            if (thumb)
            {
                cpu->registers[CPSR] |= T_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~T_MASK;
            }
        }
    }
    else
    {
        // STM
        for (int i = 1; rlist > 0; i++)
        {
            if (rlist & 0b1)
            {
                for (int b = sizeof(WORD) - 1; b > 0; b--)
                {
                    cpu->memory[base + (count * sizeof(WORD)) + b] = cpu->registers[i] & (0xFF << (b * __CHAR_BIT__));
                }
                count++;
            }
        }
        if (instruction & S)
        {
            cpu->registers[CPSR] ^= cpu->registers[CPSR] & MODE_MASK;
            cpu->registers[CPSR] |= USER << MODE_POS;
        }
    }
    if ((instruction & P) != P)
    {
        base += offset;
    }
    if (instruction & W)
    {
        cpu->registers[rn] = base;
    }
}

void arm_single_data_swap(struct cpu *cpu, WORD instruction)
{
    enum
    {
        B = 0b1 << 22,
        RN = 0b1111 << 16,
        RD = 0b1111 << 12,
        RM = 0b1111
    };

    int rn = instruction & RN;
    rn >>= 16;
    int rd = instruction & RD;
    rd >>= 12;
    int rm = instruction & RM;
    rn++;
    rd++;
    rm++;
    int rm_value = cpu->registers[rm]; // in case that rm = rd
    if (instruction & B)
    {
        cpu->registers[rd] = cpu->memory[cpu->registers[rn]];
        cpu->registers[rd] &= UINT8_MAX; // for when [rn] is negetive
        rm_value &= UINT8_MAX;
        cpu->memory[cpu->registers[rn]] = rm_value;
    }
    else
    {
        cpu->registers[rd] = 0;
        for (int i = 0; i < sizeof(WORD); i++)
        {
            cpu->registers[rd] <<= 8;
            cpu->registers[rd] |= cpu->memory[cpu->registers[rn] + i];
            cpu->memory[cpu->registers[rn] + i] = rm_value << (i * __CHAR_BIT__);
        }
    }
}

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

void thumb_mov_cmp_add_sub_imm(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b11 << 11,
        RD = 0b111 << 8,
        NN = 0b11111111
    };
    int opcode = instruction & OPCODE;
    opcode >>= 11;
    int rd = instruction & RD;
    rd >>= 8;
    rd++;
    int imm = instruction & NN;
    switch(opcode)
    {
        case 0b00: // MOV
            cpu->registers[rd] = imm;
            if (imm == 0)
            {
                cpu->registers[CPSR] |= Z_MASK;
                cpu->registers[CPSR] &= ~N_MASK;
            }
            else if (imm < 0)
            {
                cpu->registers[CPSR] |= N_MASK;
                cpu->registers[CPSR] &= ~Z_MASK;
            }
            break;
        case 0b01: // CMP
            int result = cpu->registers[rd] - imm;
            cpu->registers[CPSR] &= ~C_MASK;
            if (test_overflow(cpu->registers[rd], -imm))
            {
                cpu->registers[CPSR] |= V_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~V_MASK;
            }
            if (result == 0)
            {
                cpu->registers[CPSR] |= Z_MASK;
                cpu->registers[CPSR] &= ~N_MASK;
            }
            else if (result < 0)
            {
                cpu->registers[CPSR] |= N_MASK;
                cpu->registers[CPSR] &= ~Z_MASK;
            }
        case 0b10: // ADD
            if (test_overflow(cpu->registers[rd], imm))
            {
                cpu->registers[CPSR] |= V_MASK | C_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~(V_MASK | C_MASK);
            }
            cpu->registers[rd] += imm;
            if (cpu->registers[rd] == 0)
            {
                cpu->registers[CPSR] |= Z_MASK;
                cpu->registers[CPSR] &= ~N_MASK;
            }
            else if (cpu->registers[rd] < 0)
            {
                cpu->registers[CPSR] |= N_MASK;
                cpu->registers[CPSR] &= ~Z_MASK;
            }
            break;
        case 0b11: // SUB
            if (test_overflow(cpu->registers[rd], -imm))
            {
                cpu->registers[CPSR] |= V_MASK | C_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~(V_MASK | C_MASK);
            }
            cpu->registers[rd] -= imm;
            if (cpu->registers[rd] == 0)
            {
                cpu->registers[CPSR] |= Z_MASK;
                cpu->registers[CPSR] &= ~N_MASK;
            }
            else if (cpu->registers[rd] < 0)
            {
                cpu->registers[CPSR] |= N_MASK;
                cpu->registers[CPSR] &= ~Z_MASK;
            }
            break;
    }
}

void thumb_add_sub(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b11 << 9,
        RN = 0b111 << 6,
        NN = 0b111 << 6,
        RS = 0b111 << 3,
        RD = 0b111
    };
    int rn = instruction & RN;
    rn >>= 6;
    rn++;
    int imm = instruction & NN;
    imm >>= 6;
    int rs = instruction & RS;
    rs >>= 3;
    rs++;
    int rd = instruction & RD;
    rd++;
    int opcode = instruction & OPCODE;
    switch(opcode >> 9)
    {
        case 0b00: // add reg
            cpu->registers[rd] = cpu->registers[rn] + cpu->registers[rs];
            if (test_overflow(cpu->registers[rs], cpu->registers[rn]))
            {
                cpu->registers[CPSR] |= V_MASK | C_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~(V_MASK | C_MASK);
            }
            break;
        case 0b01: // sub reg
            cpu->registers[rd] = cpu->registers[rn] - cpu->registers[rs];
            if (test_overflow(cpu->registers[rs], -cpu->registers[rn]))
            {
                cpu->registers[CPSR] |= V_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~V_MASK;
            }
            break;
        case 0b10: // add imm
            cpu->registers[rd] = cpu->registers[rn] + imm;
            if (test_overflow(cpu->registers[rs], imm))
            {
                cpu->registers[CPSR] |= V_MASK | C_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~(V_MASK | C_MASK);
            }
            break;
        case 0b11: // sub imm
            cpu->registers[rd] = cpu->registers[rn] - imm;
            if (test_overflow(cpu->registers[rs], -imm))
            {
                cpu->registers[CPSR] |= V_MASK;
            }
            else
            {
                cpu->registers[CPSR] &= ~V_MASK;
            }
            break;
    }
    if (cpu->registers[rd] == 0)
    {
        cpu->registers[CPSR] |= Z_MASK;
        cpu->registers[CPSR] &= ~N_MASK;
    }
    else if (cpu->registers[rd])
    {
        cpu->registers[CPSR] |= N_MASK;
        cpu->registers[CPSR] &= ~Z_MASK;
    }
}

void thumb_move_shifted_register(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b11 << 11,
        OFFSET = 0b11111 << 6,
        RS = 0b111 << 3,
        RD = 0b111
    };
    int opcode = instruction & OPCODE;
    int rs = instruction & RS;
    rs >>= 3;
    rs++;
    int rd = instruction & RD;
    rd++;
    int offset = instruction & OFFSET;
    offset >>= 6;
    cpu->registers[rd] = shift_immediate(cpu, opcode >> 11, cpu->registers[rs], offset);
    if (cpu->registers[rd] == 0)
    {
        cpu->registers[CPSR] |= Z_MASK;
        cpu->registers[CPSR] &= ~N_MASK;
    }
    else if (cpu->registers[rd] < 0)
    {
        cpu->registers[CPSR] |= N_MASK;
        cpu->registers[CPSR] &= ~Z_MASK;
    }
}

bool check_condition(struct cpu *cpu, WORD instruction)
{
    int cond = (instruction >> 28) & 0xF;
    switch (cond)
    {
    case 0x0: // EQ
        return cpu->registers[CPSR] & Z_MASK;
    case 0x1: // NE
        return !(cpu->registers[CPSR] & Z_MASK);
    case 0x2: // CS
        return cpu->registers[CPSR] & C_MASK;
    case 0x3: // CC
        return !(cpu->registers[CPSR] & C_MASK);
    case 0x4: // MI
        return (cpu->registers[CPSR] & N_MASK);
    case 0x5: // PL
        return !(cpu->registers[CPSR] & N_MASK);
    case 0x6: // VS
        return (cpu->registers[CPSR] & V_MASK);
    case 0x7: // VC
        return !(cpu->registers[CPSR] & V_MASK);
    case 0x8: // HI
        return (cpu->registers[CPSR] & C_MASK) && !(cpu->registers[CPSR] & Z_MASK);
    case 0x9: // LS
        return !(cpu->registers[CPSR] & C_MASK) || (cpu->registers[CPSR] & Z_MASK);
    case 0xa: // GE
        return ((cpu->registers[CPSR] & N_MASK) >> N_POS) == ((cpu->registers[CPSR] & V_MASK) >> V_POS);
    case 0xb: // LT
        return ((cpu->registers[CPSR] & N_MASK) >> N_POS) != ((cpu->registers[CPSR] & V_MASK) >> V_POS);
    case 0xc: // GT
        return ((cpu->registers[CPSR] & Z_MASK) == 0) && ((cpu->registers[CPSR] & N_MASK) >> N_POS) == ((cpu->registers[CPSR] & V_MASK) >> V_POS);
    case 0xd: // LE
        return ((cpu->registers[CPSR] & Z_MASK) != 0) || ((cpu->registers[CPSR] & N_MASK) >> N_POS) != ((cpu->registers[CPSR] & V_MASK) >> V_POS);
    case 0xe: // AL
        return true;
    }
}