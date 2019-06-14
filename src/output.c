#include "output.h"
#include <stdio.h>

int screen_init(struct screen* screen) {
    // Check if SDL initialized correctly.
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Could not initialize SDL graphics library.\n");
        exit(EXIT_FAILURE);
    }

    screen->window = SDL_CreateWindow("ch8em", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            WINDOW_WIDTH * SCALING, WINDOW_HEIGHT * SCALING, SDL_WINDOW_SHOWN);
    if(!screen->window) {
        fprintf(stderr, "Could not create window.\n");
        exit(EXIT_FAILURE);
    }

    screen->renderer = SDL_CreateRenderer(screen->window, -1, SDL_RENDERER_ACCELERATED);
    if(!screen->renderer) {
        fprintf(stderr, "Could not create renderer.\n");
        exit(EXIT_FAILURE);
    }

    screen->texture = SDL_CreateTexture(screen->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
            WINDOW_WIDTH, WINDOW_HEIGHT);
    if(!screen->texture) {
        fprintf(stderr, "Could not create texture.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void screen_clear(struct screen* screen) {
    // We destroy the structs in an order opposite to that of creation.
    SDL_DestroyTexture(screen->texture);
    SDL_DestroyRenderer(screen->renderer);
    SDL_DestroyWindow(screen->window);
    SDL_Quit();
}

void screen_draw(struct screen* screen, uint32_t vbuffer[WINDOW_WIDTH * WINDOW_HEIGHT]) {
    SDL_UpdateTexture(screen->texture, NULL, vbuffer, WINDOW_WIDTH * sizeof(uint32_t));
    SDL_RenderCopy(screen->renderer, screen->texture, NULL, NULL);
    SDL_RenderPresent(screen->renderer);
}