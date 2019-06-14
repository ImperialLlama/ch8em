#pragma once
#include "chip8.h"

void chip8_err(struct chip8 *ch8);

void exec_clr_instr(struct chip8 *ch8);

void exec_jmp_instr(struct chip8 *ch8, uint16_t address);

void exec_skip_instr(struct chip8 *ch8, bool expression);

void exec_call_instr(struct chip8 *ch8, uint16_t address);

void ass_reg_instr(struct chip8 *ch8, uint8_t register_id, uint8_t value);

void add_carry_instr(struct chip8 *ch8, uint8_t left, uint8_t right);

void sub_borrow_instr(struct chip8 *ch8, uint8_t left, uint8_t right);

void lshift_instr(struct chip8 *ch8);

void rshift_instr(struct chip8 *ch8);

void exec_rand_instr(struct chip8 *ch8);

void draw_screen_instr(struct chip8 *ch8);

void wait_key_instr(struct chip8 *ch8);

void ass_i_instr(struct chip8 *ch8, uint16_t value);

void bcd_str_instr(struct chip8 *ch8);

void mem_str_instr(struct chip8 *ch8);

void mem_load_instr(struct chip8 *ch8);

void ass_time_instr(struct chip8 *ch8, uint8_t value);

void ass_stime_instr(struct chip8 *ch8, uint8_t value);
