#ifndef CPU_H
#define CPU_H

#include "gameboy.h"
#include "memory.h"

class MemoryRegister
{
private:
    Memory *memory;
    word address;

public:
    MemoryRegister(Memory *memory, word address)
        : memory(memory), 
          address(address) {};

    inline word get() const { return memory->get(address); };
    inline void set(word w) { memory->set(address, w); };
};

union Register {
    word w;
    struct {
        byte lo;
        byte hi;
    } b;
};

class CPU;
typedef void (*CommandHandler)(CPU *cpu);

typedef struct {
    byte code;
    byte length;
    byte cycles;
    const char *mnemonic;
    CommandHandler handler;
} Command;

class CPU
{
private:
    Register registerBank[6];

public:
    Memory *memory;

    byte ime; /* interrupt master enable */
    int cycles;
    bool debug;

    word &pc; byte &pc_hi; byte &pc_lo;
    word &sp;
    word &af; byte &a; byte &f;
    word &bc; byte &b; byte &c;
    word &de; byte &d; byte &e;
    word &hl; byte &h; byte &l;
    byte &ly;

    inline const byte flagZ() { return f & (1 << 7); };
    inline const byte flagN() { return f & (1 << 6); };
    inline const byte flagH() { return f & (1 << 5); };
    inline const byte flagC() { return f & (1 << 4); };

    inline void flagZ(byte v) { f = v ? (f | (1 << 7)) : (f & ~(1 << 7)); };
    inline void flagN(byte v) { f = v ? (f | (1 << 6)) : (f & ~(1 << 6)); };
    inline void flagH(byte v) { f = v ? (f | (1 << 5)) : (f & ~(1 << 5)); };
    inline void flagC(byte v) { f = v ? (f | (1 << 4)) : (f & ~(1 << 4)); };

    CPU(Memory *memory);
    virtual ~CPU();

    void step();
    word readWord();
    Command *findCommand(word address);
};

#endif
