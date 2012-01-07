#include <stdio.h>

#include <iostream>
#include <fstream>

#include "memory.h"
#include "debugger.h"

Memory::Memory(const char *file, Debugger *debugger) : debugger(debugger)
{
    std::ifstream is;
    is.open(file, std::ios::binary);

    if (is.fail()) {
        std::cerr << "Cannot open file: " << file << std::endl;
        exit(1);
    }

    is.seekg(0, std::ios::end);
    int size = is.tellg();
    is.seekg(0, std::ios::beg);

    rom = new byte[65536];
    memset(rom, 0, 65536);

    is.read((char*)rom, size);
    is.close();
}

Memory::~Memory()
{
    delete rom;
}

void Memory::dmaTransfer(byte b) {
    byte c = 0x00;
    while (c <= 0x9f) {
        rom[word(c, 0xfe).value()] = rom[word(c, b).value()];
        c++;
    }
}

template <> void Memory::set<byte>(word address, byte b) {
    if (address < 0x8000) {
        //TODO: rom bank switching
        return;
    }
    rom[address.value()] = b; 
    debugger->handleMemoryAccess(this, address, true);

    if (address == 0xff46)
        dmaTransfer(b);
}

template <> byte Memory::get<byte>(word address) {
    debugger->handleMemoryAccess(this, address, false);
    return rom[address.value()];
}

template <> void Memory::set<word>(word address, word w) {
    set<byte>(address.value(), w.lo());
    set<byte>(address.value()+1, w.hi());
}

template <> word Memory::get<word>(word address) {
    byte lo = get<byte>(address.value());
    byte hi = get<byte>(address.value()+1);
    return word(lo, hi);
}
