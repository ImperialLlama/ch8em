#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "macros.h"
#include "opcode.h"

struct chip8 {
  union opcode opcode;
  uint8_t mem[MEMORY_SIZE];
  uint8_t v[16];
  uint16_t i;
  uint16_t pc;

  uint32_t vbuffer[WINDOW_WIDTH * WINDOW_HEIGHT];
  bool keypad[16];

  bool draw;

  uint16_t stack[16];
  uint16_t sp;

  uint8_t sound_timer;
  uint8_t delay_timer;
};

struct chip8* create_chip8(char *filename);
void chip8_destroy(struct chip8* ch8);
void chip8_get_instr(struct chip8 *ch8);
void chip8_exe_instr(struct chip8 *ch8);
void chip8_update_time(struct chip8 *ch8);
void step_emulate(struct chip8 *ch8);
