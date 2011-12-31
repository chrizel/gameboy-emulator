#ifndef GAMEBOY_H
#define GAMEBOY_H

class Memory;
class CPU;
class Debugger;

class GameBoy 
{
public:
    Memory *memory;
    CPU *cpu;
    Debugger *debugger;

    GameBoy(const char *file);
    virtual ~GameBoy();

    void process();
};

#define GB_DISPLAY_WIDTH 160
#define GB_DISPLAY_HEIGHT 144

#endif
