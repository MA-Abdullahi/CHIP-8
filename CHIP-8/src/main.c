#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "SDL2/SDL.h"
#include "chip8.h"
#include "config.h"


#define RENDER_COLOR_R 255
#define RENDER_COLOR_G 255
#define RENDER_COLOR_B 255
#define RENDER_COLOR_A 0
#define CLEAR_COLOR_R 0
#define CLEAR_COLOR_G 0
#define CLEAR_COLOR_B 0
#define CLEAR_COLOR_A 0

void handleEvents(struct CPU *chip8);
void renderScreen(SDL_Renderer *renderer, struct CPU *chip8);
void loadFile(struct CPU *chip8, const char *filename);
void initializeSDL(SDL_Window **window, SDL_Renderer **renderer);
void updateTimers(struct CPU *chip8);
void processOpcode(struct CPU *chip8);

// Global keyboard map
const char keyboard_map[16] = {
    SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9,
    SDLK_a, SDLK_b, SDLK_c, SDLK_d, SDLK_e, SDLK_f
};

// Main function
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <CHIP8 ROM>\n", argv[0]);
        return -1;
    }

    
    struct CPU chip8;
    CPU_init(&chip8);
    setMap(&chip8.keyboard, keyboard_map);
  
    loadFile(&chip8, argv[1]);

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    initializeSDL(&window, &renderer);

    while (true) {
        handleEvents(&chip8);         
        renderScreen(renderer, &chip8); 
        updateTimers(&chip8);          
        processOpcode(&chip8);        
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void loadFile(struct CPU *chip8, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Couldn't open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0L, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char buffer[fsize];
    if (fread(buffer, fsize, 1, file) != 1) {
        fprintf(stderr, "Error: Failed to read file %s\n", filename);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    load(chip8, buffer, fsize);
    fclose(file);
}


void initializeSDL(SDL_Window **window, SDL_Renderer **renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    *window = SDL_CreateWindow(TITLE,
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               WIDTH * 10,
                               HEIGHT * 10,
                               SDL_WINDOW_SHOWN);

    if (*window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
}


void handleEvents(struct CPU *chip8) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            SDL_Quit();
            exit(EXIT_SUCCESS);
        }

        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            char key = event.key.keysym.sym;
            int virtual_key = buttonmapping(&chip8->keyboard, key);
            if (virtual_key != 102) {
                if (event.type == SDL_KEYDOWN) {
                    buttonDown(&chip8->keyboard, virtual_key);
                } else {
                    buttonUp(&chip8->keyboard, virtual_key);
                }
            }
        }
    }
}

void renderScreen(SDL_Renderer *renderer, struct CPU *chip8) {
    SDL_SetRenderDrawColor(renderer, CLEAR_COLOR_R, CLEAR_COLOR_G, CLEAR_COLOR_B, CLEAR_COLOR_A);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, RENDER_COLOR_R, RENDER_COLOR_G, RENDER_COLOR_B, RENDER_COLOR_A);
    for (int x = 0; x < 64; ++x) {
        for (int y = 0; y < 32; ++y) {
            if (isPixelSet(&chip8->screen, x, y)) {
                SDL_Rect rect = {
                    .x = x * 10,
                    .y = y * 10,
                    .w = 10,
                    .h = 10
                };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void updateTimers(struct CPU *chip8) {
    if (chip8->registers.dt > 0) {
        usleep(10); 
        chip8->registers.dt--;
    }
    if (chip8->registers.st > 0) {
        chip8->registers.st = 0; 
    }
}

void processOpcode(struct CPU *chip8) {
    unsigned short opcode = memoryShort(&chip8->memory, chip8->registers.pc);
    chip8->registers.pc += 2;
    execute(chip8, opcode);
}
