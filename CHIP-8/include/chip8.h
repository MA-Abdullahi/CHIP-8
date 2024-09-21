#ifndef CHIP8_H
#define CHIP8_H
#include<assert.h>
#include "config.h"
#include "keyboard.h"
#include <stddef.h>
struct CPUmem {
    char memory[4096];
};
struct CPUreg {
    unsigned char V[16];
    unsigned short I;
    unsigned char dt;//delay timer
    unsigned char st;//sound timer
    unsigned short pc;
    unsigned char sp;
};
struct CPUstack{
    unsigned short stack[16];
};
struct CPUscreen{
    bool pixels[32][64];

};
struct CPU {
    struct CPUmem memory;
    struct CPUreg registers;
    struct CPUstack stack;
    struct Keyboard keyboard;
    struct CPUscreen screen;
};

//initialize CPU
void CPU_init(struct CPU* cpu);

//gets and sets the value inside the chip8 memory
unsigned char getMemory(int idx,struct CPUmem * memory);
void setMemory(struct CPUmem * memory,int idx, unsigned char val);
unsigned short memoryShort(struct CPUmem* memory,int idx);
//push or pop into stack
void stackPush(struct CPU* cpu,unsigned short val);
unsigned short stackPop(struct CPU* cpu);

void setPixel(struct CPUscreen *screen,int x,int y);
bool isPixelSet(struct CPUscreen *screen,int x,int y);
bool drawSprite(struct CPUscreen *screen,int x,int y, const char* sprite,int num);
void clearScreen(struct CPUscreen* screen);

void execute(struct CPU* cpu,unsigned short opcode);
void executeExtended(struct CPU* cpu,unsigned short opcode);
void execute8000(struct CPU* cpu,unsigned short opcode);
void executeF000(struct CPU* cpu,unsigned short opcode);
void load(struct CPU* cpu,const char * buffer, size_t size);
#endif
