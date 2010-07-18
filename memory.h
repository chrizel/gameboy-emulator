#ifndef MEMORY_H
#define MEMORY_H

#include "gameboy.h"

class Memory
{
private:
    char *rom;

public:
    Memory(const char *file);
    virtual ~Memory();

    inline void set(word address, byte b) { rom[address] = b; };
    inline byte get(word address) const { return rom[address]; };
};

#endif
