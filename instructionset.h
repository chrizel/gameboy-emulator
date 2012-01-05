#ifndef INSTRUCTIONSET_H
#define INSTRUCTIONSET_H

#include "word.h"

struct Instruction;
class CPU;

class InstructionSet
{
private:
    Instruction *instructions[256];

public:
    InstructionSet();
    ~InstructionSet();

    Instruction * findInstruction(byte code);
    void add(Instruction *instruction);
};

#endif
