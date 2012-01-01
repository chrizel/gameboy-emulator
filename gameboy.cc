#include <iostream>

#include "gameboy.h"
#include "cpu.h"
#include "memory.h"
#include "debugger.h"

GameBoy::GameBoy(const char *file)
{
    debugger = new Debugger();
    memory = new Memory(file, debugger);
    cpu = new CPU(memory, debugger);
}

GameBoy::~GameBoy()
{
    delete debugger;
    delete cpu;
    delete memory;
}

void GameBoy::process()
{
    cpu->cycles = 0;
    while (cpu->cycles < 100) {
        cpu->step();

        cpu->ly += 1;
        if (cpu->ly > 153) {
            cpu->ly = 0;
        } else if (cpu->ly == 144) {
            /* vblank interrupt */
        }
    }
}
