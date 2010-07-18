#ifndef CPU_H
#define CPU_H

#include "gameboy.h"

typedef void (*CommandHandler)(GameBoy *gb);

typedef struct {
    byte code;
    byte length;
    byte cycles;
    const char *mnemonic;
    CommandHandler handler;
} Command;

void gbCPUInit(GameBoy *gb);
void gbCPUStep(GameBoy *gb);
Command * gbCPUFindCommand(GameBoy *gb, word address);

#endif
