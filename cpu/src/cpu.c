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
    cpu->registers[CPSR] |= E_MASK;
    cpu->isOn = true;
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
    if ((instruction & PSR_TRANSFER_OPCODE_MASK) == PSR_TRANSFER_OPCODE)
    {
        return PSR_TRANSFER;
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
    if ((instruction & DATA_PROCESSING_OPCODE_MASK) == DATA_PROCESSING_OPCODE)
    {
        return DATA_PROCESSING;
    }
    return UNDEFINED;
}

WORD cpu_fetch_arm_instruction(struct cpu *cpu)
{
    WORD instruction = read_word_from_memory(cpu, cpu->registers[PC]);
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
                value = read_word_from_memory(cpu, address);
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
                write_word_to_memory(cpu, address, value);
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
    if (opcode >> 21 >= 0x8 && opcode >> 21 <= 0xB && !s) { // if TEQ, TST, CMP, CMN and S bit is 0 move to PSR
        arm_psr_transfer(cpu, instruction);
        return;
    }
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
    switch (cpu->registers[CPSR] & MODE_MASK) {
        case USER: // uses regular registers
            break;
        case SYS: // uses SPSR_SYS and SP_SYS
            updated_cpsr = cpu->registers[SPSR_SYS];
            break;
        case UND:
            updated_cpsr = cpu->registers[SPSR_UND];
            break;
        case SVC:
            updated_cpsr = cpu->registers[SPSR_SVC];
            break;
        case IRQ:
            updated_cpsr = cpu->registers[SPSR_IRQ];
            break;
        case FIQ:
            updated_cpsr = cpu->registers[SPSR_FIQ];
            break;
        case ABT:

            break;
    }
    int op1 = cpu->registers[rn];
    if ((instruction & I) == I)
    {
        int shift_amount = instruction & ISI;
        shift_amount >>= 7;
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
    if (s)
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
    enum gba_syscall_number syscall = cpu->registers[R7];
    switch (syscall)
    {
        case EXIT:
            cpu->isOn = false;
            break;
        case DIV_ARM: // swi 7
            cpu->registers[R0] = cpu->registers[R0] * cpu->registers[R1];
            cpu->registers[R1] = cpu->registers[R0] / cpu->registers[R1];
            cpu->registers[R0] = cpu->registers[R0] / cpu->registers[R1];
        case DIV: // swi 6
            int result = cpu->registers[R0] / cpu->registers[R1];
            cpu->registers[R3] = cpu->registers[R0] - (cpu->registers[R1] * result);
            cpu->registers[R0] = result;
            cpu->registers[R0] = result & INT32_MAX;
            break;
        case SQRT: // swi 8
            int correction = 0;
            while(!(cpu->registers[R0] & (0b11 << 30)))
            {
                cpu->registers[R0] <<= 2;
                correction++;
            }
            cpu->registers[R0] = (uint32_t)(cpu->registers[R0]);
            cpu->registers[R0] >>= correction;
            break;
        case ARCTAN: // swi 9
            cpu->registers[R0] = (uint32_t)(atan((int32_t)cpu->registers[R0] & UINT16_MAX));
            break;
        case ARCTAN2: // swi 10
            int x = cpu->registers[R0];
            int y = cpu->registers[R1];
            if (x >= 0)
            {
                cpu->registers[R0] = (uint32_t)(atan(y/x));
            }
            else
            {
                cpu->registers[R0] = (uint32_t)(M_PI - atan(abs(y/x)));
                if (y < 0)
                {
                    cpu->registers[R0] *= -1;
                }
            }
            break;
        case BG_AFFINE_SET:
            int offset = 0;
            int32_t og_x = read_word_from_memory(cpu, cpu->registers[R0]);
            offset += sizeof(og_x);
            int32_t og_y = read_word_from_memory(cpu, cpu->registers[R0] + offset);
            offset += sizeof(og_y);
            int16_t camera_x = read_half_word_from_memory(cpu, cpu->registers[R0] + offset);
            offset += sizeof(camera_x);
            int16_t camera_y = read_half_word_from_memory(cpu, cpu->registers[R0] + offset);
            offset += sizeof(camera_y);
            int16_t scale_x = read_half_word_from_memory(cpu, cpu->registers[R0] + offset);
            offset += sizeof(scale_x);
            int16_t scale_y = read_half_word_from_memory(cpu, cpu->registers[R0] + offset);
            offset += sizeof(scale_y);
            uint16_t rotation = read_half_word_from_memory(cpu, cpu->registers[R0] + offset);
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
            cpu->registers[dst] |= op << CPSR_FLAGS;
        }
        else if (instruction & S)
        {
            cpu->registers[dst] &= 0xFF00FFFF;
            cpu->registers[dst] |= op << CPSR_STATUS;
        }
        else if (instruction & X)
        {
            cpu->registers[dst] &= 0xFFFF00FF;
            cpu->registers[dst] |= op << CPSR_EXTENTION;
        }
        else if (instruction & C)
        {
            cpu->registers[dst] &= 0xFFFFFF00;
            cpu->registers[dst] |= op << CPSR_CONTROL;
        }
    }
    else // MRS
    {
        int rd = instruction & RD;
        rd >>= 12;
        if (rd == 0xF)
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
                cpu->registers[i] = read_word_from_memory(cpu, base + (count * sizeof(WORD)));
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
                write_word_to_memory(cpu, base + (count * sizeof(WORD)), cpu->registers[i]);
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
    if (rn == 0xF) {
        arm_psr_transfer(cpu, instruction);
    }
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
        cpu->registers[rd] = read_word_from_memory(cpu, cpu->registers[rn]);
        write_word_to_memory(cpu, cpu->registers[rn], rm_value);
    }
}

