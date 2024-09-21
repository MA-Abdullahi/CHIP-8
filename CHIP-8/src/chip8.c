#include "chip8.h"
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include "SDL2/SDL.h"
#include <time.h>
#include <stdlib.h>
const char charSet[] = {0xF0, 0x90, 0x90, 0x90, 0xF0,  
    0x20, 0x60, 0x20, 0x20, 0x70,  
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  
    0x90, 0x90, 0xF0, 0x10, 0x10,  
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  
    0xF0, 0x10, 0x20, 0x40, 0x40,  
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  
    0xF0, 0x90, 0xF0, 0x90, 0x90,  
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  
    0xF0, 0x80, 0x80, 0x80, 0xF0,  
    0xE0, 0x90, 0x90, 0x90, 0xE0,  
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  
    0xF0, 0x80, 0xF0, 0x80, 0x80};
void CPU_init(struct CPU* cpu){
    memset(cpu,0,sizeof(struct CPU));
    memcpy(&cpu->memory.memory,charSet,sizeof(charSet));
}
unsigned char getMemory(int index, struct CPUmem *mem) {
    assert(index >= 0 && index < CHIP8_MEMORY_SIZE);
    unsigned char value = mem->memory[index];
    return value;
}

void setMemory(struct CPUmem *mem, int index, unsigned char value) {
    assert(index >= 0 && index < CHIP8_MEMORY_SIZE);
    mem->memory[index] = value;

}

void stackPush(struct CPU* cpu, unsigned short val) {
    assert(cpu->registers.sp < STACK_LENGTH);  
    cpu->stack.stack[cpu->registers.sp] = val;
    cpu->registers.sp += 1;                    
}

unsigned short stackPop(struct CPU* cpu) {
    assert(cpu->registers.sp > 0);              
    cpu->registers.sp -= 1;                    
    unsigned short val = cpu->stack.stack[cpu->registers.sp]; 
    return val;                                  
}
void setPixel(struct CPUscreen *screen, int x, int y) {
    //printf("%d %d\n",x,y);
    assert(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT);
    screen->pixels[y][x] = true;
}

void screenClear(struct CPUscreen * screen){
    memset(screen->pixels,0,sizeof(screen->pixels));
}
bool isPixelSet(struct CPUscreen *screen, int x, int y) {
    //printf("%d %d\n",x,y);
    assert(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT);
    return screen->pixels[y][x];
}
bool drawSprite(struct CPUscreen *screen,int x,int y, const char* sprite,int num){
    bool pixelChange = false;

    for (int ly = 0; ly<num; ly++){
        char c = sprite[ly];
        for (int lx = 0;lx <8;lx++){
            if ((c &(0b10000000>>lx))==0){
                continue;
            }
            if (screen->pixels[(ly+y)% HEIGHT][(lx+x)%WIDTH]){
                pixelChange = true;
            }
            screen->pixels[(ly+y)% HEIGHT][(lx+x)%WIDTH] ^= true;

        }
    }
    return pixelChange;
}
void execute(struct CPU* cpu,unsigned short opcode)
{
    switch(opcode){
        //clear screen
        case 0x00E0:
            screenClear(&cpu->screen);
            break;
        case 0x00EE:
            cpu->registers.pc = stackPop(cpu);
            break;
        default:
            executeExtended(cpu,opcode);
            break;
    }

}
void execute8000(struct CPU* cpu,unsigned short opcode){
    unsigned char x = (opcode >>8) & 0x000f;
    unsigned char y = (opcode >>4) & 0x000f;
    unsigned char last4bits = opcode & 0x000f;
    unsigned short tmp = 0;
    switch(last4bits){
        case 0x00:
            cpu->registers.V[x] = cpu->registers.V[y];
        break;
        case 0x01:
            cpu->registers.V[x] = cpu->registers.V[x] | cpu->registers.V[y];
        break;
        case 0x02:
            cpu->registers.V[x] = cpu->registers.V[x] & cpu->registers.V[y];
        break;
        case 0x03:
            cpu->registers.V[x] = cpu->registers.V[x] ^ cpu->registers.V[y];
        break;
        case 0x04:
           tmp =cpu->registers.V[x] + cpu->registers.V[y];
           cpu->registers.V[0x0f] = false;
           if (tmp>0xff){
                cpu->registers.V[0x0f] = true;
           }
           cpu->registers.V[x] = tmp;
        break;
        case 0x05:
           cpu->registers.V[0x0f] = false;
           if (cpu->registers.V[x] > cpu->registers.V[y]){
                cpu->registers.V[0x0f] = true;
           }
           cpu->registers.V[x] = cpu->registers.V[x] - cpu->registers.V[y];
        break;
        case 0x06:
           cpu->registers.V[0x0f] = cpu->registers.V[x] & 0x01;
           cpu->registers.V[x]/=2;
        break;
        case 0x07:
            cpu->registers.V[0x0f] = cpu->registers.V[y] >cpu->registers.V[x];
            cpu->registers.V[x] = cpu->registers.V[y]- cpu->registers.V[x];
        break;
        case 0x0E:
            cpu->registers.V[0x0f] = cpu->registers.V[x] & 0x80;
           cpu->registers.V[x]*=2;
        break;

    }

}
char keyWait(struct CPU* cpu){
    SDL_Event event;
    while(SDL_WaitEvent(&event)){
        if(event.type !=SDL_KEYDOWN)
            continue;
        char c = event.key.keysym.sym;
        char key = buttonmapping(&cpu->keyboard,c);
        if (key !=-1){
            return key;
        }
    }

}
void executeF000(struct CPU* cpu, unsigned short opcode) {
    unsigned char x = (opcode >> 8) & 0x0F;
    unsigned char lowOpcode = opcode & 0x00FF;

    switch (lowOpcode) {
        case 0x07:
            cpu->registers.V[x] = cpu->registers.dt;
            break;

        case 0x0A:
            cpu->registers.V[x] = keyWait(cpu);
            break;

        case 0x15:
            cpu->registers.dt = cpu->registers.V[x];
            break;

        case 0x18:
            cpu->registers.st = cpu->registers.V[x];
            break;

        case 0x1E:
            cpu->registers.I += cpu->registers.V[x];
            break;

        case 0x29:
            cpu->registers.I = cpu->registers.V[x] * 5;
            break;

        case 0x33:
            {
                unsigned char value = cpu->registers.V[x];
                setMemory(&cpu->memory, cpu->registers.I, value / 100);
                setMemory(&cpu->memory, cpu->registers.I + 1, (value / 10) % 10);
                setMemory(&cpu->memory, cpu->registers.I + 2, value % 10);
            }
            break;

        case 0x55:
            for (int i = 0; i <= x; i++) {
                setMemory(&cpu->memory, cpu->registers.I + i, cpu->registers.V[i]);
            }
            break;

        case 0x65:
            for (int i = 0; i <= x; i++) {
                cpu->registers.V[i] = getMemory(cpu->registers.I + i, &cpu->memory);
            }
            break;
    }
}


