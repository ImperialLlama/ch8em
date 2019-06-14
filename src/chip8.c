#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "macros.h"

static void chip8_err(struct chip8 *ch8)
{
	fprintf(stderr, "ERROR: %x at pc %u could not be executed.\n",
	        ch8->opcode.instruction, ch8->pc);
}

static void exec_clr_instr(struct chip8 *cpu)
{
	memset(cpu->vbuffer, BLACK_COLOR, sizeof(cpu->vbuffer));
}

static void exec_jmp_instr(struct chip8 *ch8, uint16_t address)
{
	ch8->pc = address;
}

static void exec_skip_instr(struct chip8 *ch8, bool expression)
{
	if (expression) {
		ch8->pc += 2;
	}
}

static void exec_call_instr(struct chip8 *ch8, uint16_t address)
{
	ch8->stack[ch8->sp++] = ch8->pc;
	exec_jmp_instr(ch8, address);
}

static void ass_reg_instr(struct chip8 *ch8, uint8_t register_id, uint8_t value)
{
	ch8->v[register_id] = value;
}

static void add_carry_instr(struct chip8 *ch8, uint8_t left, uint8_t right)
{
	ch8->v[VF] = left + right > 255;
	ch8->v[ch8->opcode.x] = left + right;
}

static void sub_borrow_instr(struct chip8 *ch8, uint8_t left, uint8_t right)
{
	ch8->v[VF] = left > right;
	ch8->v[ch8->opcode.x] = left - right;
}

static void lshift_instr(struct chip8 *ch8)
{
	ch8->v[VF] = (bool) (ch8->v[ch8->opcode.x] & 0x80);
	ch8->v[ch8->opcode.x] <<= 2;
}

static void rshift_instr(struct chip8 *ch8)
{
	ch8->v[VF] = ch8->v[ch8->opcode.x] & 0b1;
	ch8->v[ch8->opcode.x] >>= 2;
}

static void exec_rand_instr(struct chip8 *ch8)
{
	ch8->v[ch8->opcode.x] = (rand() % 0xff) & ch8->opcode.kk;
}

static void draw_screen_instr(struct chip8 *ch8)
{
	ch8->v[VF] = 0;
	for (int y = 0; y < ch8->opcode.n; y++) {
		for (int x = 0; x < 8; x++) {
			uint8_t pixel = ch8->mem[ch8->i + y];
			if (pixel & (0x80 >> x)) {
				int index =
						(ch8->v[ch8->opcode.x] + x) % WINDOW_WIDTH +
						((ch8->v[ch8->opcode.y] + y) % WINDOW_HEIGHT) * WINDOW_WIDTH;
				if (ch8->vbuffer[index] == WHITE_COLOR) {
					ch8->v[VF] = 1;
					ch8->vbuffer[index] = BLACK_COLOR;
				} else {
					ch8->vbuffer[index] = WHITE_COLOR;
				}
				ch8->draw = true;
			}
		}
	}
}

static void wait_key_instr(struct chip8 *ch8)
{
	// Sub PC in order to keep PC repeating this instruction until input is received.
	ch8->pc -= 2;
	// We check if any button is held down using GetKeyboardState function.
	for (int i = 0; i < 16; i++) {
		if (SDL_GetKeyboardState(NULL)[keymap[i]]) {
			ch8->v[ch8->opcode.x] = i;
			ch8->pc += 2;
			break;
		}
	}
}

static void ass_i_instr(struct chip8 *ch8, uint16_t value)
{
	ch8->i = value;
}

static void bcd_str_instr(struct chip8 *ch8)
{
	ch8->mem[ch8->i] = ch8->v[ch8->opcode.x] / 100;
	ch8->mem[ch8->i + 1] = (ch8->v[ch8->opcode.x] / 10) % 10;
	ch8->mem[ch8->i + 2] = ch8->v[ch8->opcode.x] % 10;
}

static void mem_str_instr(struct chip8 *ch8)
{
	memcpy(ch8->mem + ch8->i, ch8->v, ch8->opcode.x + 1);
}

static void mem_load_instr(struct chip8 *ch8)
{
	memcpy(ch8->v, ch8->mem + ch8->i, ch8->opcode.x + 1);
}

static void ass_time_instr(struct chip8 *ch8, uint8_t value)
{
	ch8->delay_timer = value;
}

static void ass_stime_instr(struct chip8 *ch8, uint8_t value)
{
	ch8->sound_timer = value;
}

static long get_file_size(FILE *fp)
{
	fseek(fp, 0L, SEEK_END);
	long res = ftell(fp);
	rewind(fp);
	return res;
}

struct chip8 *create_chip8(char *filename)
{
	struct chip8 *result = calloc(1, sizeof(struct chip8));

