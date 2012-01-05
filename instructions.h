#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <iostream>

#include "cpu.h"
#include "references.h"
#include "memory.h"
#include "debugger.h"

struct Instruction
{
    byte code;
    byte length;
    byte cycles0;
    byte cycles1;
    byte arg;
    const char *mnemonic;
    Condition *condition;
    CPU *cpu;

    Instruction() : code(0), length(0), cycles0(0), cycles1(0), arg(0), mnemonic(), condition(0) {}
    virtual ~Instruction() {
        if (condition)
            delete condition;
    }

    virtual void run() = 0;
};

template <class T>
struct ReferenceInstruction : public Instruction
{
    Reference<T> *ref0;
    Reference<T> *ref1;

    ReferenceInstruction() : ref0(0), ref1(0) {}
    virtual ~ReferenceInstruction() {
        if (ref0)
            delete ref0;
        if (ref1)
            delete ref1;
    }
};

struct NOP_Instruction : public Instruction {
    void run() {}
};

template <class T>
struct JP_Instruction : public ReferenceInstruction<T> {
};

template <>
struct JP_Instruction<word> : public ReferenceInstruction<word> {
    void run() {
        cpu->pc = ref0->get();
    }
};

template <class T>
struct JR_Instruction : public ReferenceInstruction<T> {
};

template <>
struct JR_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        byte v = ref0->get();
        cpu->pc++;
        cpu->pc.addSignedByte(v);
    }
};

template <class T>
struct XOR_Instruction : public ReferenceInstruction<T> {
};

template <>
struct XOR_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        cpu->a ^= ref0->get();
        cpu->flagZ(cpu->a == 0);
        cpu->flagN(0);
        cpu->flagH(0);
        cpu->flagC(0);
        cpu->pc += length-1;
    }
};

struct RLCA_Instruction : public Instruction {
    void run() {
        byte bit7 = cpu->a & (1 << 7);
        cpu->a = cpu->a << 1;
        cpu->a = bit7 ? (cpu->a | 1) : (cpu->a & ~1);

        cpu->flagZ(0);
        cpu->flagH(0);
        cpu->flagN(0);
        cpu->flagC(bit7);
    }
};

template <class T>
struct OR_Instruction : public ReferenceInstruction<T> {
};

template <>
struct OR_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        cpu->a |= ref0->get();
        cpu->flagZ(cpu->a == 0);
        cpu->flagN(0);
        cpu->flagH(0);
        cpu->flagC(0);
        cpu->pc += length-1;
    }
};

template <class T>
struct AND_Instruction : public ReferenceInstruction<T> {
};

template <>
struct AND_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        cpu->a &= ref0->get();
        cpu->flagZ(cpu->a == 0);
        cpu->flagN(0);
        cpu->flagH(1);
        cpu->flagC(0);
        cpu->pc += length-1;
    };
};

struct CPL_Instruction : public Instruction {
    void run() {
        cpu->a = ~cpu->a;
        cpu->flagN(1);
        cpu->flagH(1);
    }
};

struct SCF_Instruction : public Instruction {
    void run() {
        cpu->flagN(0);
        cpu->flagH(0);
        cpu->flagC(1);
    }
};

template <class T>
struct LD_Instruction : public ReferenceInstruction<T> {
};

template <>
struct LD_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        ref0->set(ref1->get());
        cpu->pc += length-1;
    }
};

template <>
struct LD_Instruction<word> : public ReferenceInstruction<word> {
    void run() {
        ref0->set(ref1->get());
        cpu->pc += length-1;
    }
};

template <class T>
struct ADD_Instruction : public ReferenceInstruction<T> {
};

template <>
struct ADD_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        byte x = ref0->get();
        byte v = x + ref1->get();
        ref0->set(v);
        cpu->flagZ(v == 0);
        cpu->flagN(0);
        cpu->flagH(0); // TODO: half carry flag
        cpu->flagC(v < x); // TODO: carry flag
    }
};

template <>
struct ADD_Instruction<word> : public ReferenceInstruction<word> {
    void run() {
        word x = ref0->get();
        word v = x + ref1->get();
        ref0->set(v);
        cpu->flagN(0);
        cpu->flagH(0); // TODO: half carry flag
        cpu->flagC(v < x); // TODO: carry flag
    }
};

template <class T>
struct INC_Instruction : public ReferenceInstruction<T> {
};

template <>
struct INC_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        byte v = ref0->get() + (byte)1;
        ref0->set(v);
        cpu->flagZ(v == 0);
        cpu->flagN(0);
        cpu->flagH(0); // TOOD: half carry flag
    }
};

template <>
struct INC_Instruction<word> : public ReferenceInstruction<word> {
    void run() {
        ref0->set(ref0->get() + (word)1);
    }
};

template <class T>
struct DEC_Instruction : public ReferenceInstruction<T> {
};

