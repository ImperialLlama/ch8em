#include "chip8_instruction.h"

#include "SDL.h"
#include "macros.h"

void chip8_err(struct chip8 *ch8) {
    fprintf(stderr, "ERROR: %x at pc %u could not be executed.\n",
            ch8->opcode.instruction, ch8->pc);
}

void exec_clr_instr(struct chip8 *cpu) {
    memset(cpu->vbuffer, BLACK_COLOR, sizeof(cpu->vbuffer));
}

void exec_jmp_instr(struct chip8 *ch8, uint16_t address) {
    ch8->pc = address;
}

void exec_skip_instr(struct chip8 *ch8, bool expression) {
    if (expression) {
        ch8->pc += 2;
    }
}

void exec_call_instr(struct chip8 *ch8, uint16_t address) {
    ch8->stack[ch8->sp++] = ch8->pc;
    exec_jmp_instr(ch8, address);
}

void ass_reg_instr(struct chip8 *ch8, uint8_t register_id, uint8_t value) {
    ch8->v[register_id] = value;
}

void add_carry_instr(struct chip8 *ch8, uint8_t left, uint8_t right) {
    ch8->v[VF] = left + right > 255;
    ch8->v[ch8->opcode.x] = left + right;
}

void sub_borrow_instr(struct chip8 *ch8, uint8_t left, uint8_t right) {
    ch8->v[VF] = left > right;
    ch8->v[ch8->opcode.x] = left - right;
}

void lshift_instr(struct chip8 *ch8) {
    ch8->v[VF] = (bool) (ch8->v[ch8->opcode.x] & 0x80);
    ch8->v[ch8->opcode.x] <<= 2;
}

void rshift_instr(struct chip8 *ch8) {
    ch8->v[VF] = ch8->v[ch8->opcode.x] & 0b1;
    ch8->v[ch8->opcode.x] >>= 2;
}

void exec_rand_instr(struct chip8 *ch8) {
    ch8->v[ch8->opcode.x] = (rand() % 0xff) & ch8->opcode.kk;
}

void draw_screen_instr(struct chip8 *ch8) {
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

void wait_key_instr(struct chip8 *ch8) {
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

void ass_i_instr(struct chip8 *ch8, uint16_t value) {
    ch8->i = value;
}

void bcd_str_instr(struct chip8 *ch8) {
    ch8->mem[ch8->i] = ch8->v[ch8->opcode.x] / 100;
    ch8->mem[ch8->i + 1] = (ch8->v[ch8->opcode.x] / 10) % 10;
    ch8->mem[ch8->i + 2] = ch8->v[ch8->opcode.x] % 10;
}

void mem_str_instr(struct chip8 *ch8) {
    memcpy(ch8->mem + ch8->i, ch8->v, ch8->opcode.x + 1);
}

void mem_load_instr(struct chip8 *ch8) {
    memcpy(ch8->v, ch8->mem + ch8->i, ch8->opcode.x + 1);
}

void ass_time_instr(struct chip8 *ch8, uint8_t value) {
    ch8->delay_timer = value;
}

void ass_stime_instr(struct chip8 *ch8, uint8_t value) {
    ch8->sound_timer = value;
}
