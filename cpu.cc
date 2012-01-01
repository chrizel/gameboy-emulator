#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "debugger.h"
#include "word.h"
#include "memory.h"

template <class T>
class Reference
{
public:
    virtual ~Reference() {};
    virtual T get() = 0;
    virtual void set(T v) = 0;
};

template <class T>
class ValueReference : public Reference<T>
{
private:
    T r;
public:
    ValueReference(T r): r(r) {};
    virtual T get() { return r; };
    virtual void set(T v) {
        fprintf(stderr, "*** ValueReference can not be set\n");
    }
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

template <class T, class R>
class MemoryReference : public Reference<T>
{
private:
    CPU *cpu;
    R &r;
    R add;
public:
    MemoryReference(CPU *cpu, R &r, R add=0) : cpu(cpu), r(r), add(add) {};
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

template <class T>
class Memory8Reference : public Reference<T>
{
private:
    CPU *cpu;
    word &r;
public:
    Memory8Reference(CPU *cpu, word &r): cpu(cpu), r(r) {};
    virtual T get() {
        return cpu->memory->get<T>( word(cpu->memory->get<byte>(r), 0xff));
    };
    virtual void set(T v) {
        cpu->memory->set<T>(word(cpu->memory->get<byte>(r), 0xff), v);
    };
};

class NOP_Command : public Command {
public:
    NOP_Command(byte code, byte length, byte cycles, const char *mnemonic)
        : Command(code, length, cycles, mnemonic) {};
    void run(CPU *cpu) {}; 
};

class JP_Command : public Command {
private:
    Reference<word> *ref;
public:
    JP_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<word> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~JP_Command() { delete ref; };
    void run(CPU *cpu) {
        cpu->pc = ref->get();
    };
};

class JP_a16_Command : public Command {
private:
    Condition *condition;
public:
    JP_a16_Command(byte code, byte length, byte cycles, const char *mnemonic, Condition *condition)
        : Command(code, length, cycles, mnemonic), condition(condition) {};
    virtual ~JP_a16_Command() { delete condition; };
    void run(CPU *cpu) {
        if ((*condition)(cpu)) {
            cpu->pc = cpu->memory->get<word>(cpu->pc);
            cpu->cycles += 16;
        } else {
            cpu->cycles += 12;
            cpu->pc++;
            cpu->pc++;
        }
    };
};

class JR_Command : public Command {
private:
    Condition *condition;
public:
    JR_Command(byte code, byte length, byte cycles, const char *mnemonic)
        : Command(code, length, cycles, mnemonic) {};
    void run(CPU *cpu) {
        byte v = cpu->memory->get<byte>(cpu->pc++);
        cpu->pc.addSignedByte(v);
    };
};

class JR_r8_Command : public Command {
private:
    Condition *condition;
public:
    JR_r8_Command(byte code, byte length, byte cycles, const char *mnemonic, Condition *condition)
        : Command(code, length, cycles, mnemonic), condition(condition) {};
    virtual ~JR_r8_Command() { delete condition; };
    void run(CPU *cpu) {
        if ((*condition)(cpu)) {
            byte v = cpu->memory->get<byte>(cpu->pc++);
            cpu->pc.addSignedByte(v);
            cpu->cycles += 4;
        } else {
            cpu->cycles += 8;
            cpu->pc++;
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
        cpu->a ^= ref->get();
        cpu->flagZ(cpu->a == 0);
        cpu->flagN(0);
        cpu->flagH(0);
        cpu->flagC(0);
        cpu->pc += length-1;
    }; 
};

class RLCA_Command : public Command {
public:
    RLCA_Command(byte code, byte length, byte cycles, const char *mnemonic)
        : Command(code, length, cycles, mnemonic) {};
    void run(CPU *cpu) {
        byte bit7 = cpu->a & (1 << 7);
        cpu->a = cpu->a << 1;
        cpu->a = bit7 ? (cpu->a | 1) : (cpu->a & ~1);

        cpu->flagZ(0);
        cpu->flagH(0);
        cpu->flagN(0);
        cpu->flagC(bit7);
    };
};

class OR_Command : public Command {
private:
    Reference<byte> *ref;
public:
    OR_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<byte> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~OR_Command() { delete ref; };
    void run(CPU *cpu) {
        cpu->a |= ref->get();
        cpu->flagZ(cpu->a == 0);
        cpu->flagN(0);
        cpu->flagH(0);
        cpu->flagC(0);
        cpu->pc += length-1;
    };
};

class AND_Command : public Command {
private:
    Reference<byte> *ref;
public:
    AND_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<byte> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~AND_Command() { delete ref; };
    void run(CPU *cpu) {
        cpu->a &= ref->get();
        cpu->flagZ(cpu->a == 0);
        cpu->flagN(0);
        cpu->flagH(1);
        cpu->flagC(0);
        cpu->pc += length-1;
    };
};

class CPL_Command : public Command {
public:
    CPL_Command(byte code, byte length, byte cycles, const char *mnemonic)
        : Command(code, length, cycles, mnemonic) {};
    void run(CPU *cpu) {
        cpu->a = ~cpu->a;
        cpu->flagN(1);
        cpu->flagH(1);
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

template <class T>
class ADD_Command : public Command {
};

template <>
class ADD_Command<byte> : public Command {
private:
    Reference<byte> *ref0;
    Reference<byte> *ref1;
public:
    ADD_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<byte> *ref0, Reference<byte> *ref1)
        : Command(code, length, cycles, mnemonic), ref0(ref0), ref1(ref1) {};
    virtual ~ADD_Command() { delete ref0; delete ref1; };
    void run(CPU *cpu) {
        byte v = ref0->get() + ref1->get();
        ref0->set(v);
        cpu->flagZ(v == 0);
        cpu->flagN(0);
        cpu->flagH(0); // TODO: half carry flag
        cpu->flagC(0); // TODO: carry flag
    }
};

template <>
class ADD_Command<word> : public Command {
private:
    Reference<word> *ref0;
    Reference<word> *ref1;
public:
    ADD_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<word> *ref0, Reference<word> *ref1)
        : Command(code, length, cycles, mnemonic), ref0(ref0), ref1(ref1) {};
    virtual ~ADD_Command() { delete ref0; delete ref1; };
    void run(CPU *cpu) {
        word v = ref0->get() + ref1->get();
        ref0->set(v);
        cpu->flagN(0);
        cpu->flagH(0); // TODO: half carry flag
        cpu->flagC(0); // TODO: carry flag
    }
};

template <class T>
class INC_Command : public Command {
};

template <>
class INC_Command<byte> : public Command {
private:
    Reference<byte> *ref;
    byte value;
public:
    INC_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<byte> *ref, byte value)
        : Command(code, length, cycles, mnemonic), ref(ref), value(value) {};
    virtual ~INC_Command() { delete ref; };
    void run(CPU *cpu) {
        byte v = ref->get() + value;
        ref->set(v);
        cpu->flagZ(v == 0);
        cpu->flagN(value < 0);
        cpu->flagH(0); // TOOD: half carry flag
    }
};

template <>
class INC_Command<word> : public Command {
private:
    Reference<word> *ref;
    word value;
public:
    INC_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<word> *ref, word value)
        : Command(code, length, cycles, mnemonic), ref(ref), value(value) {};
    virtual ~INC_Command() { delete ref; };
    void run(CPU *cpu) {
        ref->set(ref->get() + value);
    }
};


class CP_Command : public Command {
private:
    Reference<byte> *ref;
public:
    CP_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<byte> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~CP_Command() { delete ref; };
    void run(CPU *cpu) {
        cpu->flagZ(ref->get() == cpu->a);
        cpu->flagN(1);
        cpu->flagH(0); // TODO: half carry flag
        cpu->flagC(0); // TODO: carry flag
        cpu->pc += length-1;
    }
};

class CALL_Command : public Command {
private:
    Reference<word> *ref;
public:
    CALL_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<word> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~CALL_Command() { delete ref; };
    void run(CPU *cpu) {
        word address = ref->get();
        cpu->pc += length-1;
        cpu->sp--;
        cpu->memory->set(cpu->sp, cpu->pc_hi);
        cpu->sp--;
        cpu->memory->set(cpu->sp, cpu->pc_lo);
        cpu->pc = address;
    }
};

class RST_Command : public Command {
private:
    Reference<byte> *ref;
public:
    RST_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<byte> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~RST_Command() { delete ref; };
    void run(CPU *cpu) {
        cpu->sp--;
        cpu->memory->set(cpu->sp, cpu->pc_hi);
        cpu->sp--;
        cpu->memory->set(cpu->sp, cpu->pc_lo);
        cpu->pc = word(ref->get(), 0x00);
    }
};

class PUSH_Command : public Command {
private:
    Reference<word> *ref;
public:
    PUSH_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<word> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~PUSH_Command() { delete ref; };
    void run(CPU *cpu) {
        word value = ref->get();
        cpu->sp--;
        cpu->memory->set(cpu->sp, value.hi());
        cpu->sp--;
        cpu->memory->set(cpu->sp, value.lo());
    }
};

class POP_Command : public Command {
private:
    Reference<word> *ref;
public:
    POP_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<word> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~POP_Command() { delete ref; };
    void run(CPU *cpu) {
        word value;
        value.setlo(cpu->memory->get<byte>(cpu->sp++));
        value.sethi(cpu->memory->get<byte>(cpu->sp++));
        ref->set(value);
    }
};

class RET_Command : public Command {
private:
    Condition *condition;
public:
    RET_Command(byte code, byte length, byte cycles, const char *mnemonic, Condition *condition = 0)
        : Command(code, length, cycles, mnemonic), condition(condition) {};
    void run(CPU *cpu) {
        if (condition) {
            if ((*condition)(cpu)) {
                cpu->pc_lo = cpu->memory->get<byte>(cpu->sp++);
                cpu->pc_hi = cpu->memory->get<byte>(cpu->sp++);
                cpu->cycles += 20;
            } else {
                cpu->cycles += 8;
            }
        } else {
            cpu->pc_lo = cpu->memory->get<byte>(cpu->sp++);
            cpu->pc_hi = cpu->memory->get<byte>(cpu->sp++);
            cpu->cycles += 16;
        }
    }
};

class RETI_Command : public Command {
public:
    RETI_Command(byte code, byte length, byte cycles, const char *mnemonic)
        : Command(code, length, cycles, mnemonic) {};
    void run(CPU *cpu) {
        cpu->pc_lo = cpu->memory->get<byte>(cpu->sp++);
        cpu->pc_hi = cpu->memory->get<byte>(cpu->sp++);
        cpu->ime = 1;
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

class SWAP_Command : public Command {
private:
    Reference<byte> *ref;
public:
    SWAP_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<byte> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~SWAP_Command() { delete ref; };

    void run(CPU *cpu) {
        byte v = ref->get();
        v = (v << 4) | (v >> 4);
        ref->set(v);
        cpu->flagZ(v == 0);
        cpu->flagN(0);
        cpu->flagH(0);
        cpu->flagC(0);
    }
};

class RES_Command : public Command {
private:
    byte bit;
    Reference<byte> *ref;
public:
    RES_Command(byte code, byte length, byte cycles, const char *mnemonic, byte bit, Reference<byte> *ref)
        : Command(code, length, cycles, mnemonic), bit(bit), ref(ref) {};
    virtual ~RES_Command() { delete ref; };

    void run(CPU *cpu) {
        byte v = ref->get();
        v = v & ~(1 << bit);
        ref->set(v);
    }
};

class CB_Command : public Command {
private:
    Commands commands;
public:
    CB_Command(byte code, byte length, byte cycles, const char *mnemonic, CPU *cpu)
        : Command(code, length, cycles, mnemonic)
    {
        commands.push_back(new SWAP_Command(0x37, 2, 8, "SWAP A", new RegisterReference<byte>(cpu->a)));
        commands.push_back(new RES_Command(0x87, 2, 8, "RES 0,A", 0, new RegisterReference<byte>(cpu->a)));
    }

    Command * findCommand(byte code)
    {
        for (Commands::iterator i = commands.begin(); i != commands.end(); ++i)
            if ((*i)->code == code)
                return *i;
        return 0;
    }

    void run(CPU *cpu) {
        byte code = cpu->memory->get<byte>(cpu->pc);
        Command *command = findCommand(code);
        if (command) {
            cpu->pc++;
            command->run(cpu);
            cpu->cycles += command->cycles;
        } else {
            fprintf(stderr, "%04x *** Unknown CB machine code: %02x\n", cpu->pc.value(), code);
            cpu->debugger->prompt(cpu);
        }

    }
};


CPU::CPU(Memory *memory, Debugger *debugger)
    : memory(memory),
      debugger(debugger),
      ime(1),
      cycles(0),
      pc(registerBank[0]), pc_hi(registerBank[0].hi()), pc_lo(registerBank[0].lo()),
      sp(registerBank[1]),
      af(registerBank[2]), a(registerBank[2].hi()), f(registerBank[2].lo()),
      bc(registerBank[3]), b(registerBank[3].hi()), c(registerBank[3].lo()),
      de(registerBank[4]), d(registerBank[4].hi()), e(registerBank[4].lo()),
      hl(registerBank[5]), h(registerBank[5].hi()), l(registerBank[5].lo()),
      ly(memory->getRef(0xff44)), IE(memory->getRef(0xffff)), IF(memory->getRef(0xff0f))
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

    commands.push_back(new JP_Command(   0xc3, 3,   16, "JP a16",  new MemoryReference<word, word>(this, pc)));
    commands.push_back(new JP_Command(   0xe9, 1,    4, "JP (HL)", new RegisterReference<word>(hl)));

    commands.push_back(new JP_a16_Command( 0xca, 3, 0, "JP Z,a16", new Z_Condition()));

    commands.push_back(new XOR_Command(  0xaf, 1,    4, "XOR A", new RegisterReference<byte>(a)));
    commands.push_back(new XOR_Command(  0xa9, 1,    4, "XOR C", new RegisterReference<byte>(c)));
    commands.push_back(new LD_Command<word>( 0x21, 3,   12, "LD HL,d16",
                                                        new RegisterReference<word>(hl),
                                                        new MemoryReference<word, word>(this, pc)));

    commands.push_back(new LD_Command<byte>( 0x0e, 2,    8, "LD C,d8",
                                                        new RegisterReference<byte>(c),
                                                        new MemoryReference<byte, word>(this, pc)));
    commands.push_back(new LD_Command<byte>( 0x06, 2,    8, "LD B,d8",
                                                        new RegisterReference<byte>(b),
                                                        new MemoryReference<byte, word>(this, pc)));
    commands.push_back(new LD_Command<byte>( 0x16, 2,    8, "LD D,d8",
                                                        new RegisterReference<byte>(d),
                                                        new MemoryReference<byte, word>(this, pc)));

    commands.push_back(new LD_Command<byte>( 0x32, 1,    8, "LD (HL-),A",
                                                        new MemoryReference<byte, word>(this, hl, -1),
                                                        new RegisterReference<byte>(a)));

    commands.push_back(new LD_Command<byte>( 0x47, 1, 4, "LD B,A", new RegisterReference<byte>(b), new RegisterReference<byte>(a)));
    commands.push_back(new LD_Command<byte>( 0x4f, 1, 4, "LD C,A", new RegisterReference<byte>(c), new RegisterReference<byte>(a)));
    commands.push_back(new LD_Command<byte>( 0x79, 1, 4, "LD A,C", new RegisterReference<byte>(a), new RegisterReference<byte>(c)));
    commands.push_back(new LD_Command<byte>( 0x5f, 1, 4, "LD E,A", new RegisterReference<byte>(e), new RegisterReference<byte>(a)));
    commands.push_back(new LD_Command<byte>( 0x7c, 1, 4, "LD A,H", new RegisterReference<byte>(a), new RegisterReference<byte>(h)));

    commands.push_back(new INC_Command<byte>( 0x04, 1, 4, "INC B", new RegisterReference<byte>(b), 1));
    commands.push_back(new INC_Command<byte>( 0x0c, 1, 4, "INC C", new RegisterReference<byte>(c), 1));
    commands.push_back(new INC_Command<byte>( 0x2c, 1, 4, "INC L", new RegisterReference<byte>(l), 1));


    commands.push_back(new INC_Command<byte>( 0x05, 1, 4, "DEC B", new RegisterReference<byte>(b), -1));
    commands.push_back(new INC_Command<byte>( 0x0d, 1, 4, "DEC C", new RegisterReference<byte>(c), -1));
    commands.push_back(new INC_Command<byte>( 0x35, 1, 12, "DEC (HL)", new MemoryReference<byte, word>(this, hl), -1));

    commands.push_back(new JR_r8_Command( 0x20, 2, 0, "JR NZ,r8", new NZ_Condition()));
    commands.push_back(new JR_r8_Command( 0x28, 2, 0, "JR N,r8", new Z_Condition()));
    commands.push_back(new JR_Command( 0x18, 2, 12, "JR r8"));

    commands.push_back(new LD_Command<byte>( 0x3e, 2, 8, "LD A,d8",
                                                        new RegisterReference<byte>(a),
                                                        new MemoryReference<byte, word>(this, pc)));

    commands.push_back(new SET_IME_Command( 0xf3, 1, 4, "DI", 0));
    commands.push_back(new SET_IME_Command( 0xfb, 1, 4, "EI", 1));

    commands.push_back(new LD_Command<byte>( 0xe0, 2, 12, "LDH (a8),A",
                                                        new Memory8Reference<byte>(this, pc),
                                                        new RegisterReference<byte>(a)));
    commands.push_back(new LD_Command<byte>( 0xf0, 2, 12, "LDH A,(a8)",
                                                        new RegisterReference<byte>(a),
                                                        new Memory8Reference<byte>(this, pc)));
    commands.push_back(new CP_Command( 0xfe, 2, 8, "CP d8", new MemoryReference<byte, word>(this, pc)));

    commands.push_back(new LD_Command<byte>( 0x36, 2, 12, "LD (HL),d8",
                                                        new MemoryReference<byte, word>(this, hl),
                                                        new MemoryReference<byte, word>(this, pc)));
    commands.push_back(new LD_Command<byte>( 0xea, 3, 16, "LD (a16),A",
                                                        new MemoryReference<byte, word>(this, pc),
                                                        new RegisterReference<byte>(a)));
    commands.push_back(new LD_Command<byte>( 0xfa, 3, 16, "LD A,(a16)",
                                                        new RegisterReference<byte>(a),
                                                        new MemoryReference<byte, word>(this, pc)));
    commands.push_back(new LD_Command<word>( 0x31, 3, 12, "LD SP,d16",
                                                        new RegisterReference<word>(sp),
                                                        new MemoryReference<word, word>(this, pc)));
    commands.push_back(new LD_Command<byte>( 0x2a, 1, 8, "LD A,(HL+)",
                                                        new RegisterReference<byte>(a),
                                                        new MemoryReference<byte, word>(this, hl, 1)));
    commands.push_back(new LD_Command<byte>( 0x22, 1, 8, "LD (HL+),A",
                                                        new MemoryReference<byte, word>(this, hl, 1),
                                                        new RegisterReference<byte>(a)));
    commands.push_back(new LD_Command<byte>( 0x56, 1, 8, "LD D,(HL)",
                                                        new RegisterReference<byte>(d),
                                                        new MemoryReference<byte, word>(this, hl)));
    commands.push_back(new LD_Command<byte>( 0x5e, 1, 8, "LD E,(HL)",
                                                        new RegisterReference<byte>(e),
                                                        new MemoryReference<byte, word>(this, hl)));
    commands.push_back(new LD_Command<byte>( 0x7e, 1, 8, "LD A,(HL)",
                                                        new RegisterReference<byte>(a),
                                                        new MemoryReference<byte, word>(this, hl)));

    commands.push_back(new LD_Command<byte>( 0xe2, 1, 8, "LD (C),A",
                                                        new MemoryReference<byte, byte>(this, c),
                                                        new RegisterReference<byte>(a)));

    commands.push_back(new CALL_Command( 0xcd, 3, 24, "CALL a16", new MemoryReference<word, word>(this, pc)));

    commands.push_back(new LD_Command<word>( 0x01, 3, 12, "LD BC,d16",
                                                        new RegisterReference<word>(bc),
                                                        new MemoryReference<word, word>(this, pc)));
    commands.push_back(new INC_Command<word>( 0x0b, 1, 8, "DEC BC", new RegisterReference<word>(bc), -1));
    commands.push_back(new INC_Command<word>( 0x13, 1, 8, "INC DE", new RegisterReference<word>(de), 1));
    commands.push_back(new INC_Command<word>( 0x23, 1, 8, "INC HL", new RegisterReference<word>(hl), 1));

    commands.push_back(new LD_Command<byte>( 0x78, 1, 4, "LD A,B",
                                                        new RegisterReference<byte>(a),
                                                        new RegisterReference<byte>(b)));

    commands.push_back(new OR_Command( 0xb0, 1, 4, "OR B", new RegisterReference<byte>(b)));
    commands.push_back(new OR_Command( 0xb1, 1, 4, "OR C", new RegisterReference<byte>(c)));

    commands.push_back(new RET_Command( 0xc9, 1, 0, "RET"));

    commands.push_back(new CPL_Command( 0x2f, 1, 4, "CPL"));

    commands.push_back(new AND_Command( 0xe6, 2, 8, "AND d8", new MemoryReference<byte, word>(this, pc)));
    commands.push_back(new AND_Command( 0xa1, 1, 4, "AND C", new RegisterReference<byte>(c)));
    commands.push_back(new AND_Command( 0xa7, 1, 4, "AND A", new RegisterReference<byte>(a)));

    commands.push_back(new RLCA_Command( 0x07, 1, 4, "RLCA"));

    commands.push_back(new RET_Command( 0xd0, 1, 0, "RET NC", new NC_Condition()));

    commands.push_back(new LD_Command<word>( 0x11, 3, 12, "LD DE,d16",
                                                        new RegisterReference<word>(de),
                                                        new MemoryReference<word, word>(this, pc)));

    commands.push_back(new LD_Command<byte>( 0x12, 1, 8, "LD (DE),A",
                                                        new MemoryReference<byte, word>(this, de),
                                                        new RegisterReference<byte>(a)));
    commands.push_back(new LD_Command<byte>( 0x1a, 1, 8, "LD A,(DE)",
                                                        new RegisterReference<byte>(a),
                                                        new MemoryReference<byte, word>(this, de)));

    commands.push_back(new PUSH_Command( 0xc5, 1, 16, "PUSH BC", new RegisterReference<word>(bc)));
    commands.push_back(new PUSH_Command( 0xd5, 1, 16, "PUSH DE", new RegisterReference<word>(de)));
    commands.push_back(new PUSH_Command( 0xe5, 1, 16, "PUSH HL", new RegisterReference<word>(hl)));
    commands.push_back(new PUSH_Command( 0xf5, 1, 16, "PUSH AF", new RegisterReference<word>(af)));

    commands.push_back(new POP_Command(  0xc1, 1, 12, "POP BC", new RegisterReference<word>(bc)));
    commands.push_back(new POP_Command(  0xd1, 1, 12, "POP DE", new RegisterReference<word>(de)));
    commands.push_back(new POP_Command(  0xe1, 1, 12, "POP HL", new RegisterReference<word>(hl)));
    commands.push_back(new POP_Command(  0xf1, 1, 12, "POP AF", new RegisterReference<word>(af)));

    commands.push_back(new CB_Command( 0xcb, 1, 0, "CB", this));

    commands.push_back(new RST_Command( 0xef, 1, 16, "RST 28H", new ValueReference<byte>(0x28)));

    commands.push_back(new ADD_Command<byte>( 0x87, 1, 4, "ADD A,A", new RegisterReference<byte>(a), new RegisterReference<byte>(a)));

    commands.push_back(new ADD_Command<word>( 0x19, 1, 8, "ADD HL,DE", new RegisterReference<word>(hl), new RegisterReference<word>(de)));

    commands.push_back(new RETI_Command( 0xd9, 1, 16, "RETI"));
}

CPU::~CPU()
{
    for (Commands::iterator i = commands.begin(); i != commands.end(); ++i)
        delete *i;
}

Command * CPU::findCommand(word address)
{
    byte code = memory->get<byte>(address);
    for (Commands::iterator i = commands.begin(); i != commands.end(); ++i)
        if ((*i)->code == code)
            return *i;
    return 0;
}

void CPU::step()
{
    // Check for interrupts...
    byte irqs = IE & IF;
    if (ime && irqs) {
        if (irqs & (INT_VBLANK+1))
            callInterrupt(INT_VBLANK,  0x0040);
        else if (irqs & (INT_LCDSTAT+1))
            callInterrupt(INT_LCDSTAT, 0x0048);
        else if (irqs & (INT_TIMER+1))
            callInterrupt(INT_TIMER,   0x0050);
        else if (irqs & (INT_SERIAL+1))
            callInterrupt(INT_SERIAL,  0x0058);
        else if (irqs & (INT_JOYPAD+1))
            callInterrupt(INT_JOYPAD,  0x0060);
    }

    // Process command...
    Command * cmd = findCommand(pc);
    if (cmd) {
        debugger->handleInstruction(this, pc);
        pc++;
        cmd->run(this);
        cycles += cmd->cycles;
    } else {
        fprintf(stderr, "%04x *** Unknown machine code: %02x\n", 
                pc.value(), memory->get<byte>(pc));
        debugger->prompt(this);
    }
}

void CPU::callInterrupt(Interrupt irq, word address)
{
    // Reset interrupt flag in IF
    IF &= ~(1 << irq);

    // Reset IME
    ime = 0;

    // Push current PC on stack...
    sp--;
    memory->set(sp, pc_hi);
    sp--;
    memory->set(sp, pc_lo);

    // Set new PC to interrupt address
    pc = address;
}

void CPU::requestInterrupt(Interrupt irq)
{
    IF |= (1 << irq);
}
