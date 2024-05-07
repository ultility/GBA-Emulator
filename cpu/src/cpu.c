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

void cpu_loop(struct cpu *cpu)
{
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
        case BREAKPOINT:
            printf("\tbreakpoint");
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
    if ((instruction & BRANCH_OPCODE_MASK) == BRANCH_OPCODE)
    {
        return BRANCH;
    }
    if ((instruction & BRANCH_AND_EXCHANGE_OPCODE_MASK) == BRANCH_AND_EXCHANGE_OPCODE)
    {
        return BRANCH_AND_EXCHANGE;
    }
    if ((instruction & SOFTWARE_INTERRUPT_OPCODE_MASK) == SOFTWARE_INTERRUPT_OPCODE)
    {
        return SOFTWARE_INTERRUPT;
    }
    if ((instruction & BREAKPOINT_OPCODE_MASK) == BREAKPOINT_OPCODE)
    {
        return BREAKPOINT;
    }
    if ((instruction & UNDEFINED_OPCODE_MASK) == UNDEFINED_OPCODE)
    {
        return UNDEFINED;
    }
    if ((instruction & DATA_PROCESSING_OPCODE_MASK) == DATA_PROCESSING_OPCODE)
    {
        return DATA_PROCESSING;
    }
    if ((instruction & MULTIPLY_OPCODE_MASK) == MULTIPLY_OPCODE)
    {
        return MULTIPLY;
    }
    if ((instruction & PSR_TRANSFER_OPCODE_MASK) == PSR_TRANSFER_OPCODE)
    {
        return PSR_TRANSFER;
    }
    if ((instruction & SINGLE_DATA_TRANSFER_OPCODE_MASK) == SINGLE_DATA_TRANSFER_OPCODE)
    {
        return SINGLE_DATA_TRANSFER;
    }
    if ((instruction & HDS_DATA_TRANSFER_OPCODE_MASK) == HDS_DATA_TRANSFER_OPCODE)
    {
        return HDS_DATA_TRANSFER;
    }
    if ((instruction & BLOCK_DATA_TRANSFER_OPCODE_MASK) == BLOCK_DATA_TRANSFER_OPCODE)
    {
        return BLOCK_DATA_TRANSFER;
    }
    if ((instruction & SINGLE_DATA_SWAP_OPCODE_MASK) == SINGLE_DATA_SWAP_OPCODE)
    {
        return SINGLE_DATA_SWAP;
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
        for (int i = 3; i > 0; i--)
        {
            BYTE byte = cpu->memory[cpu->registers[PC] + i];
            instruction |= byte << (i * 8);
        }
    }
    return instruction;
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
    uint8_t address_reg = (instruction & Rn) >> 16;
    uint32_t address_offset = 0;
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