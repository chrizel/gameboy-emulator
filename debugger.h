#ifndef DEBUG_H
#define DEBUG_H

#include <vector>
#include "word.h"

typedef std::vector<word> Breakpoints;
typedef std::vector<word> Watches;

class CPU;
class Memory;

class Debugger
{
private:
    Breakpoints breakpoints;
    Watches watches;

public:
    bool verboseCPU, verboseMemory, stepMode;

    Debugger();

    void handleInstruction(CPU *cpu, word address);
    void handleMemoryAccess(Memory *memory, word address, bool set);
    void handleInterrupt(int irq, word address);

    void toggleBreakpoint(word address);
    void listBreakpoints();

    void toggleWatch(word address);
    void listWatches();

    void showMemory(CPU *cpu, word address);
    void showStack(CPU *cpu);
    void printInstruction(CPU *cpu, word address);
    void prompt(CPU *cpu);
};

#endif
