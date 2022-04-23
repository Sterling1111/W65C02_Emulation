#include "EEPROM.h"

EEPROM::EEPROM() { initialize(); }

void EEPROM::initialize() {
    for(dword i{}; i < MAX_MEM; ++i)
        data[i] = 0xea; //nop
}

void EEPROM::loadProgram(const std::string& programObjFile) {
    std::fstream program(programObjFile, std::ios::in | std::ios::binary);
    program.seekg(0, std::ios::end);
    auto size = program.tellg();
    program.seekg(0, std::ios::beg);
    program.read(reinterpret_cast<char*>(&data), size);
}

byte EEPROM::operator[](word address) const { return data[address]; }