void thumb_software_interrupt(struct cpu *cpu, HALF_WORD instruction)
{
}

void thumb_unconditional_branch(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        NN = 0b11111111111
    };
    int nn = (int16_t)(instruction & NN);
    cpu->registers[PC] += nn << 1;
}

void thumb_conditional_branch(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b1111 << 8,
        NN = 0b11111111
    };
    bool branch = false;
    int opcode = instruction & OPCODE;
    int nn = (int8_t)(instruction & NN);

    switch (opcode >> 8)
    {
    case 0x0: // BEQ
        branch = cpu->registers[CPSR] & Z_MASK;
        break;
    case 0x1: // BNE
        branch = !(cpu->registers[CPSR] & Z_MASK);
        break;
    case 0x2: // BCS
        branch = cpu->registers[CPSR] & C_MASK;
        break;
    case 0x3: // BCC
        branch = !(cpu->registers[CPSR] & C_MASK);
        break;
    case 0x4: // BMI
        branch = cpu->registers[CPSR] & N_MASK;
        break;
    case 0x5: // BPL
        branch = !(cpu->registers[CPSR] & N_MASK);
        break;
    case 0x6: // BVS
        branch = cpu->registers[CPSR] & V_MASK;
        break;
    case 0x7: // BVC
        branch = !(cpu->registers[CPSR] & V_MASK);
        break;
    case 0x8: // BHI
        branch = cpu->registers[CPSR] & C_MASK && !(cpu->registers[CPSR] & Z_MASK);
        break;
    case 0x9: // BLS
        branch = !(cpu->registers[CPSR] & C_MASK) || (cpu->registers[CPSR] & Z_MASK);
        break;
    case 0xa: // BGE
        branch = ((cpu->registers[CPSR] & N_MASK) == 0) == ((cpu->registers[CPSR] & V_MASK) == 0);
        break;
    case 0xb: // BLT
        branch = ((cpu->registers[CPSR] & N_MASK) == 0) != ((cpu->registers[CPSR] & V_MASK) == 0);
        break;
    case 0xc: // BGT
        branch = ((cpu->registers[CPSR] & Z_MASK) == 0) && ((cpu->registers[CPSR] & N_MASK) == 0) && ((cpu->registers[CPSR] & V_MASK) == 0);
        break;
    case 0xd: // BLE
        branch = ((cpu->registers[CPSR] & Z_MASK) != 0) || ((cpu->registers[CPSR] & N_MASK) != 0) || ((cpu->registers[CPSR] & V_MASK) != 0);
        break;
    // 0xe is undefined and 0xf is reserved
    default:
        break;
    }
    if (branch)
    {
        cpu->registers[PC] += nn << 1;
    }
}

