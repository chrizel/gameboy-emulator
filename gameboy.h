#ifndef GAMEBOY_H
#define GAMEBOY_H

#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;

typedef union {
    word w;
    struct {
        byte lo;
        byte hi;
    } b;
} reg;

class GameBoy 
{
public:
    struct {
        byte ime; /* interrupt master enable */
        reg pc;
        reg sp;
        reg af;
        reg bc;
        reg de;
        reg hl;
        int cycles;
    } cpu;
    byte *mem;
    byte debug;

    GameBoy(const char *file);
    virtual ~GameBoy();

    void process();
};

#define MEM(gb, x) (gb->mem[x])

#define REG_PC(gb) (gb->cpu.pc.w)
#define REG_SP(gb) (gb->cpu.sp.w)
#define REG_AF(gb) (gb->cpu.af.w)
#define REG_BC(gb) (gb->cpu.bc.w)
#define REG_DE(gb) (gb->cpu.de.w)
#define REG_HL(gb) (gb->cpu.hl.w)

#define REG_A(gb) (gb->cpu.af.b.hi)
#define REG_F(gb) (gb->cpu.af.b.lo)
#define REG_B(gb) (gb->cpu.bc.b.hi)
#define REG_C(gb) (gb->cpu.bc.b.lo)
#define REG_D(gb) (gb->cpu.de.b.hi)
#define REG_E(gb) (gb->cpu.de.b.lo)
#define REG_H(gb) (gb->cpu.hl.b.hi)
#define REG_L(gb) (gb->cpu.hl.b.lo)

#define REG_LY(gb) MEM(gb, 0xff44)

#define FLAG_Z(gb) (REG_F(gb) & (1 << 7))
#define FLAG_N(gb) (REG_F(gb) & (1 << 6))
#define FLAG_H(gb) (REG_F(gb) & (1 << 5))
#define FLAG_C(gb) (REG_F(gb) & (1 << 4))

#define FLAG_Z_SET(gb, v)  \
    REG_F(gb) = ((v) ? (REG_F(gb) | (1 << 7)) : (REG_F(gb) & ~(1 << 7)))
#define FLAG_N_SET(gb, v)  \
    REG_F(gb) = ((v) ? (REG_F(gb) | (1 << 6)) : (REG_F(gb) & ~(1 << 6)))
#define FLAG_H_SET(gb, v)  \
    REG_F(gb) = ((v) ? (REG_F(gb) | (1 << 5)) : (REG_F(gb) & ~(1 << 5)))
#define FLAG_C_SET(gb, v) \
    REG_F(gb) = ((v) ? (REG_F(gb) | (1 << 4)) : (REG_F(gb) & ~(1 << 4)))

#define GB_DISPLAY_WIDTH 160
#define GB_DISPLAY_HEIGHT 144

#endif
