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

    rom = new byte[size];
    is.read((char*)rom, size);
    is.close();
}

Memory::~Memory()
{
    delete rom;
}
