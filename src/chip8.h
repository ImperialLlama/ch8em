#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <stdint.h>
#include <stdbool.h>
#include "macros.h"


struct chip8 {
	uint8_t mem[MEMORY_SIZE];
	uint8_t v[16];

	uint8_t i;

	uint8_t delay_timer;
	uint8_t sound_timer;

	uint16_t pc;
	uint8_t sp;
	uint16_t stack[16];

	uint8_t keypad[16];
	uint32_t vbuffer[WINDOW_WIDTH * WINDOW_HEIGHT];
	bool draw;
};

void create_chip8(struct chip8* ch8, char *filepath);

void destroy_chip8(struct chip8 *chip8);

void load_program(const char *filepath, struct chip8 *ch8);

void step_emulate(struct chip8 *ch8);

#endif //_CHIP8_H_
