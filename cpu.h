#ifndef CPU_H
#define CPU_H

#include <vector>

#include "word.h"
#include "memory.h"

class CPU;
class Debugger;

class Command
{
public:
    byte code;
    byte length;
    byte cycles;
    const char *mnemonic;
    Command(byte code, byte length, byte cycles, const char *mnemonic) :
        code(code),
        length(length),
        cycles(cycles),
        mnemonic(mnemonic) {};
    virtual ~Command() {};
    virtual void run(CPU *cpu) = 0;
};

typedef std::vector<Command*> Commands;

class CPU
{
private:
    word registerBank[6];
    Commands commands;

public:
    Memory *memory;
    Debugger *debugger;

    byte ime; /* interrupt master enable */
    int cycles;

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

    CPU(Memory *memory, Debugger *debugger);
    virtual ~CPU();

    void step();
    Command *findCommand(word address);
};

struct Condition { 
    virtual ~Condition() {};
    virtual bool operator()(CPU *cpu) = 0; 
};
struct NZ_Condition : Condition { virtual bool operator()(CPU *cpu) { return !cpu->flagZ(); }; };
struct NC_Condition : Condition { virtual bool operator()(CPU *cpu) { return !cpu->flagC(); }; };

#endif
