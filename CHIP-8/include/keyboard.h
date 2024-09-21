#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdbool.h>
#include "config.h"
struct Keyboard{
    bool keyboard[16];
    const char* map;
};
void setMap(struct Keyboard* keyboard,const char* map);
int buttonmapping(struct Keyboard* keyboard,char button);
void buttonDown(struct Keyboard* keyboard,int button);
void buttonUp(struct Keyboard* keyboard,int button);
bool isButtonDown(struct Keyboard* keyboard,int button);
#endif