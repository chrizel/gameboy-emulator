#include <cstring>

#include "cpu.h"
#include "instructions.h"
#include "instructionset.h"

InstructionSet::InstructionSet()
{
    memset(instructions, 0, 256);
}

void InstructionSet::add(Instruction *instruction)
{
    instructions[instruction->code] = instruction;
}

InstructionSet::~InstructionSet()
{
    for (int i = 0; i < 256; ++i)
        if (instructions[i])
            delete instructions[i];
}

Instruction * InstructionSet::findInstruction(byte code)
{
    return instructions[code];
}
