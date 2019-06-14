#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#include "macros.h"
#include "output.h"
#include "chip8.h"

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <ROM file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	// Copy the ROM filepath into a string.
	char *filepath = argv[1];

	// Alocate a chip8 state in heap.
	struct chip8 *ch8 = create_chip8(filepath);

	struct screen screen;
	screen_init(&screen, argv[1]);

	uint32_t initial_tick;
	uint32_t frame_speed;

    SDL_Event e;
	bool quit = false;
	while (!quit) {
	    initial_tick = SDL_GetTicks();

	    // Performs an emulator cycle: fetch, execute, update flags.
        step_emulate(ch8);

	    if(ch8->draw) {
	        screen_draw(&screen, ch8->vbuffer);
            ch8->draw = false;
	    }

	    frame_speed = SDL_GetTicks() - initial_tick;
	    if(frame_speed < MILLIS_PER_CYCLE)
	        SDL_Delay(MILLIS_PER_CYCLE - frame_speed);

	    // Analyze input.
	    while(SDL_PollEvent(&e))
	        if(e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE)
	            quit = true;
	}

	screen_clear(&screen);
	destroy_chip8(ch8);

	return 0;
}