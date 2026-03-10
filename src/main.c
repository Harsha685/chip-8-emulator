#include <stdio.h>
#include <SDL2/SDL.h>
#include "chip8.h"

#define SCALE 10
#define WINDOW_W 640
#define WINDOW_H 320
#define CYCLE_PER_FRAME 10

int main(int argc, char **argv) {
    if(argc < 2){
        printf("Usage: ./chip8 <rom path>\n");
        return 1;
    }

    // SDL SETUP
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, 0
    );
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    // CHIP8 SETUP
    Chip8 chip8;
    chip8_init(&chip8);
    chip8_load_rom(&chip8, argv[1]);

    // MAIN LOOP
    int running = 1;
    SDL_Event event;
    int paused = 0;

    while(running){

        // HANDLE INPUT
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT)
                running = 0;

            if(event.type == SDL_KEYDOWN){
                switch(event.key.keysym.sym){
                    case SDLK_x: chip8.keypad[0x0] = 1; break;
                    case SDLK_1: chip8.keypad[0x1] = 1; break;
                    case SDLK_2: chip8.keypad[0x2] = 1; break;
                    case SDLK_3: chip8.keypad[0x3] = 1; break;
                    case SDLK_q: chip8.keypad[0x4] = 1; break;
                    case SDLK_w: chip8.keypad[0x5] = 1; break;
                    case SDLK_e: chip8.keypad[0x6] = 1; break;
                    case SDLK_a: chip8.keypad[0x7] = 1; break;
                    case SDLK_s: chip8.keypad[0x8] = 1; break;
                    case SDLK_d: chip8.keypad[0x9] = 1; break;
                    case SDLK_z: chip8.keypad[0xA] = 1; break;
                    case SDLK_c: chip8.keypad[0xB] = 1; break;
                    case SDLK_4: chip8.keypad[0xC] = 1; break;
                    case SDLK_r: chip8.keypad[0xD] = 1; break;
                    case SDLK_f: chip8.keypad[0xE] = 1; break;
                    case SDLK_v: chip8.keypad[0xF] = 1; break;
                    case SDLK_ESCAPE:
                        paused = !paused;
                        break;
                    default: break;
                }
            }

            if(event.type == SDL_KEYUP){
                switch(event.key.keysym.sym){
                    case SDLK_x: chip8.keypad[0x0] = 0; break;
                    case SDLK_1: chip8.keypad[0x1] = 0; break;
                    case SDLK_2: chip8.keypad[0x2] = 0; break;
                    case SDLK_3: chip8.keypad[0x3] = 0; break;
                    case SDLK_q: chip8.keypad[0x4] = 0; break;
                    case SDLK_w: chip8.keypad[0x5] = 0; break;
                    case SDLK_e: chip8.keypad[0x6] = 0; break;
                    case SDLK_a: chip8.keypad[0x7] = 0; break;
                    case SDLK_s: chip8.keypad[0x8] = 0; break;
                    case SDLK_d: chip8.keypad[0x9] = 0; break;
                    case SDLK_z: chip8.keypad[0xA] = 0; break;
                    case SDLK_c: chip8.keypad[0xB] = 0; break;
                    case SDLK_4: chip8.keypad[0xC] = 0; break;
                    case SDLK_r: chip8.keypad[0xD] = 0; break;
                    case SDLK_f: chip8.keypad[0xE] = 0; break;
                    case SDLK_v: chip8.keypad[0xF] = 0; break;
                    default: break;
                }
            }
        }

        // RUN CPU
        for(int i = 0; i < CYCLE_PER_FRAME; i++)
            chip8_cycle(&chip8);

        // UPDATE TIMERS
        if(chip8.dt > 0) chip8.dt--;
        if(chip8.st > 0) chip8.st--;

        if(!paused){
            // RUN CPU
            for(int i = 0; i < CYCLE_PER_FRAME; i++)
                chip8_cycle(&chip8);

            // UPDATE TIMERS
            if(chip8.dt > 0) chip8.dt--;
            if(chip8.st > 0) chip8.st--;
        }

        // RENDER
        if(chip8.draw_flag){
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            for(int row = 0; row < 32; row++){
                for(int col = 0; col < 64; col++){
                    if(chip8.display[row * 64 + col]){
                        SDL_Rect rect = {col * SCALE, row * SCALE, SCALE, SCALE};
                        SDL_RenderFillRect(renderer, &rect);
                    }
                }
            }
            SDL_RenderPresent(renderer);
            chip8.draw_flag = 0;
        }
        SDL_Delay(16); // ~60 FPS
    }

    // CLEANUP
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}