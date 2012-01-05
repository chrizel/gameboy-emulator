#ifndef COMMANDS_H
#define COMMANDS_H

#include "cpu.h"
#include "references.h"

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

class SCF_Command : public Command {
public:
    SCF_Command(byte code, byte length, byte cycles, const char *mnemonic)
        : Command(code, length, cycles, mnemonic) {};
    void run(CPU *cpu) {
        cpu->flagN(0);
        cpu->flagH(0);
        cpu->flagC(1);
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
    signed_byte value;
public:
    INC_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<byte> *ref, signed_byte value)
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

class BIT_Command : public Command {
private:
    byte bit;
    Reference<byte> *ref;
public:
    BIT_Command(byte code, byte length, byte cycles, const char *mnemonic, byte bit, Reference<byte> *ref)
        : Command(code, length, cycles, mnemonic), bit(bit), ref(ref) {};
    virtual ~BIT_Command() { delete ref; };

    void run(CPU *cpu) {
        cpu->flagZ((ref->get() & (1 << bit)) == 0);
        cpu->flagN(0);
        cpu->flagH(1);
    }
};

class SLA_Command : public Command {
private:
    Reference<byte> *ref;
public:
    SLA_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<byte> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~SLA_Command() { delete ref; };
    void run(CPU *cpu) {
        byte v = ref->get();
        byte bit7 = v & (1 << 7);
        v <<= 1;
        ref->set(v);

        cpu->flagZ(v == 0);
        cpu->flagH(0);
        cpu->flagN(0);
        cpu->flagC(bit7);
    };
};

class SRL_Command : public Command {
private:
    Reference<byte> *ref;
public:
    SRL_Command(byte code, byte length, byte cycles, const char *mnemonic, Reference<byte> *ref)
        : Command(code, length, cycles, mnemonic), ref(ref) {};
    virtual ~SRL_Command() { delete ref; };
    void run(CPU *cpu) {
        byte v = ref->get();
        byte bit0 = v & 1;
        v >>= 1;
        ref->set(v);

        cpu->flagZ(v == 0);
        cpu->flagH(0);
        cpu->flagN(0);
        cpu->flagC(bit0);
    };
};

class CB_Command : public Command {
private:
    Commands commands;
public:
    CB_Command(byte code, byte length, byte cycles, const char *mnemonic, CPU *cpu)
        : Command(code, length, cycles, mnemonic)
    {
        commands.push_back(new SLA_Command(0x27, 2, 8, "SLA A", new RegisterReference<byte>(cpu->a)));
        commands.push_back(new SWAP_Command(0x37, 2, 8, "SWAP A", new RegisterReference<byte>(cpu->a)));
        commands.push_back(new SWAP_Command(0x33, 2, 8, "SWAP E", new RegisterReference<byte>(cpu->e)));
        commands.push_back(new SRL_Command(0x3f, 2, 8, "SRL A", new RegisterReference<byte>(cpu->a)));
        commands.push_back(new BIT_Command(0x5f, 2, 8, "BIT 3,A", 3, new RegisterReference<byte>(cpu->a)));
        commands.push_back(new BIT_Command(0x7f, 2, 8, "BIT 7,A", 7, new RegisterReference<byte>(cpu->a)));
        commands.push_back(new BIT_Command(0x40, 2, 8, "BIT 0,B", 0, new RegisterReference<byte>(cpu->b)));
        commands.push_back(new BIT_Command(0x48, 2, 8, "BIT 1,B", 1, new RegisterReference<byte>(cpu->b)));
        commands.push_back(new BIT_Command(0x50, 2, 8, "BIT 2,B", 2, new RegisterReference<byte>(cpu->b)));
        commands.push_back(new BIT_Command(0x58, 2, 8, "BIT 3,B", 3, new RegisterReference<byte>(cpu->b)));
        commands.push_back(new BIT_Command(0x60, 2, 8, "BIT 4,B", 4, new RegisterReference<byte>(cpu->b)));
        commands.push_back(new BIT_Command(0x68, 2, 8, "BIT 5,B", 5, new RegisterReference<byte>(cpu->b)));
        commands.push_back(new BIT_Command(0x7e, 2, 16, "BIT 7,(HL)", 7, new MemoryReference<byte>(cpu, cpu->hl)));
        commands.push_back(new RES_Command(0x87, 2, 8, "RES 0,A", 0, new RegisterReference<byte>(cpu->a)));
        commands.push_back(new RES_Command(0x86, 2, 16, "RES 0,(HL)", 0, new MemoryReference<byte>(cpu, cpu->hl)));
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

#endif
