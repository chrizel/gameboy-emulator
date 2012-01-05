#ifndef INSTRUCTIONSET_H
#define INSTRUCTIONSET_H

#include <string>
#include <vector>

#include "word.h"

struct Instruction;
class CPU;

typedef std::vector<Instruction*> Instructions;

class InstructionSet
{
private:
    Instructions instructions;

public:
    InstructionSet();
    ~InstructionSet();

    Instruction * findInstruction(byte code);
    void add(Instruction *instruction);
};

#endif
