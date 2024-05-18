#pragma once

enum arm_instruction_set
{
    BRANCH,
    BRANCH_AND_EXCHANGE,
    SOFTWARE_INTERRUPT,
    UNDEFINED,
    DATA_PROCESSING,
    MULTIPLY,
    PSR_TRANSFER,
    SINGLE_DATA_TRANSFER,
    HDS_DATA_TRANSFER, // halfword, doubleword and signed data transfer
    BLOCK_DATA_TRANSFER,
    SINGLE_DATA_SWAP,
    COPROCESSOR,
};

enum arm_instruction_set_opcodes
{
    BRANCH_OPCODE = 0b101 << 25,
    BRANCH_AND_EXCHANGE_OPCODE = 0b00010010111111111111 << 8,
    SOFTWARE_INTERRUPT_OPCODE = 0b1111 << 24,
    UNDEFINED_OPCODE = 0b011 << 25 | 0b1 << 4,
    DATA_PROCESSING_OPCODE = 0b00 << 26,
    MULTIPLY_OPCODE = 0b000 << 25,
    PSR_TRANSFER_OPCODE = 0b00 << 26 | 0b10 << 23 || 0b0 << 20,
    SINGLE_DATA_TRANSFER_OPCODE = 0b01 << 26,
    HDS_DATA_TRANSFER_OPCODE = 0b000 << 25,
    BLOCK_DATA_TRANSFER_OPCODE = 0b100 << 25,
    SINGLE_DATA_SWAP_OPCODE = 0b00010 << 23,
    COPROCESSOR_OPCODE = 0b1110 << 24,
};

enum arm_instruction_set_opcode_masks
{
    BRANCH_OPCODE_MASK = 0b111 << 25,
    BRANCH_AND_EXCHANGE_OPCODE_MASK = 0b11111111111111111111 << 8,
    SOFTWARE_INTERRUPT_OPCODE_MASK = 0b1111 << 24,
    UNDEFINED_OPCODE_MASK = 0b111 << 25 | 0b1 << 4,
    DATA_PROCESSING_OPCODE_MASK = 0b11 << 26,
    MULTIPLY_OPCODE_MASK = 0b111 << 25,
    PSR_TRANSFER_OPCODE_MASK = 0b11 << 26 | 0b11 << 23 | 0b1 << 20,
    SINGLE_DATA_TRANSFER_OPCODE_MASK = 0b11 << 26,
    HDS_DATA_TRANSFER_OPCODE_MASK = 0b111 << 25,
    BLOCK_DATA_TRANSFER_OPCODE_MASK = 0b111 << 25,
    SINGLE_DATA_SWAP_OPCODE_MASK = 0b11111 << 23,
    COPROCESSOR_OPCODE_MASK = 0b1111 << 24,
};