#ifndef MEMORY_H
#define MEMORY_H

#include "gameboy.h"

class Memory
{
private:
    byte *rom;

public:
    Memory(const char *file);
    virtual ~Memory();

    inline void set(word address, byte b) { rom[address] = b; };
    inline byte get(word address) const { return rom[address]; };

    inline byte & getRef(word address) { return rom[address]; };
};

#endif