	printf("Loading ROM file \"%s\"\n", filename);

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Unable to open file '%s'!\n", filename);
		exit(EXIT_FAILURE);
	}

	long size = get_file_size(fp);
	if (size > MAX_ROM_SIZE) {
		fprintf(stderr, "ROM too large to fit in CHIP-8 memory\n");
		exit(EXIT_FAILURE);
	}
	printf("ROM size: %ld bytes\n", size);

	size_t read = fread(&result->mem[PROG_START], sizeof(uint8_t), MEMORY_SIZE - PROG_START, fp);
	if (read != size) {
		fprintf(stderr, "Failed to read ROM\n");
		exit(EXIT_FAILURE);
	}

	memcpy(result->mem, chip8_font, sizeof(chip8_font));
	memset(result->vbuffer, BLACK_COLOR, sizeof(result->vbuffer));

	result->pc = PROG_START;

	srand(time(NULL));

	fclose(fp);
	return result;
}

void destroy_chip8(struct chip8 *ch8)
{
	free(ch8);
}

void chip8_update_time(struct chip8 *ch8)
{
	if (ch8->delay_timer > 0)
		ch8->delay_timer--;
	if (ch8->sound_timer > 0) {
		// TODO implement sound
		ch8->sound_timer--;
	}
}

void chip8_exe_instr(struct chip8 *ch8)
{
	// Fetch instruction in opcode field.
	ch8->opcode.instruction = ch8->mem[ch8->pc] << 8 | ch8->mem[ch8->pc + 1];
	ch8->pc += 2;
	uint8_t vx = ch8->v[ch8->opcode.x];
	uint8_t vy = ch8->v[ch8->opcode.y];

	switch (ch8->opcode.op_id) {
	case 0x0:
		switch (ch8->opcode.n) {
		case 0x0:
			return exec_clr_instr(ch8);
		case 0xE:
			return exec_jmp_instr(ch8, ch8->stack[--ch8->sp]);
		default:
			return chip8_err(ch8);
		}
	case 0x1:
		return exec_jmp_instr(ch8, ch8->opcode.address);
	case 0x2:
		return exec_call_instr(ch8, ch8->opcode.address);
	case 0x3:
		return exec_skip_instr(ch8, vx == ch8->opcode.kk);
	case 0x4:
		return exec_skip_instr(ch8, vx != ch8->opcode.kk);
	case 0x5:
		return exec_skip_instr(ch8, vx == vy);
	case 0x6:
		return ass_reg_instr(ch8, ch8->opcode.x, ch8->opcode.kk);
	case 0x7:
		return ass_reg_instr(ch8, ch8->opcode.x, vx + ch8->opcode.kk);
	case 0x8:
		switch (ch8->opcode.n) {
		case 0x0:
			return ass_reg_instr(ch8, ch8->opcode.x, vy);
		case 0x1:
			return ass_reg_instr(ch8, ch8->opcode.x, vx | vy);
		case 0x2:
			return ass_reg_instr(ch8, ch8->opcode.x, vx & vy);
		case 0x3:
			return ass_reg_instr(ch8, ch8->opcode.x, vx ^ vy);
		case 0x4:
			return add_carry_instr(ch8, vx, vy);
		case 0x5:
			return sub_borrow_instr(ch8, vx, vy);
		case 0x6:
			return rshift_instr(ch8);
		case 0x7:
			return sub_borrow_instr(ch8, vy, vx);
		case 0xE:
			return lshift_instr(ch8);
		default:
			return chip8_err(ch8);
		}
	case 0x9:
		return exec_skip_instr(ch8, vx != vy);
	case 0xA:
		return ass_i_instr(ch8, ch8->opcode.address);
	case 0xB:
		return exec_jmp_instr(ch8, ch8->opcode.address + ch8->v[0]);
	case 0xC:
		return exec_rand_instr(ch8);
	case 0xD:
		return draw_screen_instr(ch8);
	case 0xE:
		switch (ch8->opcode.kk) {
		case 0x9E:
			return exec_skip_instr(ch8, SDL_GetKeyboardState(NULL)[keymap[vx]]);
		case 0xA1:
			return exec_skip_instr(ch8, !SDL_GetKeyboardState(NULL)[keymap[vx]]);
		default:
			return chip8_err(ch8);
		}
	case 0xF:
		switch (ch8->opcode.kk) {
		case 0x07:
			return ass_reg_instr(ch8, ch8->opcode.x, ch8->delay_timer);
		case 0x0A:
			return wait_key_instr(ch8);
		case 0x15:
			return ass_time_instr(ch8, vx);
		case 0x18:
			return ass_stime_instr(ch8, vx);
		case 0x1E:
			return ass_i_instr(ch8, ch8->i + vx);
		case 0x29:
			return ass_i_instr(ch8, vx * 0x5);
		case 0x33:
			return bcd_str_instr(ch8);
		case 0x55:
			return mem_str_instr(ch8);
		case 0x65:
			return mem_load_instr(ch8);
		default:
			return chip8_err(ch8);
		}
	}
	return chip8_err(ch8);
}

void step_emulate(struct chip8 *ch8)
{
	chip8_exe_instr(ch8);
	chip8_update_time(ch8);
}
