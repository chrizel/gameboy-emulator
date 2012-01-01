#ifndef MEMORY_H
#define MEMORY_H

#include "word.h"

class Debugger;

class Memory
{
private:
    byte *rom;
    Debugger *debugger;

    void dmaTransfer(byte b);

public:
    Memory(const char *file, Debugger *debugger);
    virtual ~Memory();

    template <class T> void set(word address, T b) {};
    template <class T> T get(word address) { return 0; };

    byte & getRef(word address) { return rom[address.value()]; };
};

#endif
