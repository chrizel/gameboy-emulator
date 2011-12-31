#ifndef GAMEBOY_H
#define GAMEBOY_H

#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;

union Register {
    word w;
    struct {
        byte lo;
        byte hi;
    } b;
};

word word_from_bytes(byte lo, byte hi);

class Memory;
class CPU;

class GameBoy 
{
public:
    Memory *memory;
    CPU *cpu;

    GameBoy(const char *file);
    virtual ~GameBoy();

    void process();
};

#define GB_DISPLAY_WIDTH 160
#define GB_DISPLAY_HEIGHT 144

#endif