void thumb_multiple_load_store(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b1 << 11,
        RB = 0b111 << 8,
        RLIST = 0b11111111
    };
    int opcode = instruction & OPCODE;
    int rb = instruction & RB;
    rb >>= 8;
    rb++;
    int rlist = instruction & RLIST;
    // undefined behaviour of armv4
    if (rlist == 0)
    {
        if (opcode)
        {
            // LDM
            cpu->registers[PC] = read_word_from_memory(cpu, cpu->registers[rb]);
        }
        else
        {
            // STR
            write_word_to_memory(cpu, cpu->registers[rb], cpu->registers[PC]);
        }
        cpu->registers[rb] += 0x40;
    }
    // normal behaviour
    for (int i = 0; rlist != 0; i++, rlist >>= 1)
    {
        if (rlist & 0b1)
        {
            if (opcode)
            {
                // LDM
                cpu->registers[i] = read_word_from_memory(cpu, cpu->registers[rb]);
            }
            else
            {
                // STM
                write_word_to_memory(cpu, cpu->registers[rb], cpu->registers[i]);
            }
            cpu->registers[rb] += sizeof(WORD);
        }
    }
}

void thumb_long_branch_and_link(struct cpu *cpu, HALF_WORD instruction)
{
}

void thumb_offset_stackpointer(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b1 << 7,
        NN = 0b1111111
    };
    int opcode = instruction & OPCODE;
    int nn = instruction & NN;
    if (opcode)
    {
        // sub
        cpu->registers[SP] -= nn;
    }
    else
    {
        // add
        cpu->registers[SP] += nn;
    }
}

void thumb_push_pop_registers(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b1 << 11,
        PC_LR = 0b1 << 8,
        RLIST = 0b11111111
    };
    int opcode = instruction & OPCODE;
    int pc_lr = instruction & PC_LR;
    int rlist = instruction & RLIST;
    for (int i = R0; rlist != 0; rlist >>= 1, i++)
    {
        if (rlist & 0b1)
        {
            if (opcode)
            {
                // POP
                cpu->registers[i] = read_word_from_memory(cpu, cpu->registers[SP]);
                cpu->registers[SP] += sizeof(WORD);
            }
            else
            {
                // PUSH
                write_word_to_memory(cpu, cpu->registers[SP], cpu->registers[i]);
                cpu->registers[SP] -= sizeof(WORD);
            }
        }
    }
    if (pc_lr)
    {
        if (opcode)
        {
            // POP PC
            cpu->registers[PC] = read_word_from_memory(cpu, cpu->registers[SP]);
            cpu->registers[SP] += sizeof(WORD);
        }
        else
        {
            // PUSH LR
            write_word_to_memory(cpu, cpu->registers[SP], cpu->registers[LR]);
            cpu->registers[SP] -= sizeof(WORD);
        }
    }
}

void thumb_load_store_halfword(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b1 << 11,
        NN = 0b11111 << 6,
        RB = 0b1111 << 3,
        RD = 0b111
    };
    int opcode = instruction & OPCODE;
    int nn = instruction & NN;
    nn >>= 6;
    int rb = instruction & RB;
    rb >>= 3;
    rb++;
    int rd = instruction & RD;
    rd++;
    if (opcode)
    {
        // LDRH
        cpu->registers[rd] = 0 | read_half_word_from_memory(cpu, cpu->registers[rb] + nn);
    }
    else
    {
        // STRH
        write_half_word_to_memory(cpu, cpu->registers[rb] + nn, cpu->registers[rd]);
    }
}

void thumb_sp_relative_load_store(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b1 << 11,
        RD = 0b1111 << 8,
        NN = 0b11111111
    };
    int opcode = instruction & OPCODE;
    int rd = instruction & RD;
    rd >>= 8;
    rd++;
    int nn = instruction & NN;

    if (opcode)
    {
        // LDR
        cpu->registers[rd] = read_word_from_memory(cpu, cpu->registers[SP] + nn);
    }
    else
    {
        // STR
        write_word_to_memory(cpu, cpu->registers[SP] + nn, cpu->registers[rd]);
    }
}

void thumb_load_address(struct cpu *cpu, HALF_WORD instruction)
{
}

