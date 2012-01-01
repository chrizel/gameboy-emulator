#include <stdio.h>

#include <iostream>
#include <fstream>

#include "memory.h"

Memory::Memory(const char *file)
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

template <> void Memory::set<byte>(word address, byte b) {
    /*
    if (address.value() >= 0x8000) {
        printf("set %04x -> %02x\n", address.value(), b);
    }
    */
    rom[address.value()] = b; 
}

template <> byte Memory::get<byte>(word address) const {
    /*
    if (address.value() >= 0x8000) {
        printf("get %04x -> %02x\n", address.value(), rom[address.value()]);
    }
    */
    return rom[address.value()];
}

template <> void Memory::set<word>(word address, word w) {
    set<byte>(address.value(), w.lo());
    set<byte>(address.value()+1, w.hi());
}

template <> word Memory::get<word>(word address) const {
    byte lo = get<byte>(address.value());
    byte hi = get<byte>(address.value()+1);
    return word(lo, hi);
}
