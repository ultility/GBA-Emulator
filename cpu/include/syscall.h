#pragma once

enum syscall_number
{
    RESTART_SYSCALL = 0x0,
    EXIT,
    FORK,
    READ,
    WRITE,
    OPEN,
    CLOSE,
};