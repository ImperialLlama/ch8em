#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const uint8_t chip8_font[] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
		0x20, 0x60, 0x20, 0x20, 0x70,  // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
		0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
		0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
		0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
		0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
		0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

struct chip8 *create_chip8(void)
{
	struct chip8 *result = (struct chip8*) calloc(1, sizeof(struct chip8));

	result->pc = PROG_START;
	result->draw = true;
	for (int i = 0; i < 80; ++i)
		result->mem[i] = chip8_font[i];

	srand(time(NULL));

	return result;
}

void destroy_chip8(struct chip8 *chip8)
{
	free(chip8);
}

static long get_file_size(FILE *fp)
{
	fseek(fp, 0L, SEEK_END);
	long res = ftell(fp);
	rewind(fp);
	return res;
}

void load_program(const char *filepath, struct chip8 *ch8)
{
	FILE *fp = fopen(filepath, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Failed to open file %s\n", filepath);
		exit(EXIT_FAILURE);
	}

	long size = get_file_size(fp);
	if (size > MAX_ROM_SIZE) {
		fprintf(stderr, "File too large to be read into memory\n");
		exit(EXIT_FAILURE);
	}

	uint8_t *rom = (uint8_t*) malloc(size * sizeof(uint8_t));
	if (rom == NULL) {
		fprintf(stderr, "malloc failed\n");
		exit(EXIT_FAILURE);
	}

	size_t read = fread(rom, sizeof(uint8_t), size, fp);
	if (read != size) {
		fprintf(stderr, "Failed to read ROM\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < size; ++i)
		ch8->mem[i + PROG_START] = rom[i];

	free(rom);
	fclose(fp);
}

static void exec_0_instr(uint16_t opcode, struct chip8 *ch8)
{
	switch (opcode) {
	// CLS - clear screen
	case 0x00e0:
		for (int i = 0; i < DISPLAY_SIZE; ++i)
			ch8->vbuffer[i] = 0;
		ch8->draw = true;
		break;
	// RET - return from subroutine
	case 0x00ee:
		ch8->sp--;
		ch8->pc = ch8->stack[ch8->sp];
		break;
	default:
		fprintf(stderr, "SYS instruction unsupported: %04x\n", opcode);
		break;
	}
	ch8->pc += 2;
}

static void exec_jp_instr(uint16_t opcode, struct chip8 *ch8)
{
	uint16_t dest = opcode & 0x0fff;
	ch8->pc = dest;
}

static void exec_call_instr(uint16_t opcode, struct chip8 *ch8)
{
	uint16_t dest = opcode & 0x0fff;
	ch8->stack[ch8->sp] = ch8->pc;
	ch8->sp++;
	ch8->pc = dest;
}

static void exec_skip_byte_instr(uint16_t opcode, struct chip8 *ch8, bool eq)
{
	uint16_t x = (opcode & 0x0f00) >> 8;
	uint16_t kk = opcode & 0x00ff;
	if (eq)
		ch8->pc += ch8->v[x] == kk ? 4 : 2;
	else
		ch8->pc += ch8->v[x] != kk ? 4 : 2;
}

static void exec_skip_reg_instr(uint16_t opcode, struct chip8 *ch8, bool eq)
{
	uint16_t x = (opcode & 0x0f00) >> 8;
	uint16_t y = (opcode & 0x00f0) >> 4;
	if (eq)
		ch8->pc += ch8->v[x] == ch8->v[y] ? 4 : 2;
	else
		ch8->pc += ch8->v[x] != ch8->v[y] ? 4 : 2;
}

static void exec_ld_byte_instr(uint16_t opcode, struct chip8 *ch8)
{
	uint16_t x = (opcode & 0x0f00) >> 8;
	uint16_t kk = opcode & 0x00ff;
	ch8->v[x] = kk;
	ch8->pc += 2;
}

static void exec_add_byte_instr(uint16_t opcode, struct chip8 *ch8)
{
	uint16_t x = (opcode & 0x0f00) >> 8;
	uint16_t kk = opcode & 0x00ff;
	ch8->v[x] += kk;
	ch8->pc += 2;
}

static void exec_8_instr(uint16_t opcode, struct chip8 *ch8)
{
	uint16_t x = (opcode & 0x0f00) >> 8;
	uint16_t y = (opcode & 0x00f0) >> 4;
	switch (opcode & 0x000f) {
	case 0x0:
		ch8->v[x] = ch8->v[y];
		break;
	case 0x1:
		ch8->v[x] = ch8->v[x] | ch8->v[y];
		break;
	case 0x2:
		ch8->v[x] = ch8->v[x] & ch8->v[y];
		break;
	case 0x3:
		ch8->v[x] = ch8->v[x] ^ ch8->v[y];
		break;
	case 0x4:
		ch8->v[0xf] = ch8->v[y] > 0xff - ch8->v[x] ? 1 : 0;
		ch8->v[x] = ch8->v[x] + ch8->v[y];
		break;
	case 0x5:
		ch8->v[0xf] = ch8->v[x] > ch8->v[y] ? 1 : 0;
		ch8->v[x] = ch8->v[x] - ch8->v[y];
		break;
	case 0x6:
		ch8->v[0xf] = ch8->v[x] & 0b1;
		ch8->v[x] = ch8->v[x] >> 1;
		break;
	case 0x7:
		ch8->v[0xf] = ch8->v[y] > ch8->v[x] ? 1 : 0;
		ch8->v[x] = ch8->v[y] - ch8->v[x];
		break;
	case 0xe:
		ch8->v[0xf] = ch8->v[x] >> 7;
		ch8->v[x] = ch8->v[x] << 1;
		break;
	default:
		fprintf(stderr, "Unknown instruction: %04x\n", opcode);
		exit(EXIT_FAILURE);
	}
	ch8->pc += 2;
}

static void exec_ld_i_instr(uint16_t opcode, struct chip8 *ch8)
{
	uint16_t dest = opcode & 0x0fff;
	ch8->i = dest;
	ch8->pc += 2;
}

static void exec_jp_v0_instr(uint16_t opcode, struct chip8 *ch8)
{
	uint16_t dest = opcode & 0x0fff;
	ch8->pc = ch8->v[0] + dest;
}

static void exec_rnd_instr(uint16_t opcode, struct chip8 *ch8)
{
	uint16_t x = (opcode & 0x0f00) >> 8;
	uint16_t kk = opcode & 0x00ff;
	ch8->v[x] = (rand() % 0x100) & kk;
	ch8->pc += 2;
}

static void exec_drw_instr(uint16_t opcode, struct chip8 *ch8)
{
	uint16_t vx = (opcode & 0x0f00) >> 8;
	uint16_t vy = (opcode & 0x00f0) >> 4;
	uint16_t n = opcode & 0x000f;
	ch8->v[0xf] = 0;
	for (int y = 0; y < n; ++y) {
		uint8_t pixel = ch8->mem[ch8->i + y];
		for (int x = 0; x < 8; ++x) {
			if ((pixel & (0x80 >> x)) != 0) {
				int coord = ch8->v[vx] + x + (ch8->v[vy] + y) * 64;
				if (ch8->vbuffer[coord] == 1)
					ch8->v[0xf] = 1;
				ch8->vbuffer[coord] ^= 1;
			}
		}
	}
	ch8->draw = true;
	ch8->pc += 2;
}

static void exec_skp_instr(uint16_t opcode, struct chip8 *ch8)
{
	uint16_t x = (opcode & 0x0f00) >> 8;
	bool press = (opcode & 0x00ff) == 0x9e;
	if (press)
		ch8->pc += ch8->keypad[x] != 0 ? 4 : 2;
	else
		ch8->pc += ch8->keypad[x] == 0 ? 4 : 2;
}

static bool exec_f_instr(uint16_t opcode, struct chip8 *ch8)
{
	uint16_t x = (opcode & 0x0f00) >> 8;
	bool press = false;
	switch (opcode & 0x00ff) {
	case 0x07:
		ch8->v[x] = ch8->delay_timer;
		break;
	case 0x0a:
		for (int i = 0; i < 0xf; ++i) {
			if (ch8->keypad[i] != 0) {
				ch8->v[x] = i;
				press = true;
			}
		}
		if (!press)
			return false;
		break;
	case 0x15:
		ch8->delay_timer = ch8->v[x];
		break;
	case 0x18:
		ch8->sound_timer = ch8->v[x];
		break;
	case 0x1e:
		ch8->i = ch8->i + ch8->v[x];
		break;
	case 0x29:
		ch8->i = ch8->v[x] * 0x5;
		break;
	case 0x33:
		ch8->mem[ch8->i] = ch8->v[x] / 100;
		ch8->mem[ch8->i + 1] = (ch8->v[x] / 10) % 10;
		ch8->mem[ch8->i + 2] = ch8->v[x] % 10;
		break;
	case 0x55:
		for (int i = 0; i <= x; ++i)
			ch8->mem[ch8->i + i] = ch8->v[i];
		break;
	case 0x65:
		for (int i = 0; i <= x; i++)
			ch8->v[i] = ch8->mem[ch8->i + i];
		break;
	default:
		fprintf(stderr, "Unrecognized instruction: %04x\n", opcode);
		exit(EXIT_FAILURE);
	}
	ch8->pc += 2;
	return true;
}

void step_emulate(struct chip8 *ch8)
{
	uint16_t opcode = ch8->mem[ch8->pc] << 8 | ch8->mem[ch8->pc + 1];
//	printf("Executing instruction %04x\n", opcode);

	switch (opcode & 0xf000) {
	case 0x0000:
		exec_0_instr(opcode, ch8);
		break;
	case 0x1000:
		exec_jp_instr(opcode, ch8);
		break;
	case 0x2000:
		exec_call_instr(opcode, ch8);
		break;
	case 0x3000:
		exec_skip_byte_instr(opcode, ch8, true);
		break;
	case 0x4000:
		exec_skip_byte_instr(opcode, ch8, false);
		break;
	case 0x5000:
		exec_skip_reg_instr(opcode, ch8, true);
		break;
	case 0x6000:
		exec_ld_byte_instr(opcode, ch8);
		break;
	case 0x7000:
		exec_add_byte_instr(opcode, ch8);
		break;
	case 0x8000:
		exec_8_instr(opcode, ch8);
		break;
	case 0x9000:
		exec_skip_reg_instr(opcode, ch8, false);
		break;
	case 0xa000:
		exec_ld_i_instr(opcode, ch8);
		break;
	case 0xb000:
		exec_jp_v0_instr(opcode, ch8);
		break;
	case 0xc000:
		exec_rnd_instr(opcode, ch8);
		break;
	case 0xd000:
		exec_drw_instr(opcode, ch8);
		break;
	case 0xe000:
		exec_skp_instr(opcode, ch8);
		break;
	case 0xf000:
		if (!exec_f_instr(opcode, ch8))
			return;
		break;
	}
	if (ch8->delay_timer > 0)
		ch8->delay_timer--;
	if (ch8->sound_timer > 0) {
		// TODO implement sound
		ch8->sound_timer--;
	}
}
