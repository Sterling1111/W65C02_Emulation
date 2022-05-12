#ifndef INC_65C02_EMULATION_SYSTEM_H
#define INC_65C02_EMULATION_SYSTEM_H

#include <limits>
#include "W65C02.h"

class System {
public:
    System(sdword ramMin, sdword ramMax, sdword regMin, sdword regMax, sdword romMin, sdword romMax, double Mhz =1);
    void executeProgram(const std::string& programObjFile, uint64_t instructionsToExecute = std::numeric_limits<uint64_t>::max(), bool logging = false, const std::string& outFile = "");
    void loadProgram(const std::string& programObjFile);

    LCD lcd;
    PortBus portBus;
    W65C02 cpu{1};
    W65C22 registers{};
    RAM ram{};
    EEPROM eeprom{};
    Bus bus;
    bool firstReset{};
};

#endif //INC_65C02_EMULATION_SYSTEM_H