template <>
struct DEC_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        byte v = ref0->get() - 1;
        ref0->set(v);
        cpu->flagZ(v == 0);
        cpu->flagN(1);
        cpu->flagH(0); // TOOD: half carry flag
    }
};

template <>
struct DEC_Instruction<word> : public ReferenceInstruction<word> {
    void run() {
        ref0->set(ref0->get() - 1);
    }
};

template <class T>
struct CP_Instruction : public ReferenceInstruction<T> {
};

template <>
struct CP_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        byte a = cpu->a;
        byte n = ref0->get();
        cpu->flagZ(a == n);
        cpu->flagN(1);
        cpu->flagH(0); // TODO: half carry flag
        cpu->flagC(a < n); // TODO: carry flag
        cpu->pc += length-1;
    }
};

template <class T>
struct CALL_Instruction : public ReferenceInstruction<T> {
};

template <>
struct CALL_Instruction<word> : public ReferenceInstruction<word> {
    void run() {
        word address = ref0->get();
        cpu->pc += length-1;
        cpu->sp--;
        cpu->memory->set(cpu->sp, cpu->pc_hi);
        cpu->sp--;
        cpu->memory->set(cpu->sp, cpu->pc_lo);
        cpu->pc = address;
    }
};

struct RST_Instruction : public Instruction {
    void run() {
        cpu->sp--;
        cpu->memory->set(cpu->sp, cpu->pc_hi);
        cpu->sp--;
        cpu->memory->set(cpu->sp, cpu->pc_lo);
        cpu->pc = word(arg, 0x00);
    }
};

template <class T>
struct PUSH_Instruction : public ReferenceInstruction<T> {
};

template <>
struct PUSH_Instruction<word> : public ReferenceInstruction<word> {
    void run() {
        word value = ref0->get();
        cpu->sp--;
        cpu->memory->set(cpu->sp, value.hi());
        cpu->sp--;
        cpu->memory->set(cpu->sp, value.lo());
    }
};

template <class T>
struct POP_Instruction : public ReferenceInstruction<T> {
};

template <>
struct POP_Instruction<word> : public ReferenceInstruction<word> {
    void run() {
        word value;
        value.setlo(cpu->memory->get<byte>(cpu->sp++));
        value.sethi(cpu->memory->get<byte>(cpu->sp++));
        ref0->set(value);
    }
};

struct RET_Instruction : public Instruction {
    void run() {
        cpu->pc_lo = cpu->memory->get<byte>(cpu->sp++);
        cpu->pc_hi = cpu->memory->get<byte>(cpu->sp++);
    }
};

struct RETI_Instruction : public Instruction {
    void run() {
        cpu->pc_lo = cpu->memory->get<byte>(cpu->sp++);
        cpu->pc_hi = cpu->memory->get<byte>(cpu->sp++);
        cpu->ime = 1;
    }
};

struct DI_Instruction : public Instruction {
    void run() {
        cpu->ime = 0;
    }
};

struct EI_Instruction : public Instruction {
    void run() {
        cpu->ime = 1;
    }
};

template <class T>
struct SWAP_Instruction : public ReferenceInstruction<T> {
};

template <>
struct SWAP_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        byte v = ref0->get();
        v = (v << 4) | (v >> 4);
        ref0->set(v);
        cpu->flagZ(v == 0);
        cpu->flagN(0);
        cpu->flagH(0);
        cpu->flagC(0);
    }
};

template <class T>
struct RES_Instruction : public ReferenceInstruction<T> {
};

template <>
struct RES_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        byte v = ref0->get();
        v = v & ~(1 << arg);
        ref0->set(v);
    }
};

template <class T>
struct BIT_Instruction : public ReferenceInstruction<T> {
};

template <>
struct BIT_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        cpu->flagZ((ref0->get() & (1 << arg)) == 0);
        cpu->flagN(0);
        cpu->flagH(1);
    }
};

template <class T>
struct SLA_Instruction : public ReferenceInstruction<T> {
};

template <>
struct SLA_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        byte v = ref0->get();
        byte bit7 = v & (1 << 7);
        v <<= 1;
        ref0->set(v);

        cpu->flagZ(v == 0);
        cpu->flagH(0);
        cpu->flagN(0);
        cpu->flagC(bit7);
    }
};

template <class T>
struct SRL_Instruction : public ReferenceInstruction<T> {
};

template <>
struct SRL_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        byte v = ref0->get();
        byte bit0 = v & 1;
        v >>= 1;
        ref0->set(v);

        cpu->flagZ(v == 0);
        cpu->flagH(0);
        cpu->flagN(0);
        cpu->flagC(bit0);
    }
};

class InstructionSet;

struct CB_Instruction : public Instruction {
    InstructionSet *instructionSet;
    virtual ~CB_Instruction();
    CB_Instruction();
    void run();
};

#endif
