#ifndef _CH8EM_OUTPUT_H
#define _CH8EM_OUTPUT_H

#include "macros.h"

struct screen {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
};

void screen_init(struct screen *screen, char *filename);

void screen_clear(struct screen *screen);

void screen_draw(struct screen *screen, uint32_t vbuffer[WINDOW_WIDTH * WINDOW_HEIGHT]);

#endif //_CH8EM_OUTPUT_H
