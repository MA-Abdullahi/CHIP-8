#include "chip8.h"
#include <assert.h>
#define BUTTONS 16
void setMap(struct Keyboard* keyboard,const char* map){
    keyboard->map=map;

}
int buttonmapping(struct Keyboard* keyboard,char button){
    for(int i =0; i<BUTTONS;i++){
        if (keyboard->map[i]==button){
            return i;
        }

    }

}
void buttonDown(struct Keyboard* keyboard, int button) {
    if (button >= 0 && button < BUTTONS) { 
        keyboard->keyboard[button] = true;       
    }
}
void buttonUp(struct Keyboard* keyboard, int button) {
    if (button >= 0 && button < BUTTONS) { 
        keyboard->keyboard[button] = false;     
    }
}
bool isButtonDown(struct Keyboard* keyboard, int button) {
    if (button >= 0 && button < BUTTONS) { 
        return keyboard->keyboard[button];        
    }
    return false; 
}
