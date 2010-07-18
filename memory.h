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

    template <class T> inline void set(word address, T b) {};
    template <class T> inline T get(word address) const {};

    inline byte & getRef(word address) { return rom[address]; };
};

template <> inline void Memory::set(word address, byte b) { rom[address] = b; };
template <> inline byte Memory::get(word address) const { return rom[address]; };

template <> inline void Memory::set(word address, word w) {
    Register r;
    r.w = w;
    rom[address] = r.b.lo;
    rom[address+1] = r.b.hi;
};
template <> inline word Memory::get(word address) const {
    Register result;
    result.b.lo = rom[address];
    result.b.hi = rom[address+1];
    return result.w;
};

#endif