void thumb_load_store_with_offset(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b11 << 11,
        NN = 0b11111 << 6,
        RB = 0b111 << 3,
        RD = 0b111
    };
    int opcode = instruction & OPCODE;
    int nn = instruction & NN;
    nn >>= 6;
    int rb = instruction & RB;
    rb >>= 3;
    rb++;
    int rd = instruction & RD;
    rd++;
    switch (opcode >> 11)
    {
    case 0b00: // STR
        write_word_to_memory(cpu, cpu->registers[rb] + nn, cpu->registers[rd]);
        break;
    case 0b01: // LDR
        cpu->registers[rd] = read_word_from_memory(cpu, cpu->registers[rb] + nn);
        break;
    case 0b10: // STRB
        cpu->memory[cpu->registers[rb] + nn] = cpu->registers[rd] & 0xFF;
        break;
    case 0b11: // LDRB
        cpu->registers[rd] = 0 | cpu->memory[cpu->registers[rb] + nn];
        break;
    }
}

void thumb_load_store_with_reg_offset(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b11 << 10,
        RO = 0b111 << 6,
        RB = 0b111 << 3,
        RD = 0b111
    };
    int opcode = instruction & OPCODE;
    opcode >>= 10;
    int ro = instruction & RO;
    ro >>= 6;
    ro++;
    int rb = instruction & RB;
    rb >>= 3;
    rb++;
    int rd = instruction & RD;
    rd++;
    switch (opcode)
    {
    case 0b00: // STR
        write_word_to_memory(cpu, cpu->registers[rb] + cpu->registers[ro], cpu->registers[rd]);
        break;
    case 0b01: // STRB
        cpu->memory[cpu->registers[rb] + cpu->registers[ro]] = cpu->registers[rd] & 0xFF;
        break;
    case 0b10: // LDR
        cpu->registers[rd] = read_word_from_memory(cpu, cpu->registers[rb] + cpu->registers[ro]);
        break;
    case 0b11: // LDRB
        cpu->registers[rd] = 0 | cpu->memory[cpu->registers[rb] + cpu->registers[ro]];
        break;
    }
}

void thumb_load_store_sign_extended_byte_halfword(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b11 << 10,
        RO = 0b111 << 6,
        RB = 0b111 << 3,
        RD = 0b111
    };
    int opcode = instruction & OPCODE;
    int ro = instruction & RO;
    ro >>= 6;
    ro++;
    int rb = instruction & RB;
    rb >>= 3;
    rb++;
    int rd = instruction & RD;
    rd++;
    switch (opcode >> 10)
    {
    case 0b00: // STRH
        write_half_word_to_memory(cpu, cpu->registers[rb] + cpu->registers[ro], cpu->registers[rd]);
        break;
    case 0b01: // LDSB
        cpu->registers[rd] = cpu->memory[cpu->registers[rb] + cpu->registers[ro]] & 0xFF;
        if (cpu->registers[rd] & (0b1 << 7))
        {
            cpu->registers[rd] |= (UINT32_MAX << 8);
        }
        break;
    case 0b10: // LDRH
        cpu->registers[rd] = read_half_word_from_memory(cpu, cpu->registers[rb] + cpu->registers[ro]);
        break;
    case 0b11: // LDSH
        cpu->registers[rd] = read_half_word_from_memory(cpu, cpu->registers[rb] + cpu->registers[ro]);
        if (cpu->registers[rd] & (0b1 << 15))
        {
            cpu->registers[rd] |= (UINT32_MAX << 16);
        }
        break;
    }
}

void thumb_pc_relative_load(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        RD = 0b111 << 8,
        NN = 0b11111111
    };
    int rd = instruction & RD;
    rd >>= 8;
    rd++;
    int nn = instruction & NN;
    cpu->registers[rd] = read_word_from_memory(cpu, cpu->registers[PC] + nn);
}

