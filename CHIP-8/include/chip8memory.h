#ifndef CHIP8STACK_H
#define CHIP8STACK_H

#include "config.h"
struct CPU;
struct CPUstack{
    unsigned short stack[16];
};
#endif