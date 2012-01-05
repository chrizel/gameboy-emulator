#include "cpu.h"
#include "instructions.h"
#include "instructionset.h"

InstructionSet::InstructionSet(CPU *cpu)
{
    instructions.push_back(new NOP_Instruction(  0x00, 1,    4, "NOP"));

    instructions.push_back(new JP_Instruction(   0xc3, 3,   16, "JP a16",  new MemoryReference<word>(cpu, cpu->pc)));
    instructions.push_back(new JP_Instruction(   0xe9, 1,    4, "JP (HL)", new RegisterReference<word>(cpu->hl)));

    instructions.push_back(new JP_a16_Instruction( 0xc2, 3, 0, "JP NZ,a16", new NZ_Condition()));
    instructions.push_back(new JP_a16_Instruction( 0xca, 3, 0, "JP Z,a16", new Z_Condition()));

    instructions.push_back(new XOR_Instruction(  0xaf, 1,    4, "XOR A", new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new XOR_Instruction(  0xa9, 1,    4, "XOR C", new RegisterReference<byte>(cpu->c)));
    instructions.push_back(new LD_Instruction<word>( 0x21, 3,   12, "LD HL,d16",
                                                        new RegisterReference<word>(cpu->hl),
                                                        new MemoryReference<word>(cpu, cpu->pc)));

    instructions.push_back(new LD_Instruction<byte>( 0x3e, 2, 8, "LD A,d8",
                                                        new RegisterReference<byte>(cpu->a),
                                                        new MemoryReference<byte>(cpu, cpu->pc)));
    instructions.push_back(new LD_Instruction<byte>( 0x1e, 2,    8, "LD E,d8",
                                                        new RegisterReference<byte>(cpu->e),
                                                        new MemoryReference<byte>(cpu, cpu->pc)));
    instructions.push_back(new LD_Instruction<byte>( 0x0e, 2,    8, "LD C,d8",
                                                        new RegisterReference<byte>(cpu->c),
                                                        new MemoryReference<byte>(cpu, cpu->pc)));
    instructions.push_back(new LD_Instruction<byte>( 0x06, 2,    8, "LD B,d8",
                                                        new RegisterReference<byte>(cpu->b),
                                                        new MemoryReference<byte>(cpu, cpu->pc)));
    instructions.push_back(new LD_Instruction<byte>( 0x16, 2,    8, "LD D,d8",
                                                        new RegisterReference<byte>(cpu->d),
                                                        new MemoryReference<byte>(cpu, cpu->pc)));
    instructions.push_back(new LD_Instruction<byte>( 0x26, 2,    8, "LD H,d8",
                                                        new RegisterReference<byte>(cpu->h),
                                                        new MemoryReference<byte>(cpu, cpu->pc)));

    instructions.push_back(new LD_Instruction<byte>( 0x47, 1, 4, "LD B,A", new RegisterReference<byte>(cpu->b), new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new LD_Instruction<byte>( 0x40, 1, 4, "LD B,B", new RegisterReference<byte>(cpu->b), new RegisterReference<byte>(cpu->b)));
    instructions.push_back(new LD_Instruction<byte>( 0x4f, 1, 4, "LD C,A", new RegisterReference<byte>(cpu->c), new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new LD_Instruction<byte>( 0x79, 1, 4, "LD A,C", new RegisterReference<byte>(cpu->a), new RegisterReference<byte>(cpu->c)));
    instructions.push_back(new LD_Instruction<byte>( 0x7a, 1, 4, "LD A,D", new RegisterReference<byte>(cpu->a), new RegisterReference<byte>(cpu->d)));
    instructions.push_back(new LD_Instruction<byte>( 0x7b, 1, 4, "LD A,E", new RegisterReference<byte>(cpu->a), new RegisterReference<byte>(cpu->e)));
    instructions.push_back(new LD_Instruction<byte>( 0x57, 1, 4, "LD D,A", new RegisterReference<byte>(cpu->d), new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new LD_Instruction<byte>( 0x54, 1, 4, "LD D,H", new RegisterReference<byte>(cpu->d), new RegisterReference<byte>(cpu->h)));
    instructions.push_back(new LD_Instruction<byte>( 0x5f, 1, 4, "LD E,A", new RegisterReference<byte>(cpu->e), new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new LD_Instruction<byte>( 0x5d, 1, 4, "LD E,L", new RegisterReference<byte>(cpu->e), new RegisterReference<byte>(cpu->l)));
    instructions.push_back(new LD_Instruction<byte>( 0x67, 1, 4, "LD H,A", new RegisterReference<byte>(cpu->h), new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new LD_Instruction<byte>( 0x60, 1, 4, "LD H,B", new RegisterReference<byte>(cpu->h), new RegisterReference<byte>(cpu->b)));
    instructions.push_back(new LD_Instruction<byte>( 0x62, 1, 4, "LD H,D", new RegisterReference<byte>(cpu->h), new RegisterReference<byte>(cpu->d)));
    instructions.push_back(new LD_Instruction<byte>( 0x69, 1, 4, "LD L,C", new RegisterReference<byte>(cpu->l), new RegisterReference<byte>(cpu->c)));
    instructions.push_back(new LD_Instruction<byte>( 0x6f, 1, 4, "LD L,A", new RegisterReference<byte>(cpu->l), new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new LD_Instruction<byte>( 0x6b, 1, 4, "LD L,E", new RegisterReference<byte>(cpu->l), new RegisterReference<byte>(cpu->e)));
    instructions.push_back(new LD_Instruction<byte>( 0x7c, 1, 4, "LD A,H", new RegisterReference<byte>(cpu->a), new RegisterReference<byte>(cpu->h)));
    instructions.push_back(new LD_Instruction<byte>( 0x7d, 1, 4, "LD A,L", new RegisterReference<byte>(cpu->a), new RegisterReference<byte>(cpu->l)));

    instructions.push_back(new INC_Instruction<byte>( 0x3c, 1, 4, "INC A", new RegisterReference<byte>(cpu->a), 1));
    instructions.push_back(new INC_Instruction<byte>( 0x04, 1, 4, "INC B", new RegisterReference<byte>(cpu->b), 1));
    instructions.push_back(new INC_Instruction<byte>( 0x0c, 1, 4, "INC C", new RegisterReference<byte>(cpu->c), 1));
    instructions.push_back(new INC_Instruction<byte>( 0x1c, 1, 4, "INC E", new RegisterReference<byte>(cpu->e), 1));
    instructions.push_back(new INC_Instruction<byte>( 0x2c, 1, 4, "INC L", new RegisterReference<byte>(cpu->l), 1));
    instructions.push_back(new INC_Instruction<byte>( 0x24, 1, 4, "INC H", new RegisterReference<byte>(cpu->h), 1));
    instructions.push_back(new INC_Instruction<byte>( 0x34, 1, 12, "INC (HL)", new MemoryReference<byte>(cpu, cpu->hl), 1));

    instructions.push_back(new INC_Instruction<byte>( 0x3d, 1, 4, "DEC A", new RegisterReference<byte>(cpu->a), -1));
    instructions.push_back(new INC_Instruction<byte>( 0x05, 1, 4, "DEC B", new RegisterReference<byte>(cpu->b), -1));
    instructions.push_back(new INC_Instruction<byte>( 0x0d, 1, 4, "DEC C", new RegisterReference<byte>(cpu->c), -1));
    instructions.push_back(new INC_Instruction<byte>( 0x2d, 1, 4, "DEC L", new RegisterReference<byte>(cpu->l), -1));
    instructions.push_back(new INC_Instruction<byte>( 0x35, 1, 12, "DEC (HL)", new MemoryReference<byte>(cpu, cpu->hl), -1));

    instructions.push_back(new JR_r8_Instruction( 0x20, 2, 0, "JR NZ,r8", new NZ_Condition()));
    instructions.push_back(new JR_r8_Instruction( 0x28, 2, 0, "JR N,r8", new Z_Condition()));
    instructions.push_back(new JR_Instruction( 0x18, 2, 12, "JR r8"));

    instructions.push_back(new SET_IME_Instruction( 0xf3, 1, 4, "DI", 0));
    instructions.push_back(new SET_IME_Instruction( 0xfb, 1, 4, "EI", 1));

    instructions.push_back(new LD_Instruction<byte>( 0xe0, 2, 12, "LDH (a8),A",
                                                        new Memory_a8_Reference<byte>(cpu, cpu->pc),
                                                        new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new LD_Instruction<byte>( 0xf0, 2, 12, "LDH A,(a8)",
                                                        new RegisterReference<byte>(cpu->a),
                                                        new Memory_a8_Reference<byte>(cpu, cpu->pc)));
    instructions.push_back(new CP_Instruction( 0xfe, 2, 8, "CP d8", new MemoryReference<byte>(cpu, cpu->pc)));

    instructions.push_back(new LD_Instruction<byte>( 0x36, 2, 12, "LD (HL),d8",
                                                        new MemoryReference<byte>(cpu, cpu->hl),
                                                        new MemoryReference<byte>(cpu, cpu->pc)));
    instructions.push_back(new LD_Instruction<byte>( 0xea, 3, 16, "LD (a16),A",
                                                        new Memory_a16_Reference<byte>(cpu, cpu->pc),
                                                        new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new LD_Instruction<byte>( 0xfa, 3, 16, "LD A,(a16)",
                                                        new RegisterReference<byte>(cpu->a),
                                                        new Memory_a16_Reference<byte>(cpu, cpu->pc)));
    instructions.push_back(new LD_Instruction<word>( 0x31, 3, 12, "LD SP,d16",
                                                        new RegisterReference<word>(cpu->sp),
                                                        new MemoryReference<word>(cpu, cpu->pc)));
    instructions.push_back(new LD_Instruction<byte>( 0x2a, 1, 8, "LD A,(HL+)",
                                                        new RegisterReference<byte>(cpu->a),
                                                        new MemoryReference<byte>(cpu, cpu->hl, 1)));
    instructions.push_back(new LD_Instruction<byte>( 0x32, 1,    8, "LD (HL-),A",
                                                        new MemoryReference<byte>(cpu, cpu->hl, -1),
                                                        new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new LD_Instruction<byte>( 0x77, 1, 8, "LD (HL),A",
                                                        new MemoryReference<byte>(cpu, cpu->hl),
                                                        new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new LD_Instruction<byte>( 0x71, 1, 8, "LD (HL),C",
                                                        new MemoryReference<byte>(cpu, cpu->hl),
                                                        new RegisterReference<byte>(cpu->c)));
    instructions.push_back(new LD_Instruction<byte>( 0x72, 1, 8, "LD (HL),D",
                                                        new MemoryReference<byte>(cpu, cpu->hl),
                                                        new RegisterReference<byte>(cpu->d)));
    instructions.push_back(new LD_Instruction<byte>( 0x73, 1, 8, "LD (HL),E",
                                                        new MemoryReference<byte>(cpu, cpu->hl),
                                                        new RegisterReference<byte>(cpu->e)));
    instructions.push_back(new LD_Instruction<byte>( 0x22, 1, 8, "LD (HL+),A",
                                                        new MemoryReference<byte>(cpu, cpu->hl, 1),
                                                        new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new LD_Instruction<byte>( 0x3a, 1, 8, "LD A,(HL-)",
                                                        new RegisterReference<byte>(cpu->a),
                                                        new MemoryReference<byte>(cpu, cpu->hl, -1)));
    instructions.push_back(new LD_Instruction<byte>( 0x46, 1, 8, "LD B,(HL)",
                                                        new RegisterReference<byte>(cpu->b),
                                                        new MemoryReference<byte>(cpu, cpu->hl)));
    instructions.push_back(new LD_Instruction<byte>( 0x4e, 1, 8, "LD C,(HL)",
                                                        new RegisterReference<byte>(cpu->c),
                                                        new MemoryReference<byte>(cpu, cpu->hl)));
    instructions.push_back(new LD_Instruction<byte>( 0x56, 1, 8, "LD D,(HL)",
                                                        new RegisterReference<byte>(cpu->d),
                                                        new MemoryReference<byte>(cpu, cpu->hl)));
    instructions.push_back(new LD_Instruction<byte>( 0x5e, 1, 8, "LD E,(HL)",
                                                        new RegisterReference<byte>(cpu->e),
                                                        new MemoryReference<byte>(cpu, cpu->hl)));
    instructions.push_back(new LD_Instruction<byte>( 0x7e, 1, 8, "LD A,(HL)",
                                                        new RegisterReference<byte>(cpu->a),
                                                        new MemoryReference<byte>(cpu, cpu->hl)));
    instructions.push_back(new LD_Instruction<byte>( 0x0a, 1, 8, "LD A,(BC)",
                                                        new RegisterReference<byte>(cpu->a),
                                                        new MemoryReference<byte>(cpu, cpu->bc)));

    instructions.push_back(new LD_Instruction<byte>( 0xe2, 1, 8, "LD (C),A",
                                                        new Memory_SingleRegister_Reference<byte>(cpu, cpu->c),
                                                        new RegisterReference<byte>(cpu->a)));

    instructions.push_back(new CALL_Instruction( 0xcd, 3, 24, "CALL a16", new MemoryReference<word>(cpu, cpu->pc)));

    instructions.push_back(new LD_Instruction<word>( 0x01, 3, 12, "LD BC,d16",
                                                        new RegisterReference<word>(cpu->bc),
                                                        new MemoryReference<word>(cpu, cpu->pc)));
    instructions.push_back(new INC_Instruction<word>( 0x0b, 1, 8, "DEC BC", new RegisterReference<word>(cpu->bc), -1));
    instructions.push_back(new INC_Instruction<word>( 0x03, 1, 8, "INC BC", new RegisterReference<word>(cpu->bc), 1));
    instructions.push_back(new INC_Instruction<word>( 0x13, 1, 8, "INC DE", new RegisterReference<word>(cpu->de), 1));
    instructions.push_back(new INC_Instruction<word>( 0x23, 1, 8, "INC HL", new RegisterReference<word>(cpu->hl), 1));

    instructions.push_back(new LD_Instruction<byte>( 0x78, 1, 4, "LD A,B",
                                                        new RegisterReference<byte>(cpu->a),
                                                        new RegisterReference<byte>(cpu->b)));

    instructions.push_back(new OR_Instruction( 0xb7, 1, 4, "OR A", new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new OR_Instruction( 0xb0, 1, 4, "OR B", new RegisterReference<byte>(cpu->b)));
    instructions.push_back(new OR_Instruction( 0xb1, 1, 4, "OR C", new RegisterReference<byte>(cpu->c)));
    instructions.push_back(new OR_Instruction( 0xf6, 2, 8, "OR d8", new MemoryReference<byte>(cpu, cpu->pc)));

    instructions.push_back(new RET_Instruction( 0xc9, 1, 0, "RET"));

    instructions.push_back(new CPL_Instruction( 0x2f, 1, 4, "CPL"));

    instructions.push_back(new AND_Instruction( 0xe6, 2, 8, "AND d8", new MemoryReference<byte>(cpu, cpu->pc)));
    instructions.push_back(new AND_Instruction( 0xa1, 1, 4, "AND C", new RegisterReference<byte>(cpu->c)));
    instructions.push_back(new AND_Instruction( 0xa7, 1, 4, "AND A", new RegisterReference<byte>(cpu->a)));

    instructions.push_back(new RLCA_Instruction( 0x07, 1, 4, "RLCA"));

    instructions.push_back(new RET_Instruction( 0xc0, 1, 0, "RET NZ", new NZ_Condition()));
    instructions.push_back(new RET_Instruction( 0xd0, 1, 0, "RET NC", new NC_Condition()));
    instructions.push_back(new RET_Instruction( 0xc8, 1, 0, "RET Z",  new Z_Condition()));
    instructions.push_back(new RET_Instruction( 0xd8, 1, 0, "RET C",  new C_Condition()));

    instructions.push_back(new LD_Instruction<word>( 0x11, 3, 12, "LD DE,d16",
                                                        new RegisterReference<word>(cpu->de),
                                                        new MemoryReference<word>(cpu, cpu->pc)));

    instructions.push_back(new LD_Instruction<byte>( 0x12, 1, 8, "LD (DE),A",
                                                        new MemoryReference<byte>(cpu, cpu->de),
                                                        new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new LD_Instruction<byte>( 0x1a, 1, 8, "LD A,(DE)",
                                                        new RegisterReference<byte>(cpu->a),
                                                        new MemoryReference<byte>(cpu, cpu->de)));

    instructions.push_back(new PUSH_Instruction( 0xc5, 1, 16, "PUSH BC", new RegisterReference<word>(cpu->bc)));
    instructions.push_back(new PUSH_Instruction( 0xd5, 1, 16, "PUSH DE", new RegisterReference<word>(cpu->de)));
    instructions.push_back(new PUSH_Instruction( 0xe5, 1, 16, "PUSH HL", new RegisterReference<word>(cpu->hl)));
    instructions.push_back(new PUSH_Instruction( 0xf5, 1, 16, "PUSH AF", new RegisterReference<word>(cpu->af)));

    instructions.push_back(new POP_Instruction(  0xc1, 1, 12, "POP BC", new RegisterReference<word>(cpu->bc)));
    instructions.push_back(new POP_Instruction(  0xd1, 1, 12, "POP DE", new RegisterReference<word>(cpu->de)));
    instructions.push_back(new POP_Instruction(  0xe1, 1, 12, "POP HL", new RegisterReference<word>(cpu->hl)));
    instructions.push_back(new POP_Instruction(  0xf1, 1, 12, "POP AF", new RegisterReference<word>(cpu->af)));

    instructions.push_back(new CB_Instruction( 0xcb, 1, 0, "CB", cpu));

    instructions.push_back(new RST_Instruction( 0xef, 1, 16, "RST 28H", new ValueReference<byte>(0x28)));

    instructions.push_back(new ADD_Instruction<byte>( 0x87, 1, 4, "ADD A,A", new RegisterReference<byte>(cpu->a), new RegisterReference<byte>(cpu->a)));
    instructions.push_back(new ADD_Instruction<byte>( 0x85, 1, 4, "ADD A,L", new RegisterReference<byte>(cpu->a), new RegisterReference<byte>(cpu->l)));
    instructions.push_back(new ADD_Instruction<byte>( 0xc6, 2, 8, "ADD A,d8", new RegisterReference<byte>(cpu->a), new MemoryReference<byte>(cpu, cpu->pc)));
    instructions.push_back(new ADD_Instruction<word>( 0x09, 1, 8, "ADD HL,BC", new RegisterReference<word>(cpu->hl), new RegisterReference<word>(cpu->bc)));
    instructions.push_back(new ADD_Instruction<word>( 0x19, 1, 8, "ADD HL,DE", new RegisterReference<word>(cpu->hl), new RegisterReference<word>(cpu->de)));

    instructions.push_back(new RETI_Instruction( 0xd9, 1, 16, "RETI"));
    instructions.push_back(new SCF_Instruction( 0x37, 1, 4, "SCF"));
}

InstructionSet::~InstructionSet()
{
    for (Instructions::iterator i = instructions.begin(); i != instructions.end(); ++i)
        delete *i;
}

Instruction * InstructionSet::findInstruction(byte code)
{
    for (Instructions::iterator i = instructions.begin(); i != instructions.end(); ++i)
        if ((*i)->code == code)
            return *i;
    return 0;
}
