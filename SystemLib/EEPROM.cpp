#include "EEPROM.h"
#include <array>
#include <algorithm>
#include <stdexcept>

EEPROM::EEPROM() { initialize(); }

void EEPROM::initialize() {
    for(dword i{}; i < MAX_MEM; ++i)
        data[i] = 0xea; //nop
}

void EEPROM::loadProgram(const std::string& programObjFile) {
    std::ifstream program(programObjFile, std::ios::binary | std::ios::ate);
    if (!program) throw std::runtime_error("Cannot open ROM: " + programObjFile);
    const auto size = program.tellg();
    if (size <= 0 || size > MAX_MEM)
        throw std::runtime_error("ROM must contain between 1 and 32768 bytes: " + programObjFile);
    // Validate/read completely before changing the current ROM. Padding also
    // prevents a shorter replacement from retaining the previous program.
    std::array<byte, MAX_MEM> replacement;
    replacement.fill(0xea);
    program.seekg(0);
    if (!program.read(reinterpret_cast<char*>(replacement.data()), size))
        throw std::runtime_error("Cannot read ROM: " + programObjFile);
    std::copy(replacement.begin(), replacement.end(), data);
}

byte EEPROM::operator[](word address) const { return data[address]; }