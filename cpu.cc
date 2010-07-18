#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "debugger.h"
#include "gameboy.h"
#include "memory.h"

static word signed_addition(word w, byte b)
{
    Register r;
    r.w = w;
    r.b.lo += b;
    return r.w;
}


/*
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
*/

template <class T>
class Reference
{
public:
    virtual ~Reference() {};
    virtual T get() = 0;
    virtual void set(T v) = 0;
};

template <class T>
class RegisterReference : public Reference<T>
{
private:
    T &r;
public:
    RegisterReference(T &r) : r(r) {};
    virtual T get() { return r; };
    virtual void set(T v) { r = v; };
};

template <class T>
class MemoryReference : public Reference<T>
{
private:
    CPU *cpu;
    word &r;
    word add;
public:
    MemoryReference(CPU *cpu, word &r, word add=0) : cpu(cpu), r(r), add(add) {};
    virtual T get() {
        T v = cpu->memory->get<T>(r);
        r += add;
        return v;
    };
    virtual void set(T v) {
        cpu->memory->set<T>(r, v);
        r += add;
    };
};

class NOP_Command : public Command {
public:
    NOP_Command(byte code, byte length, byte cycles, const char *mnemonic)
        : Command(code, length, cycles, mnemonic) {};
    void run(CPU *cpu) {}; 
};

class JP_Command : public Command {
public:
    JP_Command(byte code, byte length, byte cycles, const char *mnemonic)
        : Command(code, length, cycles, mnemonic) {};
    void run(CPU *cpu) {
        cpu->pc = cpu->memory->get<word>(cpu->pc);
    };
};

class JR_NZ_r8_Command : public Command {
public:
    JR_NZ_r8_Command(byte code, byte length, byte cycles, const char *mnemonic)
        : Command(code, length, cycles, mnemonic) {};
    void run(CPU *cpu) {
        if (cpu->flagZ()) {
            cpu->cycles += 8;
            cpu->pc++;
        } else {
            byte v = cpu->memory->get<byte>(cpu->pc++);
            cpu->pc = signed_addition(cpu->pc, v);
            cpu->cycles += 4;
        }
    };
};

class XOR_Command : public Command {
private:
    Reference<byte> *ref;
public:
    XOR_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<byte> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~XOR_Command() { delete ref; };
    void run(CPU *cpu) {
        ref->set(ref->get() ^ ref->get());
        cpu->flagZ(ref->get() == 0);
        cpu->flagN(0);
        cpu->flagH(0);
        cpu->flagC(0);
    }; 
};

template <class T>
class LD_Command : public Command {
private:
    Reference<T> *ref0;
    Reference<T> *ref1;
public:
    LD_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<T> *ref0, Reference<T> *ref1)
        : Command(code, length, cycles, mnemonic), ref0(ref0), ref1(ref1) {};
    virtual ~LD_Command() { delete ref0; delete ref1; };
    void run(CPU *cpu) {
        ref0->set(ref1->get());
        cpu->pc += length-1;
    }; 
};

class DEC_Command : public Command {
private:
    Reference<byte> *ref;
public:
    DEC_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<byte> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~DEC_Command() { delete ref; };
    void run(CPU *cpu) {
        byte v = ref->get()-1;
        ref->set(v);
        cpu->flagZ(v == 0);
        cpu->flagN(1);
        cpu->flagH(0); // TOOD: half carry flag
    }
};

class SET_IME_Command : public Command {
private:
    byte value;
public:
    SET_IME_Command(byte code, byte length, byte cycles, const char *mnemonic, byte value)
        : Command(code, length, cycles, mnemonic), value(value) {};
    void run(CPU *cpu) {
        cpu->ime = value;
    }
};

