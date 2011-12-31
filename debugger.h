#ifndef DEBUG_H
#define DEBUG_H

#include <vector>
#include "word.h"

typedef std::vector<word> Breakpoints;

class CPU;

class Debugger
{
private:
    bool verboseCPU, stepMode;
    Breakpoints breakpoints;

public:
    Debugger();

    void handleInstruction(CPU *cpu, word address);
    void printInstruction(CPU *cpu, word address);
    void prompt(CPU *cpu);
    void toggleBreakpoint(word address);
    void listBreakpoints();
};

#endif
