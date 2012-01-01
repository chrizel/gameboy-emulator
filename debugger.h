#ifndef DEBUG_H
#define DEBUG_H

#include <vector>
#include "word.h"

typedef std::vector<word> Breakpoints;

class CPU;

class Debugger
{
private:
    Breakpoints breakpoints;

public:
    bool verboseCPU, stepMode;

    Debugger();

    void handleInstruction(CPU *cpu, word address);
    void printInstruction(CPU *cpu, word address);
    void prompt(CPU *cpu);
    void toggleBreakpoint(word address);
    void listBreakpoints();
    void showMemory(CPU *cpu, word address);
    void showStack(CPU *cpu);
};

#endif
