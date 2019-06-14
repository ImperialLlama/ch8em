#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "SDL.h"
#include "macros.h"
#include "chip8_instruction.h"
#include "chip8.h"

int create_chip8(struct chip8 *ch8, char *filename) {
  FILE* game = fopen(filename, "rb");
  if (!game) {
    fprintf(stderr, "Unable to open file '%s'!\n", filename);
    return -1;
  }
  fread(&ch8->mem[PROG_START], 1, MEMORY_SIZE - PROG_START, game);
  fclose(game);

  memcpy(ch8->mem, chip8_font, sizeof(chip8_font));
  memset(ch8->vbuffer, BLACK_COLOR, sizeof(ch8->vbuffer));
  memset(ch8->stack, 0, sizeof(ch8->stack));
  memset(ch8->v, 0, sizeof(ch8->v));
  memset(ch8->keypad, 0, sizeof(ch8->keypad));

  ch8->pc = PROG_START;
  ch8->opcode.instruction = 0;
  ch8->i = 0;
  ch8->sp = 0;
  ch8->draw = 0;
  ch8->sound_timer = 0;
  ch8->delay_timer = 0;

  srand(time(NULL));
  return 0;
}

void chip8_get_instr(struct chip8 *ch8) {
  ch8->opcode.instruction =
      ch8->mem[ch8->pc] << 8 | ch8->mem[ch8->pc + 1];
}

void chip8_update_time(struct chip8 *ch8) {
  if (ch8->delay_timer > 0) ch8->delay_timer--;
  if (ch8->sound_timer > 0) ch8->sound_timer--;
}


void chip8_exe_instr(struct chip8 *ch8) {
  ch8->pc += 2;
  uint8_t vx = ch8->v[ch8->opcode.x];
  uint8_t vy = ch8->v[ch8->opcode.y];
  switch (ch8->opcode.op) {
    case 0x0:
      switch (ch8->opcode.n) {
        case 0x0: return exec_clr_instr(ch8);
        case 0xE: return exec_jmp_instr(ch8, ch8->stack[--ch8->sp]);
        default:  return chip8_err(ch8);
      }
    case 0x1: return exec_jmp_instr(ch8, ch8->opcode.addr);
    case 0x2: return exec_call_instr(ch8, ch8->opcode.addr);
    case 0x3: return exec_skip_instr(ch8, vx == ch8->opcode.kk);
    case 0x4: return exec_skip_instr(ch8, vx != ch8->opcode.kk);
    case 0x5: return exec_skip_instr(ch8, vx == vy);
    case 0x6: return ass_reg_instr(ch8, ch8->opcode.x, ch8->opcode.kk);
    case 0x7: return ass_reg_instr(ch8, ch8->opcode.x, vx + ch8->opcode.kk);
    case 0x8:
      switch (ch8->opcode.n) {
        case 0x0: return ass_reg_instr(ch8, ch8->opcode.x, vy);
        case 0x1: return ass_reg_instr(ch8, ch8->opcode.x, vx | vy);
        case 0x2: return ass_reg_instr(ch8, ch8->opcode.x, vx & vy);
        case 0x3: return ass_reg_instr(ch8, ch8->opcode.x, vx ^ vy);
        case 0x4: return add_carry_instr(ch8, vx, vy);
        case 0x5: return sub_borrow_instr(ch8, vx, vy);
        case 0x6: return rshift_instr(ch8);
        case 0x7: return sub_borrow_instr(ch8, vy, vx);
        case 0xE: return lshift_instr(ch8);
        default:  return chip8_err(ch8);
      }
    case 0x9: return exec_skip_instr(ch8, vx != vy);
    case 0xA: return ass_i_instr(ch8, ch8->opcode.addr);
    case 0xB: return exec_jmp_instr(ch8, ch8->opcode.addr + ch8->v[0]);
    case 0xC: return exec_rand_instr(ch8);
    case 0xD: return draw_screen_instr(ch8);
    case 0xE:
      switch (ch8->opcode.kk) {
        case 0x9E: return exec_skip_instr(ch8, SDL_GetKeyboardState(NULL)[keymap[vx]]);
        case 0xA1: return exec_skip_instr(ch8, !SDL_GetKeyboardState(NULL)[keymap[vx]]);
        default:   return chip8_err(ch8);
      }
    case 0xF:
      switch (ch8->opcode.kk) {
        case 0x07: return ass_reg_instr(ch8, ch8->opcode.x, ch8->delay_timer);
        case 0x0A: return wait_key_instr(ch8);
        case 0x15: return ass_time_instr(ch8, vx);
        case 0x18: return ass_stime_instr(ch8, vx);
        case 0x1E: return ass_i_instr(ch8, ch8->i + vx);
        case 0x29: return ass_i_instr(ch8, vx * 5);
        case 0x33: return bcd_str_instr(ch8);
        case 0x55: return mem_str_instr(ch8);
        case 0x65: return mem_load_instr(ch8);
        default:   return chip8_err(ch8);
      }
    }
  return chip8_err(ch8);
}

void step_emulate(struct chip8 *ch8) {
    chip8_get_instr(ch8);
    chip8_exe_instr(ch8);
    chip8_update_time(ch8);
}
