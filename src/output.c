#include "output.h"
#include <stdio.h>

void screen_init(struct screen *screen, char *filename)
{
	// Check if SDL initialized correctly.
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	screen->window = SDL_CreateWindow("CHIP-8 Emulator",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			WINDOW_WIDTH * SCALING,
			WINDOW_HEIGHT * SCALING,
			SDL_WINDOW_SHOWN);

	if (!screen->window) {
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	screen->renderer = SDL_CreateRenderer(screen->window, -1, SDL_RENDERER_ACCELERATED);
	if (!screen->renderer) {
		fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	screen->texture = SDL_CreateTexture(screen->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
	                                    WINDOW_WIDTH, WINDOW_HEIGHT);
	if (!screen->texture) {
		fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	char *title = calloc(strlen(BASE_WINDOW_TITLE) + strlen(" - ") + strlen(filename) + 1, sizeof(char));
	strcat(title, BASE_WINDOW_TITLE);
	strcat(title, " - ");
	strcat(title, filename);
	SDL_SetWindowTitle(screen->window, title);
	free(title);
}

void screen_clear(struct screen *screen)
{
	// We destroy the structs in an order opposite to that of creation.
	SDL_DestroyTexture(screen->texture);
	SDL_DestroyRenderer(screen->renderer);
	SDL_DestroyWindow(screen->window);
	SDL_Quit();
}

void screen_draw(struct screen *screen, uint32_t vbuffer[WINDOW_WIDTH * WINDOW_HEIGHT])
{
	SDL_UpdateTexture(screen->texture, NULL, vbuffer, WINDOW_WIDTH * sizeof(uint32_t));
	SDL_RenderCopy(screen->renderer, screen->texture, NULL, NULL);
	SDL_RenderPresent(screen->renderer);
}