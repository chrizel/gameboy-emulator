#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <iostream>

#include "cpu.h"
#include "references.h"
#include "memory.h"
#include "debugger.h"

struct flags {
    bool z;
    bool h;
    bool c;
};

static byte addByte(const byte &a, const byte &b, flags &f)
{
    byte r = a + b;
    f.z = r == 0;
    f.h = (r ^ b ^ a) & 0x10;
    f.c = r < a;
    return r;
}

static byte subByte(const byte &a, const byte &b, flags &f)
{
    byte r = a - b;
    f.z = r == 0;
    f.h = (r ^ b ^ a) & 0x10;
    f.c = r < a;
    return r;
}

static word addSignedByte(const word &a, const byte &b, flags &f)
{
    signed_byte sb = (signed_byte)b;
    word r = a + sb;
    f.z = r == 0;
    f.h = (r.value() ^ (word_t)b ^ a.value()) & 0x1000;
    f.c = (sb > 0) ? (r < a) : (r > a);
    return r;
}

static word addWord(const word &a, const word &b, flags &f)
{
    word r = a + b;
    f.z = r == 0;
    f.h = (r.value() ^ b.value() ^ a.value()) & 0x1000;
    f.c = r < a;
    return r;
}

static word subWord(const word &a, const word &b, flags &f)
{
    word r = a - b;
    f.z = r == 0;
    f.h = (r.value() ^ b.value() ^ a.value()) & 0x1000;
    f.c = r < a;
    return r;
}

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

        cpu->flagZ(cpu->a == 0);
        cpu->flagH(0);
        cpu->flagN(0);
        cpu->flagC(bit7);
    }
};

struct RLA_Instruction : public Instruction {
    void run() {
        byte bit7 = cpu->a & (1 << 7);
        byte cf = cpu->flagC() ? 1 : 0;
        cpu->a = cpu->a << 1;
        cpu->a = cf ? (cpu->a | 1) : (cpu->a & ~1);

        cpu->flagZ(cpu->a == 0);
        cpu->flagH(0);
        cpu->flagN(0);
        cpu->flagC(bit7);
    }
};

struct DAA_Instruction : public Instruction {
    void run() {
        byte add  = 0;
        bool newc = false;

        byte op = cpu->flagN() ? 1 : 0;
        byte c  = cpu->flagC() ? 1 : 0;
        byte hi = (cpu->a & 0xf0) >> 4;
        byte h  = cpu->flagH() ? 1 : 0;
        byte lo = cpu->a & 0x0f;

#define DAA_COND(_op, _c, _hi0, _hi1, _h, _lo0, _lo1, _add, _newc) \
        if (op == _op && c == _c && in_range(hi, _hi0, _hi1) && h == _h && in_range(lo, _lo0, _lo1)) \
            { add = _add; newc = _newc; break; }

        while (true) {
            // Table according to Z80 CPU User’s Manual page 166
            //        op | c  | hi-from | hi-to | h  | lo-from | lo-to | add   | c-out
            DAA_COND( 0,   0,   0x0,      0x9,    0,   0x0,      0x9,    0x00,   0 )
            DAA_COND( 0,   0,   0x0,      0x8,    0,   0xa,      0xf,    0x06,   0 )
            DAA_COND( 0,   0,   0x0,      0x9,    1,   0x0,      0x3,    0x06,   0 )
            DAA_COND( 0,   0,   0xa,      0xf,    0,   0x0,      0x9,    0x60,   1 )
            DAA_COND( 0,   0,   0x9,      0xf,    0,   0xa,      0xf,    0x66,   1 )
            DAA_COND( 0,   0,   0xa,      0xf,    1,   0x0,      0x3,    0x66,   1 )
            DAA_COND( 0,   1,   0x0,      0x2,    0,   0x0,      0x9,    0x60,   1 )
            DAA_COND( 0,   1,   0x0,      0x2,    0,   0xa,      0xf,    0x66,   1 )
            DAA_COND( 0,   1,   0x0,      0x3,    1,   0x0,      0x3,    0x66,   1 )
            DAA_COND( 1,   0,   0x0,      0x9,    0,   0x0,      0x9,    0x00,   0 )
            DAA_COND( 1,   0,   0x0,      0x8,    1,   0x6,      0xf,    0xfa,   0 )
            DAA_COND( 1,   1,   0x7,      0xf,    0,   0x0,      0x9,    0xa0,   1 )
            DAA_COND( 1,   1,   0x6,      0x7,    1,   0x6,      0xf,    0x9a,   1 )
            break;
        }

        cpu->a += add;
        cpu->flagZ(cpu->a == 0);
        cpu->flagH(0);
        cpu->flagC(newc);
    }

