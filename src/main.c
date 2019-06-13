#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "chip8.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 512

uint8_t keymap[16] = {
		SDLK_x,
		SDLK_1,
		SDLK_2,
		SDLK_3,
		SDLK_q,
		SDLK_w,
		SDLK_e,
		SDLK_a,
		SDLK_s,
		SDLK_d,
		SDLK_z,
		SDLK_c,
		SDLK_4,
		SDLK_r,
		SDLK_f,
		SDLK_v,
};

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <ROM file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	struct chip8 *ch8 = create_chip8();


	SDL_Window *window = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Could not init SDL: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
	uint32_t pixels[2048];

	load_program(argv[1], ch8);

	while (true) {
		step_emulate(ch8);

		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				exit(EXIT_SUCCESS);

			if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_ESCAPE)
					exit(EXIT_SUCCESS);

				for (int i = 0; i < 16; ++i)
					if (e.key.keysym.sym == keymap[i])
						ch8->keypad[i] = 1;
			}

			if (e.type == SDL_KEYUP)
				for (int i = 0; i < 16; ++i)
					if (e.key.keysym.sym == keymap[i])
						ch8->keypad[i] = 0;
		}
		if (ch8->draw) {
			for (int i = 0; i < 2048; ++i) {
				uint8_t pixel = ch8->vbuffer[i];
				pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
			}
			SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t));
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	destroy_chip8(ch8);
	return 0;
}