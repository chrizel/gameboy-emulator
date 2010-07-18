#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gameboy.h"
#include "cpu.h"
#include "memory.h"

GameBoy::GameBoy(const char *file)
{
    memory = new Memory(file);
    cpu = new CPU(memory);
}

GameBoy::~GameBoy()
{
    delete cpu;
    delete memory;
}

void GameBoy::process()
{
    cpu->cycles = 0;
    while (cpu->cycles < 100) {
        cpu->step();

        cpu->ly.set(cpu->ly.get() + 1);
        if (cpu->ly.get() > 153) {
            cpu->ly.set(0);
        } else if (cpu->ly.get() == 144) {
            /* vblank interrupt */
        }
    }
}
