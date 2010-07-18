#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "debugger.h"
#include "gameboy.h"

static word word_from_bytes(byte lo, byte hi)
{
    reg r;
    r.b.lo = lo;
    r.b.hi = hi;
    return r.w;
}

static word read_word(GameBoy *gb)
{
    reg r;
    r.b.lo = MEM(gb, REG_PC(gb)++);
    r.b.hi = MEM(gb, REG_PC(gb)++);
    return r.w;
}

static word signed_addition(word w, byte b)
{
    reg r;
    r.w = w;
    r.b.lo += b;
    return r.w;
}

void gbCPUInit(GameBoy *gb)
{
    gb->debug = 1;
    gb->cpu.cycles = 0;
    gb->cpu.ime = 1;

    REG_PC(gb) = 0x100;
    REG_SP(gb) = 0xFFFE;

    REG_AF(gb) = 0x01;
    REG_B(gb)  = 0x00;
    REG_C(gb)  = 0x13;
    REG_DE(gb) = 0x00D8;
    REG_HL(gb) = 0x014D;

    REG_LY(gb) = 0x00;
}

static void ch_DEC_B(GameBoy *gb)
{
    REG_B(gb)--;
    FLAG_Z_SET(gb, REG_B(gb) == 0);
    FLAG_N_SET(gb, 1);
    FLAG_H_SET(gb, 0); // TODO: half carry flag
}

static void ch_INC_C(GameBoy *gb)
{
    REG_C(gb)++;
    FLAG_Z_SET(gb, REG_C(gb) == 0);
    FLAG_N_SET(gb, 0);
    FLAG_H_SET(gb, 0); // TODO: half carry flag
}

static void ch_DEC_BC(GameBoy *gb)
{
    REG_BC(gb)--;
}

static void ch_DEC_C(GameBoy *gb)
{
    REG_C(gb)--;
    FLAG_Z_SET(gb, REG_C(gb) == 0);
    FLAG_N_SET(gb, 1);
    FLAG_H_SET(gb, 0); // TODO: half carry flag
}

static void ch_LD_B_d8(GameBoy *gb)
{
    REG_B(gb) = MEM(gb, REG_PC(gb)++);
}

static void ch_LD_A_d8(GameBoy *gb)
{
    REG_A(gb) = MEM(gb, REG_PC(gb)++);
}

static void ch_LD_C_d8(GameBoy *gb)
{
    REG_C(gb) = MEM(gb, REG_PC(gb)++);
}

static void ch_JR_NZ_r8(GameBoy *gb)
{
    word address = MEM(gb, REG_PC(gb)++);
    if (!FLAG_Z(gb)) {
        REG_PC(gb) = signed_addition(REG_PC(gb), address);
        gb->cpu.cycles += 4;
    }
    gb->cpu.cycles += 8;
}

static void ch_LD_HL_d16(GameBoy *gb)
{
    REG_HL(gb) = read_word(gb);
}

static void ch_LD_BC_d16(GameBoy *gb)
{
    REG_BC(gb) = read_word(gb);
}

static void ch_LD_SP_d16(GameBoy *gb)
{
    REG_SP(gb) = read_word(gb);
}

static void ch_LD_aHLD_A(GameBoy *gb)
{
    MEM(gb, REG_HL(gb)) = REG_A(gb);
    REG_HL(gb)--;
}

static void ch_LD_A_aHLI(GameBoy *gb)
{
    REG_A(gb) = MEM(gb, REG_HL(gb));
    REG_HL(gb)++;
}

static void ch_XOR_A(GameBoy *gb)
{
    REG_A(gb) ^= REG_A(gb);
    FLAG_Z_SET(gb, REG_A(gb) == 0);
    FLAG_N_SET(gb, 0);
    FLAG_H_SET(gb, 0);
    FLAG_C_SET(gb, 0);
}

static void ch_JP_a16(GameBoy *gb)
{
    REG_PC(gb) = read_word(gb);
}

static void ch_DI(GameBoy *gb)
{
    gb->cpu.ime = 0;
}

static void ch_EI(GameBoy *gb)
{
    gb->cpu.ime = 1;
}

static void ch_LDH_a8_A(GameBoy *gb)
{
    word address = word_from_bytes(MEM(gb, REG_PC(gb)++), 0xff);
    MEM(gb, address) = REG_A(gb);
}

static void ch_LDH_A_a8(GameBoy *gb)
{
    word address = word_from_bytes(MEM(gb, REG_PC(gb)++), 0xff);
    REG_A(gb) = MEM(gb, address);
}

static void ch_CP_d8(GameBoy *gb)
{
    FLAG_Z_SET(gb, MEM(gb, REG_PC(gb)++) == REG_A(gb));
    FLAG_N_SET(gb, 1);
    FLAG_H_SET(gb, 0); // TODO: half carry flag
    FLAG_C_SET(gb, 0); // TODO: carry flag
}

static void ch_LD_aHL_d8(GameBoy *gb)
{
    MEM(gb, REG_HL(gb)) = REG_PC(gb)++;
}

static void ch_LD_a16_A(GameBoy *gb)
{
    MEM(gb, read_word(gb)) = REG_A(gb);
}

