#include "include/testing.h"
#include "testing.h"

void test_cpu()
{
    struct cpu cpu;
    cpu_init(&cpu);
    test_arm_branch(&cpu);
    test_arm_branch_and_exchange(&cpu);
    test_arm_block_data_transfer(&cpu);
}

void test_arm_branch(struct cpu *cpu)
{
    printf("Testing ARM branch instructions:\n");
    // branch 16 steps forward
    BYTE cond = AL;
    BYTE opcode = 0b1010;
    uint32_t offset = 0x10;
    int old_pc = cpu->registers[pc];
    WORD branch_instruction = cond << 28 | opcode << 24 | offset;
    arm_branch(cpu, branch_instruction);
    printf("\tTesting ARM branch...\t\t\t\t");
    if (cpu->registers[pc] != old_pc + offset)
    {
        printf("Failed\n");
    }
    else
    {
        printf("Passed\n");
    }
    printf("\tTesting ARM branch link...\t\t\t");
    // branch link 16 steps forward
    opcode = 0b1011;
    branch_instruction = cond << 28 | opcode << 24 | offset;
    old_pc = cpu->registers[pc];
    arm_branch(cpu, branch_instruction);
    if (cpu->registers[pc] != old_pc + offset || cpu->registers[lr] != old_pc)
    {
        printf("Failed\n");
    }
    else
    {
        printf("Passed\n");
    }
    printf("\tTesting ARM branch link and exchange...\t\tNOT IMPLEMENTED\n");
}

void test_arm_branch_and_exchange(struct cpu *cpu)
{
    printf("Testing ARM branch and exchange instructions:\n");
    // branch and exchange
    BYTE cond = AL;
    BYTE opcode = 0b0001;
    BYTE reg = 0b1001;
    WORD branch_instruction = cond << 28 | opcode << 4 | reg;
    arm_branch_and_exchange(cpu, branch_instruction);
    printf("\tTesting ARM branch and exchange...\t\t");
    if (cpu->registers[pc] != cpu->registers[reg] || (cpu->registers[cpsr] & THUMB_MASK) != THUMB_MASK)
    {
        printf("Failed\n");
    }
    else
    {
        printf("Passed\n");
    }
    printf("\tTesting ARM branch and exchange link...\t\t");
    // branch and exchange link
    opcode = 0b0011;
    branch_instruction = cond << 28 | opcode << 4 | reg;
    int old_pc = cpu->registers[pc];
    arm_branch_and_exchange(cpu, branch_instruction);
    if (cpu->registers[pc] != cpu->registers[reg] || cpu->registers[lr] != old_pc || (cpu->registers[cpsr] & THUMB_MASK) != THUMB_MASK)
    {
        printf("Failed\n");
    }
    else
    {
        printf("Passed\n");
    }
    printf("\tTesting ARM branch and exchange jazzele...\t");
    opcode = 0b0010;
    branch_instruction = cond << 28 | opcode << 4 | reg;
    arm_branch_and_exchange(cpu, branch_instruction);
    if (cpu->registers[pc] != cpu->registers[reg] || (cpu->registers[cpsr] & JAZZELE_MASK) != JAZZELE_MASK)
    {
        printf("Failed\n");
    }
    else
    {
        printf("Passed\n");
    }
}

void test_arm_block_data_transfer(struct cpu *cpu)
{
    for (int i = r0; i <= r5; i++)
    {
        cpu->registers[i] = i;
    }
    cpu->registers[r0] = STACK_START;
    printf("Testing ARM block data transfer instructions:\n");
    printf("\tTesting ARM block data storing...\t\t");
    BYTE cond = AL;
    BYTE opt = 0b11010;
    BYTE reg = r0;
    uint16_t reg_list = 0b111111;
    WORD instruction = cond << 28 | opt << 20 | reg << 16 | reg_list;
    arm_block_data_transfer(cpu, instruction);
    bool store_flag = true;
    for (int i = r0; i <= r5; i++)
    {
        if (cpu->registers[i] != cpu_load_word(cpu, cpu->registers[r0] - i * 4) && cpu->registers[i] != cpu_load_word(cpu, cpu->registers[r0] - i * 4) + sizeof(WORD) * 6)
        {
            store_flag = false;
            break;
        }
    }
    if (!store_flag)
    {
        printf("Failed\n");
    }
    else
    {
        printf("Passed\n");
    }
    printf("\tTesting ARM block data loading...\t\t");
    opt = 0b11001;
    cpu->registers[reg] = STACK_START;
    instruction = cond << 28 | opt << 20 | reg << 16 | reg_list << 6;
    arm_block_data_transfer(cpu, instruction);
    bool load_flag = true;
    for (int i = r6; i <= r11; i++)
    {
        if (cpu->registers[i] != cpu->registers[i - r6])
        {
            load_flag = false;
            break;
        }
    }
    if (!load_flag)
    {
        printf("Failed\n");
    }
    else
    {
        printf("Passed\n");
    }
}

/*
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
    */
void test_arm_data_processing(struct cpu *cpu)
{
    printf("Testing ARM data processing instructions:\n");
    printf("\t Testing AND...\t\t\t\t");
    BYTE cond = AL;
    BYTE opcode = 0x0;
    BYTE rd = r0;
    BYTE rn = r1;
}