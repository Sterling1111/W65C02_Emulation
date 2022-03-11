#ifndef _65C02_SYSTEM_EMULATION_EEPROM_H
#define _65C02_SYSTEM_EMULATION_EEPROM_H

#include "system_types.h"
#include <fstream>


class EEPROM {
public:
    EEPROM() {
        initialize();
    }
    explicit EEPROM(const byte rom[1024 * 32]) {
        initialize(rom);
    }
    void initialize(const byte rom[1024 * 32]) {
        for(dword i{}; i < MAX_MEM; ++i)
            data[i] = rom[i];
    }
    void initialize() {
        for(dword i{}; i < MAX_MEM; ++i)
            data[i] = 0xea; //nop
    }
    void loadProgram(const std::string& programObjFile) {
        std::fstream program(programObjFile, std::ios::in | std::ios::binary);
        program.seekg(0, std::ios::end);
        long size = program.tellg();
        program.seekg(0, std::ios::beg);
        program.read(reinterpret_cast<char*>(&data), size);
    }
    byte operator[](word address) const { return data[address]; }
    static constexpr word MAX_MEM = 1024 * 32;

private:
    byte data[MAX_MEM];
};

#endif //_65C02_SYSTEM_EMULATION_EEPROM_H
