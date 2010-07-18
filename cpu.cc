#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "debugger.h"
#include "gameboy.h"
#include "memory.h"

static word word_from_bytes(byte lo, byte hi)
{
    Register r;
    r.b.lo = lo;
    r.b.hi = hi;
    return r.w;
}

word CPU::readWord()
{
    Register r;
    r.b.lo = memory->get(pc++);
    r.b.hi = memory->get(pc++);
    return r.w;
}

static word signed_addition(word w, byte b)
{
    Register r;
    r.w = w;
    r.b.lo += b;
    return r.w;
}

CPU::CPU(Memory *memory)
    : memory(memory),
      ly(memory, 0xff44),
      ime(1),
      cycles(0),
      debug(true),
      pc(registerBank[0].w), pc_hi(registerBank[0].b.hi), pc_lo(registerBank[0].b.lo),
      sp(registerBank[1].w),
      af(registerBank[2].w), a(registerBank[2].b.hi), f(registerBank[2].b.lo),
      bc(registerBank[3].w), b(registerBank[3].b.hi), c(registerBank[3].b.lo),
      de(registerBank[4].w), d(registerBank[4].b.hi), e(registerBank[4].b.lo),
      hl(registerBank[5].w), h(registerBank[5].b.hi), l(registerBank[5].b.lo)

{
    pc = 0x100;
    sp = 0xFFFE;
    af = 0x01;

    b  = 0x00;
    c  = 0x13;
    de = 0x00D8;
    hl = 0x014D;

    ly.set(0x00);
}

CPU::~CPU()
{
}

static void ch_DEC_B(CPU *cpu)
{
    cpu->b--;
    cpu->flagZ(cpu->b == 0);
    cpu->flagN(1);
    cpu->flagH(0); // TODO: half carry flag
}

static void ch_INC_C(CPU *cpu)
{
    cpu->c++;
    cpu->flagZ(cpu->c == 0);
    cpu->flagN(0);
    cpu->flagH(0); // TODO: half carry flag
}

static void ch_DEC_BC(CPU *cpu)
{
    cpu->bc--;
}

static void ch_DEC_C(CPU *cpu)
{
    cpu->c--;
    cpu->flagZ(cpu->c == 0);
    cpu->flagN(1);
    cpu->flagH(0); // TODO: half carry flag
}

static void ch_LD_B_d8(CPU *cpu)
{
    cpu->b = cpu->memory->get(cpu->pc++);
}

static void ch_LD_A_d8(CPU *cpu)
{
    cpu->a = cpu->memory->get(cpu->pc++);
}

static void ch_LD_C_d8(CPU *cpu)
{
    cpu->c = cpu->memory->get(cpu->pc++);
}

static void ch_JR_NZ_r8(CPU *cpu)
{
    word address = cpu->memory->get(cpu->pc++);
    if (!cpu->flagZ()) {
        cpu->pc = signed_addition(cpu->pc, address);
        cpu->cycles += 4;
    }
    cpu->cycles += 8;
}

static void ch_LD_HL_d16(CPU *cpu)
{
    cpu->hl = cpu->readWord();
}

static void ch_LD_BC_d16(CPU *cpu)
{
    cpu->bc = cpu->readWord();
}

static void ch_LD_SP_d16(CPU *cpu)
{
    cpu->sp = cpu->readWord();
}

static void ch_LD_aHLD_A(CPU *cpu)
{
    cpu->memory->set(cpu->hl, cpu->a);
    cpu->hl--;
}

static void ch_LD_A_aHLI(CPU *cpu)
{
    cpu->a = cpu->memory->get(cpu->hl);
    cpu->hl++;
}

static void ch_XOR_A(CPU *cpu)
{
    cpu->a ^= cpu->a;
    cpu->flagZ(cpu->a == 0);
    cpu->flagN(0);
    cpu->flagH(0);
    cpu->flagC(0);
}

static void ch_JP_a16(CPU *cpu)
{
    cpu->pc = cpu->readWord();
}

static void ch_DI(CPU *cpu)
{
    cpu->ime = 0;
}

static void ch_EI(CPU *cpu)
{
    cpu->ime = 1;
}

static void ch_LDH_a8_A(CPU *cpu)
{
    word address = word_from_bytes(cpu->memory->get(cpu->pc++), 0xff);
    cpu->memory->set(address, cpu->a);
}

static void ch_LDH_A_a8(CPU *cpu)
{
    word address = word_from_bytes(cpu->memory->get(cpu->pc++), 0xff);
    cpu->a = cpu->memory->get(address);
}

static void ch_CP_d8(CPU *cpu)
{
    cpu->flagZ(cpu->memory->get(cpu->pc++) == cpu->a);
    cpu->flagN(1);
    cpu->flagH(0); // TODO: half carry flag
    cpu->flagC(0); // TODO: carry flag

}

static void ch_LD_aHL_d8(CPU *cpu)
{
    cpu->memory->set(cpu->hl, cpu->pc++);
}

static void ch_LD_a16_A(CPU *cpu)
{
    cpu->memory->set(cpu->readWord(), cpu->a);
}

static void ch_LD_aC_A(CPU *cpu)
{
    word address = word_from_bytes(cpu->c, 0xff);
    cpu->memory->set(address, cpu->a);
}

static void ch_CALL_a16(CPU *cpu)
{
    word address = cpu->readWord();
    cpu->sp--;
    cpu->memory->set(cpu->sp, cpu->pc_hi);
    cpu->sp--;
    cpu->memory->set(cpu->sp, cpu->pc_lo);
    cpu->pc = address;
}

static void ch_RET(CPU *cpu)
{
    cpu->pc_lo = cpu->memory->get(cpu->sp++);
    cpu->pc_hi = cpu->memory->get(cpu->sp++);
}

static void ch_LD_A_B(CPU *cpu)
{
    cpu->a = cpu->b;
}

static void ch_OR_C(CPU *cpu)
{
    cpu->a |= cpu->c;
    cpu->flagZ(cpu->a == 0);
    cpu->flagN(0);
    cpu->flagH(0);
    cpu->flagC(0);
}

static void ch_AND_d8(CPU *cpu)
{
    cpu->a &= cpu->pc++;
    cpu->flagZ(cpu->a == 0);
    cpu->flagN(0);
    cpu->flagH(1);
    cpu->flagC(0);
}

static void ch_CPL(CPU *cpu)
{
    cpu->a = ~cpu->a;
    cpu->flagN(1);
    cpu->flagH(1);
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

Command * gbCPUFindCommand(CPU *cpu, word address)
{
    int i = 0;
    Command * cmd;
    word code = cpu->memory->get(address);
    while ((cmd = &instructionSet[i++])->length) {
        if (cmd->code == code) {
            return cmd;
        }
    }

    return NULL;
}

void CPU::step()
{
    Command * cmd = gbCPUFindCommand(this, pc);
    if (cmd) {
        if (debug) {
            gbDebugPrintInstruction(this, pc);
            //gbDebugPrompt(this);
        }
        pc++;
        if (cmd->handler) {
            cmd->handler(this);
            cycles += cmd->cycles;
        }
    } else {
        fprintf(stderr, "%04x *** Unknown machine code: %02x\n", 
                pc, memory->get(pc));
        gbDebugPrompt(this);
        exit(1);
    }
}