    inline bool in_range(const byte &a, const byte &from, const byte &to) {
        return a >= from && a <= to;
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
        flags f;
        ref0->set(addByte(ref0->get(), ref1->get(), f));
        cpu->flagZ(f.z);
        cpu->flagN(0);
        cpu->flagH(f.h);
        cpu->flagC(f.c);
        cpu->pc += length-1;
    }
};

template <>
struct ADD_Instruction<word> : public ReferenceInstruction<word> {
    void run() {
        flags f;
        ref0->set(addWord(ref0->get(), ref1->get(), f));
        cpu->flagN(0);
        cpu->flagH(f.h);
        cpu->flagC(f.c);
        cpu->pc += length-1;
    }
};

template <class T>
struct ADD_SP_Instruction : public ReferenceInstruction<T> {
};

template <>
struct ADD_SP_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        flags f;
        cpu->sp = addSignedByte(cpu->sp, ref0->get(), f);
        cpu->flagZ(0);
        cpu->flagN(0);
        cpu->flagH(f.h);
        cpu->flagC(f.c);
        cpu->pc += length-1;
    }
};

template <class T>
struct SUB_Instruction : public ReferenceInstruction<T> {
};

template <>
struct SUB_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        flags f;
        cpu->a = subByte(cpu->a, ref0->get(), f);
        cpu->flagZ(f.z);
        cpu->flagN(1);
        cpu->flagH(f.h);
        cpu->flagC(f.c);
        cpu->pc += length-1;
    }
};

template <class T>
struct ADC_Instruction : public ReferenceInstruction<T> {
};

template <>
struct ADC_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        flags f;
        ref0->set(addByte(ref0->get(), ref1->get() + (cpu->flagC() ? 1 : 0), f)); //TODO
        cpu->flagZ(f.z);
        cpu->flagN(0);
        cpu->flagH(f.h);
        cpu->flagC(f.c);
        cpu->pc += length-1;
    }
};

template <class T>
struct SBC_Instruction : public ReferenceInstruction<T> {
};

template <>
struct SBC_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        flags f;
        ref0->set(subByte(ref0->get(), ref1->get() + (cpu->flagC() ? 1 : 0), f)); //TODO
        cpu->flagZ(f.z);
        cpu->flagN(1);
        cpu->flagH(f.h);
        cpu->flagC(f.c);
        cpu->pc += length-1;
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
        cpu->flagH(0); // TODO: half carry flag
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
        cpu->flagH(0); // TODO: half carry flag
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
        flags f;
        byte n = ref0->get();
        subByte(cpu->a, n, f);
        cpu->flagZ(cpu->a == n);
        cpu->flagN(1);
        cpu->flagH(f.h);
        cpu->flagC(cpu->a < n);
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

struct RRCA_Instruction : public Instruction {
    void run() {
        byte bit0 = cpu->a & 1;
        cpu->a >>= 1;

        cpu->flagZ(cpu->a == 0);
        cpu->flagH(0);
        cpu->flagN(0);
        cpu->flagC(bit0);
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
struct SET_Instruction : public ReferenceInstruction<T> {
};

template <>
struct SET_Instruction<byte> : public ReferenceInstruction<byte> {
    void run() {
        byte b = ref0->get();
        b |= (1 << arg);
        ref0->set(b);
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
