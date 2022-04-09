#ifndef INC_65C02_EMULATION_SYSTEM_H
#define INC_65C02_EMULATION_SYSTEM_H

#include "Bus.h"
#include <limits>

class System {
public:
    explicit System(double Mhz = 1) : cpu{Mhz}{};
    explicit System(sdword ramMin, sdword ramMax, sdword regMin, sdword regMax,
                    sdword romMin, sdword romMax, double Mhz = 1) :
                    cpu{Mhz}, ram{}, ramMin{ramMin}, ramMax{ramMax}, registers{}, regMin{regMin},
                    regMax{regMax}, eeprom{}, romMin{romMin}, romMax{romMax},
                    bus{cpu, ram, ramMin, ramMax, registers, regMin, regMax, eeprom, romMin, romMax}{}

    void executeProgram(const std::string& programObjFile, uint64_t instructionsToExecute = std::numeric_limits<uint64_t>::max(), bool logging = false, const std::string& outFile = "") {
        eeprom.loadProgram(programObjFile);
        cpu.reset(eeprom[0xFFFC - 0x8000] | eeprom[0xFFFD - 0x8000] << 8);
        bus.log = logging;
        if(!bus.openProgramOutFile(outFile)) { bus.log = false; }
        cpu.execute(instructionsToExecute);
    }

    sdword ramMin{0x0000}, ramMax{0x3FFF}, regMin{0x6000}, regMax{0x7FFF}, romMin{0x8000}, romMax{0xFFFF};
    W65C02 cpu;
    RAM ram{};
    W65C22S registers{};
    EEPROM eeprom{};
    Bus bus{cpu, ram, ramMin, ramMax, registers, regMin, regMax, eeprom, romMin, romMax};
};
#endif //INC_65C02_EMULATION_SYSTEM_H
