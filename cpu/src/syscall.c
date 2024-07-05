#include "../include/syscall.h"

void gba_div(struct cpu *cpu)
{
    int num = cpu->registers[R0];
    int denom = cpu->registers[R1];
    cpu->registers[R0] = num / denom;
    cpu->registers[R1] = num % denom;
    cpu->registers[R3] = abs(num / denom);
}