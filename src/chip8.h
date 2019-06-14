#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <stdint.h>
#include <stdbool.h>

#define PROG_START 0x200
#define PROG_START_ETI660 0x600

#define MAX_ROM_SIZE 0xe00
#define DISPLAY_SIZE 2048

struct chip8 {
	uint8_t mem[0x1000];
	uint8_t v[16];

	uint8_t i;

	uint8_t delay_timer;
	uint8_t sound_timer;

	uint16_t pc;
	uint8_t sp;
	uint16_t stack[16];

	uint8_t keypad[16];
	uint8_t vbuffer[64 * 32];
	bool draw;
};

struct chip8 *create_chip8(void);

void destroy_chip8(struct chip8 *chip8);

void load_program(const char *filepath, struct chip8 *ch8);

void step_emulate(struct chip8 *ch8);

#endif //_CHIP8_H_
