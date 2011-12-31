#ifndef MEMORY_H
#define MEMORY_H

#include "word.h"

class Memory
{
private:
    byte *rom;

public:
    Memory(const char *file);
    virtual ~Memory();

    template <class T> inline void set(word address, T b) {};
    template <class T> inline T get(word address) const {};

    inline byte & getRef(word address) { return rom[address.value()]; };
};

template <> inline void Memory::set(word address, byte b) { rom[address.value()] = b; };
template <> inline byte Memory::get(word address) const { return rom[address.value()]; };

template <> inline void Memory::set(word address, word w) {
    rom[address.value()] = w.lo();
    rom[address.value()+1] = w.hi();
};
template <> inline word Memory::get(word address) const {
    return word(rom[address.value()], rom[address.value()+1]);
};

#endif