CPU::CPU(Memory *memory)
    : memory(memory),
      ime(1),
      cycles(0),
      debug(true),
      pc(registerBank[0].w), pc_hi(registerBank[0].b.hi), pc_lo(registerBank[0].b.lo),
      sp(registerBank[1].w),
      af(registerBank[2].w), a(registerBank[2].b.hi), f(registerBank[2].b.lo),
      bc(registerBank[3].w), b(registerBank[3].b.hi), c(registerBank[3].b.lo),
      de(registerBank[4].w), d(registerBank[4].b.hi), e(registerBank[4].b.lo),
      hl(registerBank[5].w), h(registerBank[5].b.hi), l(registerBank[5].b.lo),
      ly(memory->getRef(0xff44))
{
    pc = 0x100;
    sp = 0xFFFE;
    af = 0x01;

    b  = 0x00;
    c  = 0x13;
    de = 0x00D8;
    hl = 0x014D;

    ly = 0x00;

    commands.push_back(new NOP_Command(  0x00, 1,    4, "NOP"));
    commands.push_back(new JP_Command(   0xc3, 3,   16, "JP a16"));
    commands.push_back(new XOR_Command(  0xaf, 1,    4, "XOR A",
                                                        new RegisterReference<byte>(a)));
    commands.push_back(new LD_Command<word>( 0x21, 3,   12, "LD HL,d16",
                                                        new RegisterReference<word>(hl),
                                                        new MemoryReference<word>(this, pc)));

    commands.push_back(new LD_Command<byte>( 0x0e, 2,    8, "LD C,d8",
                                                        new RegisterReference<byte>(c),
                                                        new MemoryReference<byte>(this, pc)));
    commands.push_back(new LD_Command<byte>( 0x06, 2,    8, "LD B,d8",
                                                        new RegisterReference<byte>(b),
                                                        new MemoryReference<byte>(this, pc)));

    commands.push_back(new LD_Command<byte>( 0x32, 1,    8, "LD (HL-),A",
                                                        new MemoryReference<byte>(this, hl, -1),
                                                        new RegisterReference<byte>(a)));

    commands.push_back(new DEC_Command( 0x05, 1, 4, "DEC B",
                                                        new RegisterReference<byte>(b)));

    commands.push_back(new JR_NZ_r8_Command( 0x20, 2, 0, "JR NZ,r8"));

    commands.push_back(new DEC_Command( 0x0d, 1, 4, "DEC C",
                                                        new RegisterReference<byte>(c)));

    commands.push_back(new LD_Command<byte>( 0x3e, 2, 8, "LD A,d8",
                                                        new RegisterReference<byte>(a),
                                                        new MemoryReference<byte>(this, pc)));

    commands.push_back(new SET_IME_Command( 0xf3, 1, 4, "DI", 0));
    commands.push_back(new SET_IME_Command( 0xfb, 1, 4, "EI", 1));
/*
static Command instructionSet[] = {
    {0x01, 3, 12, "LD BC,d16",  ch_LD_BC_d16},
    {0x05, 1,  4, "DEC B",      ch_DEC_B},
    {0x0b, 1,  8, "DEC BC",     ch_DEC_BC},
    {0x0c, 1,  4, "INC C",      ch_INC_C},
    {0x2a, 1,  8, "LD A,(HL+)", ch_LD_A_aHLI},
    {0x2f, 1,  4, "CPL",        ch_CPL},
    {0x31, 3, 12, "LD SP,d16",  ch_LD_SP_d16},
    {0x36, 2, 12, "LD (HL),d8", ch_LD_aHL_d8},
    {0x78, 1,  4, "LD A,B",     ch_LD_A_B},
    {0xb1, 1,  4, "OR C",       ch_OR_C},
    {0xcd, 3, 24, "CALL a16",   ch_CALL_a16},
    {0xc9, 1, 16, "RET",        ch_RET},
    {0xe0, 2, 12, "LDH (a8),A", ch_LDH_a8_A},
    {0xe2, 1,  8, "LD (C),A",   ch_LD_aC_A},
    {0xe6, 2,  8, "AND d8",     ch_AND_d8},
    {0xea, 3, 16, "LD (a16),A", ch_LD_a16_A},
    {0xf0, 2, 12, "LDH A,(a8)", ch_LDH_A_a8},
    {0xfe, 2,  8, "CP d8",      ch_CP_d8},
};
*/
}

CPU::~CPU()
{
    for (Commands::iterator i = commands.begin(); i != commands.end(); ++i)
        delete *i;
}

Command * CPU::findCommand(word address)
{
    word code = memory->get<byte>(address);
    for (Commands::iterator i = commands.begin(); i != commands.end(); ++i)
        if ((*i)->code == code)
            return *i;
    return 0;
}

void CPU::step()
{
    Command * cmd = findCommand(pc);
    if (cmd) {
        if (debug) {
            gbDebugPrintInstruction(this, pc);
            //gbDebugPrompt(this);
        }
        pc++;
        cmd->run(this);
        cycles += cmd->cycles;
    } else {
        fprintf(stderr, "%04x *** Unknown machine code: %02x\n", 
                pc, memory->get<byte>(pc));
        gbDebugPrompt(this);
        exit(1);
    }
}