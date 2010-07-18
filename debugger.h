#ifndef DEBUG_H
#define DEBUG_H

class CPU;

void gbDebugPrintInstruction(CPU *cpu, word address);
void gbDebugPrompt(CPU *cpu);

#endif
