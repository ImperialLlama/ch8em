#ifndef CH8EM_OUTPUT_H
#define CH8EM_OUTPUT_H

#include <SDL_system.h>
#include "macros.h"

struct screen {
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Texture* texture;
};

int screen_init(struct screen* screen);
void screen_clear(struct screen* screen);
void screen_draw(struct screen* screen, uint32_t vbuffer[WINDOW_WIDTH * WINDOW_HEIGHT]);

#endif //CH8EM_OUTPUT_H
