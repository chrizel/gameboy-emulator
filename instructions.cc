#include <iostream>

#include "cb_instructionset.h"
#include "debugger.h"
#include "instructions.h"
#include "instructionset.h"

CB_Instruction::CB_Instruction() : instructionSet(0)
{
}

CB_Instruction::~CB_Instruction()
{
    if (instructionSet)
        delete instructionSet;
}

void CB_Instruction::run() {
    if (!instructionSet) {
        instructionSet = new InstructionSet();
        initialize_cb_instructionset(instructionSet, cpu);
    }

    byte code = cpu->memory->get<byte>(cpu->pc);
    Instruction *instruction = instructionSet->findInstruction(code);
    if (instruction) {
        cpu->pc++;
        instruction->run();
        cpu->cycles += instruction->cycles0;
    } else {
        fprintf(stderr, "%04x *** Unknown CB machine code: %02x\n", cpu->pc.value(), code);
        cpu->debugger->prompt(cpu);
    }
}