void thumb_hi_reg_operation_branch_exchange(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b11 << 8,
        MSBD = 0b1 << 7,
        MSBS = 0b1 << 6,
        RS = 0b111 << 3,
        RD = 0b111
    };
    int opcode = instruction & OPCODE;
    int rs = instruction & MSBS;
    rs |= instruction & RS;
    rs >>= 3;
    rs++;
    if (rs > 0xF)
    {
        rs = PC;
    }
    int rd = instruction & MSBS;
    rd >>= 4;
    rd |= instruction & RD;
    rd++;
    if (rd > 0xF)
    {
        rd = PC;
    }
    switch (opcode >> 8)
    {
    case 0b00: // ADD
        cpu->registers[rd] += cpu->registers[rs];
        break;
    case 0b01: // CMP
        int result = cpu->registers[rd] - cpu->registers[rs];
        if (result == 0)
        {
            cpu->registers[CPSR] |= Z_MASK;
            cpu->registers[CPSR] &= ~N_MASK;
        }
        else if (result < 0)
        {
            cpu->registers[CPSR] &= ~Z_MASK;
            cpu->registers[CPSR] |= N_MASK;
        }
        break;
    case 0b10: // MOV
        cpu->registers[rd] = cpu->registers[rs];
        break;
    case 0b11: // BX
        cpu->registers[PC] = cpu->registers[rs];
        cpu->registers[CPSR] &= ~T_MASK;
        break;
    }
}

