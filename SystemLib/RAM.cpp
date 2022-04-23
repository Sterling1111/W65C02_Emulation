#include "RAM.h"

RAM::RAM() { initialize(); }

void RAM::initialize() {
    for(dword i{}; i < MAX_MEM; ++i)
        data[i] = 0xea;
}

byte RAM::operator[](word address) const { return data[address]; }

byte& RAM::operator[](word address) { return data[address]; }
