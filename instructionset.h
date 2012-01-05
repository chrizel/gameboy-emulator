#ifndef INSTRUCTIONSET_H
#define INSTRUCTIONSET_H

#include <string>
#include <vector>

#include "word.h"

class Instruction;
class CPU;

typedef std::vector<Instruction*> Instructions;

class InstructionSet
{
private:
    Instructions instructions;

public:
    InstructionSet(CPU *cpu);
    ~InstructionSet();

    Instruction * findInstruction(byte code);
};

#endif