void executeExtended(struct CPU* cpu,unsigned short opcode){
    unsigned short nnn = opcode & 0x0fff;
    unsigned char x = (opcode >>8) & 0x000f;
    unsigned char y = (opcode >>4) & 0x000f;
    unsigned char kk = opcode & 0x00ff;
    unsigned char n = opcode & 0x000f;
    switch(opcode & 0xf000){
        //jp address
        case 0x1000:
            cpu->registers.pc = nnn;
            break;
        //call address
        case 0x2000:
            stackPush(cpu,cpu->registers.pc);
            cpu->registers.pc = nnn;
            break;
        //skip next instruction if Vx = kk
        case 0x3000:
            if (cpu->registers.V[x]==kk){
                cpu->registers.pc +=2;
            }
            break;
        //skip next instruction if Vx != kk
        case 0x4000:
            if (cpu->registers.V[x]!=kk){
                cpu->registers.pc +=2;

            }
            break;
        //check if Vx == Vy
        case 0x5000:
            if (cpu->registers.V[x] == cpu->registers.V[y]){
                cpu->registers.pc +=2;

            }
            break;
        //put byte kk into Vx
        case 0x6000:
            cpu->registers.V[x] =kk;
            break;
        //add kk to register V[x]
        case 0x7000:
            cpu->registers.V[x] +=kk;
            break;
        case 0x8000:
            execute8000(cpu,opcode);
        break;
        case 0x9000:
            if (cpu->registers.V[x] != cpu->registers.V[y]){
                cpu->registers.pc +=2;

            }
        break;
        case 0xA000:
            cpu->registers.I = nnn;
        break;
        case 0xB000:
            cpu->registers.pc = nnn + cpu->registers.V[0x00];
        break;
        case 0xC000:
            srand(clock());
            cpu->registers.V[x]= (rand()% 255) & kk;
        break;
        case 0xD000:{
            const char * sprite = (const char*) &cpu->memory.memory[cpu->registers.I];
            cpu->registers.V[0x0f]=drawSprite(&cpu->screen,cpu->registers.V[x],cpu->registers.V[y],sprite,n);
        }
        break;
        case 0xE000:
            {
                switch(opcode & 0x00ff){
                    case 0x9e:
                        if(isButtonDown(&cpu->keyboard,cpu->registers.V[x])){
                            cpu->registers.pc +=2;

                        }
                        break;
                        case 0xa1:
                            if (!isButtonDown(&cpu->keyboard,cpu->registers.V[x])){
                                cpu->registers.pc +=2;
                            }
                        break;
                }
            }
        case 0xF000:
            executeF000(cpu,opcode);
        break;
    }

}
void load(struct CPU* cpu,const char * buffer, size_t size){
    assert( size + LOAD_ADDRESS <CHIP8_MEMORY_SIZE);
    memcpy(&cpu->memory.memory[LOAD_ADDRESS],buffer,size);
    cpu->registers.pc = LOAD_ADDRESS;
}
unsigned short memoryShort(struct CPUmem* memory,int idx){
    unsigned char byte1 = getMemory(idx,memory);
    unsigned char byte2 = getMemory(idx+1,memory);
    return byte1<<8 | byte2;

}