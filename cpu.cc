#include <iostream>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "debugger.h"
#include "word.h"
#include "memory.h"
#include "instructions.h"
#include "references.h"

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

    instructions.push_back(new NOP_Instruction(  0x00, 1,    4, "NOP"));

    instructions.push_back(new JP_Instruction(   0xc3, 3,   16, "JP a16",  new MemoryReference<word>(this, pc)));
    instructions.push_back(new JP_Instruction(   0xe9, 1,    4, "JP (HL)", new RegisterReference<word>(hl)));

    instructions.push_back(new JP_a16_Instruction( 0xc2, 3, 0, "JP NZ,a16", new NZ_Condition()));
    instructions.push_back(new JP_a16_Instruction( 0xca, 3, 0, "JP Z,a16", new Z_Condition()));

    instructions.push_back(new XOR_Instruction(  0xaf, 1,    4, "XOR A", new RegisterReference<byte>(a)));
    instructions.push_back(new XOR_Instruction(  0xa9, 1,    4, "XOR C", new RegisterReference<byte>(c)));
    instructions.push_back(new LD_Instruction<word>( 0x21, 3,   12, "LD HL,d16",
                                                        new RegisterReference<word>(hl),
                                                        new MemoryReference<word>(this, pc)));

    instructions.push_back(new LD_Instruction<byte>( 0x3e, 2, 8, "LD A,d8",
                                                        new RegisterReference<byte>(a),
                                                        new MemoryReference<byte>(this, pc)));
    instructions.push_back(new LD_Instruction<byte>( 0x1e, 2,    8, "LD E,d8",
                                                        new RegisterReference<byte>(e),
                                                        new MemoryReference<byte>(this, pc)));
    instructions.push_back(new LD_Instruction<byte>( 0x0e, 2,    8, "LD C,d8",
                                                        new RegisterReference<byte>(c),
                                                        new MemoryReference<byte>(this, pc)));
    instructions.push_back(new LD_Instruction<byte>( 0x06, 2,    8, "LD B,d8",
                                                        new RegisterReference<byte>(b),
                                                        new MemoryReference<byte>(this, pc)));
    instructions.push_back(new LD_Instruction<byte>( 0x16, 2,    8, "LD D,d8",
                                                        new RegisterReference<byte>(d),
                                                        new MemoryReference<byte>(this, pc)));
    instructions.push_back(new LD_Instruction<byte>( 0x26, 2,    8, "LD H,d8",
                                                        new RegisterReference<byte>(h),
                                                        new MemoryReference<byte>(this, pc)));

    instructions.push_back(new LD_Instruction<byte>( 0x47, 1, 4, "LD B,A", new RegisterReference<byte>(b), new RegisterReference<byte>(a)));
    instructions.push_back(new LD_Instruction<byte>( 0x40, 1, 4, "LD B,B", new RegisterReference<byte>(b), new RegisterReference<byte>(b)));
    instructions.push_back(new LD_Instruction<byte>( 0x4f, 1, 4, "LD C,A", new RegisterReference<byte>(c), new RegisterReference<byte>(a)));
    instructions.push_back(new LD_Instruction<byte>( 0x79, 1, 4, "LD A,C", new RegisterReference<byte>(a), new RegisterReference<byte>(c)));
    instructions.push_back(new LD_Instruction<byte>( 0x7a, 1, 4, "LD A,D", new RegisterReference<byte>(a), new RegisterReference<byte>(d)));
    instructions.push_back(new LD_Instruction<byte>( 0x7b, 1, 4, "LD A,E", new RegisterReference<byte>(a), new RegisterReference<byte>(e)));
    instructions.push_back(new LD_Instruction<byte>( 0x57, 1, 4, "LD D,A", new RegisterReference<byte>(d), new RegisterReference<byte>(a)));
    instructions.push_back(new LD_Instruction<byte>( 0x54, 1, 4, "LD D,H", new RegisterReference<byte>(d), new RegisterReference<byte>(h)));
    instructions.push_back(new LD_Instruction<byte>( 0x5f, 1, 4, "LD E,A", new RegisterReference<byte>(e), new RegisterReference<byte>(a)));
    instructions.push_back(new LD_Instruction<byte>( 0x5d, 1, 4, "LD E,L", new RegisterReference<byte>(e), new RegisterReference<byte>(l)));
    instructions.push_back(new LD_Instruction<byte>( 0x67, 1, 4, "LD H,A", new RegisterReference<byte>(h), new RegisterReference<byte>(a)));
    instructions.push_back(new LD_Instruction<byte>( 0x60, 1, 4, "LD H,B", new RegisterReference<byte>(h), new RegisterReference<byte>(b)));
    instructions.push_back(new LD_Instruction<byte>( 0x62, 1, 4, "LD H,D", new RegisterReference<byte>(h), new RegisterReference<byte>(d)));
    instructions.push_back(new LD_Instruction<byte>( 0x69, 1, 4, "LD L,C", new RegisterReference<byte>(l), new RegisterReference<byte>(c)));
    instructions.push_back(new LD_Instruction<byte>( 0x6f, 1, 4, "LD L,A", new RegisterReference<byte>(l), new RegisterReference<byte>(a)));
    instructions.push_back(new LD_Instruction<byte>( 0x6b, 1, 4, "LD L,E", new RegisterReference<byte>(l), new RegisterReference<byte>(e)));
    instructions.push_back(new LD_Instruction<byte>( 0x7c, 1, 4, "LD A,H", new RegisterReference<byte>(a), new RegisterReference<byte>(h)));
    instructions.push_back(new LD_Instruction<byte>( 0x7d, 1, 4, "LD A,L", new RegisterReference<byte>(a), new RegisterReference<byte>(l)));

    instructions.push_back(new INC_Instruction<byte>( 0x3c, 1, 4, "INC A", new RegisterReference<byte>(a), 1));
    instructions.push_back(new INC_Instruction<byte>( 0x04, 1, 4, "INC B", new RegisterReference<byte>(b), 1));
    instructions.push_back(new INC_Instruction<byte>( 0x0c, 1, 4, "INC C", new RegisterReference<byte>(c), 1));
    instructions.push_back(new INC_Instruction<byte>( 0x1c, 1, 4, "INC E", new RegisterReference<byte>(e), 1));
    instructions.push_back(new INC_Instruction<byte>( 0x2c, 1, 4, "INC L", new RegisterReference<byte>(l), 1));
    instructions.push_back(new INC_Instruction<byte>( 0x24, 1, 4, "INC H", new RegisterReference<byte>(h), 1));
    instructions.push_back(new INC_Instruction<byte>( 0x34, 1, 12, "INC (HL)", new MemoryReference<byte>(this, hl), 1));

    instructions.push_back(new INC_Instruction<byte>( 0x3d, 1, 4, "DEC A", new RegisterReference<byte>(a), -1));
    instructions.push_back(new INC_Instruction<byte>( 0x05, 1, 4, "DEC B", new RegisterReference<byte>(b), -1));
    instructions.push_back(new INC_Instruction<byte>( 0x0d, 1, 4, "DEC C", new RegisterReference<byte>(c), -1));
    instructions.push_back(new INC_Instruction<byte>( 0x2d, 1, 4, "DEC L", new RegisterReference<byte>(l), -1));
    instructions.push_back(new INC_Instruction<byte>( 0x35, 1, 12, "DEC (HL)", new MemoryReference<byte>(this, hl), -1));

    instructions.push_back(new JR_r8_Instruction( 0x20, 2, 0, "JR NZ,r8", new NZ_Condition()));
    instructions.push_back(new JR_r8_Instruction( 0x28, 2, 0, "JR N,r8", new Z_Condition()));
    instructions.push_back(new JR_Instruction( 0x18, 2, 12, "JR r8"));

    instructions.push_back(new SET_IME_Instruction( 0xf3, 1, 4, "DI", 0));
    instructions.push_back(new SET_IME_Instruction( 0xfb, 1, 4, "EI", 1));

    instructions.push_back(new LD_Instruction<byte>( 0xe0, 2, 12, "LDH (a8),A",
                                                        new Memory_a8_Reference<byte>(this, pc),
                                                        new RegisterReference<byte>(a)));
    instructions.push_back(new LD_Instruction<byte>( 0xf0, 2, 12, "LDH A,(a8)",
                                                        new RegisterReference<byte>(a),
                                                        new Memory_a8_Reference<byte>(this, pc)));
    instructions.push_back(new CP_Instruction( 0xfe, 2, 8, "CP d8", new MemoryReference<byte>(this, pc)));

    instructions.push_back(new LD_Instruction<byte>( 0x36, 2, 12, "LD (HL),d8",
                                                        new MemoryReference<byte>(this, hl),
                                                        new MemoryReference<byte>(this, pc)));
    instructions.push_back(new LD_Instruction<byte>( 0xea, 3, 16, "LD (a16),A",
                                                        new Memory_a16_Reference<byte>(this, pc),
                                                        new RegisterReference<byte>(a)));
    instructions.push_back(new LD_Instruction<byte>( 0xfa, 3, 16, "LD A,(a16)",
                                                        new RegisterReference<byte>(a),
                                                        new Memory_a16_Reference<byte>(this, pc)));
    instructions.push_back(new LD_Instruction<word>( 0x31, 3, 12, "LD SP,d16",
                                                        new RegisterReference<word>(sp),
                                                        new MemoryReference<word>(this, pc)));
    instructions.push_back(new LD_Instruction<byte>( 0x2a, 1, 8, "LD A,(HL+)",
                                                        new RegisterReference<byte>(a),
                                                        new MemoryReference<byte>(this, hl, 1)));
    instructions.push_back(new LD_Instruction<byte>( 0x32, 1,    8, "LD (HL-),A",
                                                        new MemoryReference<byte>(this, hl, -1),
                                                        new RegisterReference<byte>(a)));
    instructions.push_back(new LD_Instruction<byte>( 0x77, 1, 8, "LD (HL),A",
                                                        new MemoryReference<byte>(this, hl),
                                                        new RegisterReference<byte>(a)));
    instructions.push_back(new LD_Instruction<byte>( 0x71, 1, 8, "LD (HL),C",
                                                        new MemoryReference<byte>(this, hl),
                                                        new RegisterReference<byte>(c)));
    instructions.push_back(new LD_Instruction<byte>( 0x72, 1, 8, "LD (HL),D",
                                                        new MemoryReference<byte>(this, hl),
                                                        new RegisterReference<byte>(d)));
    instructions.push_back(new LD_Instruction<byte>( 0x73, 1, 8, "LD (HL),E",
                                                        new MemoryReference<byte>(this, hl),
                                                        new RegisterReference<byte>(e)));
    instructions.push_back(new LD_Instruction<byte>( 0x22, 1, 8, "LD (HL+),A",
                                                        new MemoryReference<byte>(this, hl, 1),
                                                        new RegisterReference<byte>(a)));
    instructions.push_back(new LD_Instruction<byte>( 0x3a, 1, 8, "LD A,(HL-)",
                                                        new RegisterReference<byte>(a),
                                                        new MemoryReference<byte>(this, hl, -1)));
    instructions.push_back(new LD_Instruction<byte>( 0x46, 1, 8, "LD B,(HL)",
                                                        new RegisterReference<byte>(b),
                                                        new MemoryReference<byte>(this, hl)));
    instructions.push_back(new LD_Instruction<byte>( 0x4e, 1, 8, "LD C,(HL)",
                                                        new RegisterReference<byte>(c),
                                                        new MemoryReference<byte>(this, hl)));
    instructions.push_back(new LD_Instruction<byte>( 0x56, 1, 8, "LD D,(HL)",
                                                        new RegisterReference<byte>(d),
                                                        new MemoryReference<byte>(this, hl)));
    instructions.push_back(new LD_Instruction<byte>( 0x5e, 1, 8, "LD E,(HL)",
                                                        new RegisterReference<byte>(e),
                                                        new MemoryReference<byte>(this, hl)));
    instructions.push_back(new LD_Instruction<byte>( 0x7e, 1, 8, "LD A,(HL)",
                                                        new RegisterReference<byte>(a),
                                                        new MemoryReference<byte>(this, hl)));
    instructions.push_back(new LD_Instruction<byte>( 0x0a, 1, 8, "LD A,(BC)",
                                                        new RegisterReference<byte>(a),
                                                        new MemoryReference<byte>(this, bc)));

    instructions.push_back(new LD_Instruction<byte>( 0xe2, 1, 8, "LD (C),A",
                                                        new Memory_SingleRegister_Reference<byte>(this, c),
                                                        new RegisterReference<byte>(a)));

    instructions.push_back(new CALL_Instruction( 0xcd, 3, 24, "CALL a16", new MemoryReference<word>(this, pc)));

    instructions.push_back(new LD_Instruction<word>( 0x01, 3, 12, "LD BC,d16",
                                                        new RegisterReference<word>(bc),
                                                        new MemoryReference<word>(this, pc)));
    instructions.push_back(new INC_Instruction<word>( 0x0b, 1, 8, "DEC BC", new RegisterReference<word>(bc), -1));
    instructions.push_back(new INC_Instruction<word>( 0x03, 1, 8, "INC BC", new RegisterReference<word>(bc), 1));
    instructions.push_back(new INC_Instruction<word>( 0x13, 1, 8, "INC DE", new RegisterReference<word>(de), 1));
    instructions.push_back(new INC_Instruction<word>( 0x23, 1, 8, "INC HL", new RegisterReference<word>(hl), 1));

    instructions.push_back(new LD_Instruction<byte>( 0x78, 1, 4, "LD A,B",
                                                        new RegisterReference<byte>(a),
                                                        new RegisterReference<byte>(b)));

    instructions.push_back(new OR_Instruction( 0xb7, 1, 4, "OR A", new RegisterReference<byte>(a)));
    instructions.push_back(new OR_Instruction( 0xb0, 1, 4, "OR B", new RegisterReference<byte>(b)));
    instructions.push_back(new OR_Instruction( 0xb1, 1, 4, "OR C", new RegisterReference<byte>(c)));
    instructions.push_back(new OR_Instruction( 0xf6, 2, 8, "OR d8", new MemoryReference<byte>(this, pc)));

    instructions.push_back(new RET_Instruction( 0xc9, 1, 0, "RET"));

    instructions.push_back(new CPL_Instruction( 0x2f, 1, 4, "CPL"));

    instructions.push_back(new AND_Instruction( 0xe6, 2, 8, "AND d8", new MemoryReference<byte>(this, pc)));
    instructions.push_back(new AND_Instruction( 0xa1, 1, 4, "AND C", new RegisterReference<byte>(c)));
    instructions.push_back(new AND_Instruction( 0xa7, 1, 4, "AND A", new RegisterReference<byte>(a)));

    instructions.push_back(new RLCA_Instruction( 0x07, 1, 4, "RLCA"));

    instructions.push_back(new RET_Instruction( 0xc0, 1, 0, "RET NZ", new NZ_Condition()));
    instructions.push_back(new RET_Instruction( 0xd0, 1, 0, "RET NC", new NC_Condition()));
    instructions.push_back(new RET_Instruction( 0xc8, 1, 0, "RET Z",  new Z_Condition()));
    instructions.push_back(new RET_Instruction( 0xd8, 1, 0, "RET C",  new C_Condition()));

    instructions.push_back(new LD_Instruction<word>( 0x11, 3, 12, "LD DE,d16",
                                                        new RegisterReference<word>(de),
                                                        new MemoryReference<word>(this, pc)));

    instructions.push_back(new LD_Instruction<byte>( 0x12, 1, 8, "LD (DE),A",
                                                        new MemoryReference<byte>(this, de),
                                                        new RegisterReference<byte>(a)));
    instructions.push_back(new LD_Instruction<byte>( 0x1a, 1, 8, "LD A,(DE)",
                                                        new RegisterReference<byte>(a),
                                                        new MemoryReference<byte>(this, de)));

    instructions.push_back(new PUSH_Instruction( 0xc5, 1, 16, "PUSH BC", new RegisterReference<word>(bc)));
    instructions.push_back(new PUSH_Instruction( 0xd5, 1, 16, "PUSH DE", new RegisterReference<word>(de)));
    instructions.push_back(new PUSH_Instruction( 0xe5, 1, 16, "PUSH HL", new RegisterReference<word>(hl)));
    instructions.push_back(new PUSH_Instruction( 0xf5, 1, 16, "PUSH AF", new RegisterReference<word>(af)));

    instructions.push_back(new POP_Instruction(  0xc1, 1, 12, "POP BC", new RegisterReference<word>(bc)));
    instructions.push_back(new POP_Instruction(  0xd1, 1, 12, "POP DE", new RegisterReference<word>(de)));
    instructions.push_back(new POP_Instruction(  0xe1, 1, 12, "POP HL", new RegisterReference<word>(hl)));
    instructions.push_back(new POP_Instruction(  0xf1, 1, 12, "POP AF", new RegisterReference<word>(af)));

    instructions.push_back(new CB_Instruction( 0xcb, 1, 0, "CB", this));

    instructions.push_back(new RST_Instruction( 0xef, 1, 16, "RST 28H", new ValueReference<byte>(0x28)));

    instructions.push_back(new ADD_Instruction<byte>( 0x87, 1, 4, "ADD A,A", new RegisterReference<byte>(a), new RegisterReference<byte>(a)));
    instructions.push_back(new ADD_Instruction<byte>( 0x85, 1, 4, "ADD A,L", new RegisterReference<byte>(a), new RegisterReference<byte>(l)));
    instructions.push_back(new ADD_Instruction<byte>( 0xc6, 2, 8, "ADD A,d8", new RegisterReference<byte>(a), new MemoryReference<byte>(this, pc)));
    instructions.push_back(new ADD_Instruction<word>( 0x09, 1, 8, "ADD HL,BC", new RegisterReference<word>(hl), new RegisterReference<word>(bc)));
    instructions.push_back(new ADD_Instruction<word>( 0x19, 1, 8, "ADD HL,DE", new RegisterReference<word>(hl), new RegisterReference<word>(de)));

    instructions.push_back(new RETI_Instruction( 0xd9, 1, 16, "RETI"));
    instructions.push_back(new SCF_Instruction( 0x37, 1, 4, "SCF"));

    std::cout << instructions.size() << " instructions" << std::endl;
}

CPU::~CPU()
{
    for (Instructions::iterator i = instructions.begin(); i != instructions.end(); ++i)
        delete *i;
}

Instruction * CPU::findInstruction(word address)
{
    byte code = memory->get<byte>(address);
    for (Instructions::iterator i = instructions.begin(); i != instructions.end(); ++i)
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
    Instruction * cmd = findInstruction(pc);
    if (cmd) {
        debugger->handleInstruction(this, pc);
        pc++;
        cmd->run(this);
        cycles += cmd->cycles;
    } else {
        std::cerr << pc << " *** Unknown machine code: " << memory->get<byte>(pc) << std::endl;
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