static void ch_LD_aC_A(GameBoy *gb)
{
    word address = word_from_bytes(REG_C(gb), 0xff);
    MEM(gb, address) = REG_A(gb);
}

static void ch_CALL_a16(GameBoy *gb)
{
    word address = read_word(gb);
    REG_SP(gb)--;
    MEM(gb, REG_SP(gb)) = gb->cpu.pc.b.hi;
    REG_SP(gb)--;
    MEM(gb, REG_SP(gb)) = gb->cpu.pc.b.lo;
    REG_PC(gb) = address;
}

static void ch_RET(GameBoy *gb)
{
    gb->cpu.pc.b.lo = MEM(gb, REG_SP(gb)++);
    gb->cpu.pc.b.hi = MEM(gb, REG_SP(gb)++);
}

static void ch_LD_A_B(GameBoy *gb)
{
    REG_A(gb) = REG_B(gb);
}

static void ch_OR_C(GameBoy *gb)
{
    REG_A(gb) |= REG_C(gb);
    FLAG_Z_SET(gb, REG_A(gb) == 0);
    FLAG_N_SET(gb, 0);
    FLAG_H_SET(gb, 0);
    FLAG_C_SET(gb, 0);
}

static void ch_AND_d8(GameBoy *gb)
{
    REG_A(gb) &= REG_PC(gb)++;
    FLAG_Z_SET(gb, REG_A(gb) == 0);
    FLAG_N_SET(gb, 0);
    FLAG_H_SET(gb, 1);
    FLAG_C_SET(gb, 0);
}

static void ch_CPL(GameBoy *gb)
{
    REG_A(gb) = ~REG_A(gb);
    FLAG_N_SET(gb, 1);
    FLAG_H_SET(gb, 1);
}

static Command instructionSet[] = {
    {0x00, 1,  4, "NOP",        NULL},
    {0x01, 3, 12, "LD BC,d16",  ch_LD_BC_d16},
    {0x05, 1,  4, "DEC B",      ch_DEC_B},
    {0x06, 2,  8, "LD B,d8",    ch_LD_B_d8},
    {0x0b, 1,  8, "DEC BC",     ch_DEC_BC},
    {0x0c, 1,  4, "INC C",      ch_INC_C},
    {0x0d, 1,  4, "DEC C",      ch_DEC_C},
    {0x0e, 2,  8, "LD C,d8",    ch_LD_C_d8},
    {0x20, 2,  0, "JR NZ,r8",   ch_JR_NZ_r8},
    {0x21, 3, 12, "LD HL,d16",  ch_LD_HL_d16},
    {0x2a, 1,  8, "LD A,(HL+)", ch_LD_A_aHLI},
    {0x2f, 1,  4, "CPL",        ch_CPL},
    {0x31, 3, 12, "LD SP,d16",  ch_LD_SP_d16},
    {0x32, 1,  8, "LD (HL-),A", ch_LD_aHLD_A},
    {0x36, 2, 12, "LD (HL),d8", ch_LD_aHL_d8},
    {0x3e, 2,  8, "LD A,d8",    ch_LD_A_d8},
    {0x78, 1,  4, "LD A,B",     ch_LD_A_B},
    {0xaf, 1,  4, "XOR A",      ch_XOR_A},
    {0xb1, 1,  4, "OR C",       ch_OR_C},
    {0xc3, 3, 16, "JP a16",     ch_JP_a16},
    {0xcd, 3, 24, "CALL a16",   ch_CALL_a16},
    {0xc9, 1, 16, "RET",        ch_RET},
    {0xe0, 2, 12, "LDH (a8),A", ch_LDH_a8_A},
    {0xe2, 1,  8, "LD (C),A",   ch_LD_aC_A},
    {0xe6, 2,  8, "AND d8",     ch_AND_d8},
    {0xea, 3, 16, "LD (a16),A", ch_LD_a16_A},
    {0xf0, 2, 12, "LDH A,(a8)", ch_LDH_A_a8},
    {0xf3, 1,  4, "DI",         ch_DI},
    {0xfb, 1,  4, "EI",         ch_EI},
    {0xfe, 2,  8, "CP d8",      ch_CP_d8},
    {0x00, 0,  0, NULL,         NULL},
};

Command * gbCPUFindCommand(GameBoy *gb, word address)
{
    int i = 0;
    Command * cmd;
    word code = MEM(gb, address);
    while ((cmd = &instructionSet[i++])->length) {
        if (cmd->code == code) {
            return cmd;
        }
    }

    return NULL;
}

void gbCPUStep(GameBoy *gb)
{
    Command * cmd = gbCPUFindCommand(gb, REG_PC(gb));
    if (cmd) {
        if (gb->debug) {
            gbDebugPrintInstruction(gb, REG_PC(gb));
            //gbDebugPrompt(gb);
        }
        REG_PC(gb)++;
        if (cmd->handler) {
            cmd->handler(gb);
            gb->cpu.cycles += cmd->cycles;
        }
    } else {
        fprintf(stderr, "%04x *** Unknown machine code: %02x\n", 
                REG_PC(gb), MEM(gb, REG_PC(gb)));
        gbDebugPrompt(gb);
        exit(1);
    }
}