void thumb_alu_operations(struct cpu *cpu, HALF_WORD instruction)
{
    enum
    {
        OPCODE = 0b1111 << 6,
        RS = 0b111 << 3,
        RD = 0b111
    };
    int opcode = instruction & OPCODE;
    int rs = instruction & RS;
    rs >>= 3;
    rs++;
    int rd = instruction & RD;
    rd++;
    /*     0: AND{S} Rd,Rs     ;AND logical       Rd = Rd AND Rs
           1: EOR{S} Rd,Rs     ;XOR logical       Rd = Rd XOR Rs
           2: LSL{S} Rd,Rs     ;log. shift left   Rd = Rd << (Rs AND 0FFh)
           3: LSR{S} Rd,Rs     ;log. shift right  Rd = Rd >> (Rs AND 0FFh)
           4: ASR{S} Rd,Rs     ;arit shift right  Rd = Rd SAR (Rs AND 0FFh)
           5: ADC{S} Rd,Rs     ;add with carry    Rd = Rd + Rs + Cy
           6: SBC{S} Rd,Rs     ;sub with carry    Rd = Rd - Rs - NOT Cy
           7: ROR{S} Rd,Rs     ;rotate right      Rd = Rd ROR (Rs AND 0FFh)
           8: TST    Rd,Rs     ;test            Void = Rd AND Rs
           9: NEG{S} Rd,Rs     ;negate            Rd = 0 - Rs
           A: CMP    Rd,Rs     ;compare         Void = Rd - Rs
           B: CMN    Rd,Rs     ;neg.compare     Void = Rd + Rs
           C: ORR{S} Rd,Rs     ;OR logical        Rd = Rd OR Rs
           D: MUL{S} Rd,Rs     ;multiply          Rd = Rd * Rs
           E: BIC{S} Rd,Rs     ;bit clear         Rd = Rd AND NOT Rs
           F: MVN{S}*/
    int result = 0;
    switch (opcode >> 6)
    {
    case 0b0000: // AND
        cpu->registers[rd] &= cpu->registers[rs];
        result = cpu->registers[rd];
        break;
    case 0b0001: // EOR
        cpu->registers[rd] ^= cpu->registers[rs];
        result = cpu->registers[rd];
        break;
    case 0b0010: // LSL
        cpu->registers[rd] = shift_immediate(cpu, LSL, cpu->registers[rs], cpu->registers[rd]);
        result = cpu->registers[rd];
        break;
    case 0b0011: // LSR
        cpu->registers[rd] = shift_immediate(cpu, LSR, cpu->registers[rs], cpu->registers[rd]);
        result = cpu->registers[rd];
        break;
    case 0b0100: // ASR
        cpu->registers[rd] = shift_immediate(cpu, ASR, cpu->registers[rs], cpu->registers[rd]);
        result = cpu->registers[rd];
        break;
    case 0b0101: // ADC
        if (cpu->registers[CPSR] & C_MASK)
        {
            cpu->registers[rd]++;
        }
        if (test_overflow(cpu->registers[rd], cpu->registers[rs]))
        {
            cpu->registers[CPSR] |= V_MASK | C_MASK;
        }
        else
        {
            cpu->registers[CPSR] &= ~(V_MASK | C_MASK);
        }
        cpu->registers[rd] += cpu->registers[rs];
        result = cpu->registers[rd];
        break;
    case 0b0110: // SBC
        if (cpu->registers[CPSR] & C_MASK)
        {
            cpu->registers[rd]--;
        }
        else
        {
            cpu->registers[rd]++;
        }
        cpu->registers[CPSR] &= ~C_MASK;
        if (test_overflow(cpu->registers[rd], -cpu->registers[rs]))
        {
            cpu->registers[CPSR] |= V_MASK;
        }
        else
        {
            cpu->registers[CPSR] &= ~V_MASK;
        }
        result = cpu->registers[rd];
        break;
    case 0b0111: // ROR
        cpu->registers[rd] = (cpu->registers[rd] >> cpu->registers[rs]) | (cpu->registers[rd] << (sizeof(WORD) * __CHAR_BIT__ - cpu->registers[rs]));
        result = cpu->registers[rd];
        break;
    case 0b1000: // TST
        cpu->registers[CPSR] &= ~(N_MASK | Z_MASK);
        result = cpu->registers[rd] & cpu->registers[rs];
        break;
    case 0b1001: // NEG
        cpu->registers[rd] = 0 - cpu->registers[rs];
        result = cpu->registers[rd];
        break;
    case 0b1010: // CMP
        result = cpu->registers[rd] - cpu->registers[rs];
        break;
    case 0b1011: // CMN
        result = cpu->registers[rd] + cpu->registers[rs];
        break;
    case 0b1100: // ORR
        cpu->registers[rd] |= cpu->registers[rs];
        result = cpu->registers[rd];
        break;
    case 0b1101: // MUL
        cpu->registers[rd] *= cpu->registers[rs];
        result = cpu->registers[rd];
        cpu->registers[CPSR] &= ~C_MASK;
        break;
    case 0b1110: // BIC
        cpu->registers[rd] &= ~cpu->registers[rs];
        result = cpu->registers[rd];
        break;
    case 0b1111: // MVN
        cpu->registers[rd] = ~cpu->registers[rs];
        result = cpu->registers[rd];
        break;
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
}

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
    switch (opcode)
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
    switch (opcode >> 9)
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

WORD read_word_from_memory(struct cpu *cpu, WORD address)
{
    WORD word = 0;
    for (int i = 0; i < sizeof(word); i++)
    {
        if (cpu->registers[CPSR] & E_MASK)
        {
            word <<= 8;
            WORD ca = address - i + sizeof(word) - 1;
            word |= cpu->memory[ca];
        }
        else
        {
            word <<= 8;
            word |= cpu->memory[address + i];
        }
    }
    return word;
}

HALF_WORD read_half_word_from_memory(struct cpu *cpu, WORD address)
{
    HALF_WORD half_word = 0;
    for (int i = 0; i < sizeof(half_word); i++)
    {
        if (cpu->registers[CPSR] & E_MASK)
        {
            half_word <<= 8;
            half_word |= cpu->memory[address - i + sizeof(half_word) - 1];
        }
        else
        {
            half_word <<= 8;
            half_word |= cpu->memory[address + i];
        }
    }
    return half_word;
}

void write_word_to_memory(struct cpu *cpu, WORD address, WORD value)
{
    for (int i = 0; i < sizeof(value); i++)
    {
        if (cpu->registers[CPSR] & E_MASK)
        {
            cpu->memory[address + i] = (value >> (i * 8)) & 0xFF;
        }
        else
        {
            cpu->memory[address + i] = (value >> ((sizeof(value) - 1 - i) * 8)) & 0xFF;
        }
    }
}

void write_half_word_to_memory(struct cpu *cpu, WORD address, HALF_WORD value)
{
    for (int i = 0; i < sizeof(value); i++)
    {
        if (cpu->registers[CPSR] & E_MASK)
        {
            cpu->memory[address + i] = (value >> (i * 8)) & 0xFF;
        }
        else
        {
            cpu->memory[address + i] = (value >> ((sizeof(value) - 1 - i) * 8)) & 0xFF;
        }
    }
}