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

    template <class T> void set(word address, T b) {};
    template <class T> T get(word address) const  { return 0; };

    byte & getRef(word address) { return rom[address.value()]; };
};

#endif
