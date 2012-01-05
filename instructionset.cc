#include "cpu.h"
#include "instructions.h"
#include "instructionset.h"

InstructionSet::InstructionSet()
{
}

void InstructionSet::add(Instruction *instruction)
{
    instructions.push_back(instruction);
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
