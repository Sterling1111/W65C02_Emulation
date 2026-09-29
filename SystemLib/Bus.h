#ifndef INC_65C02_EMULATION_BUS_H
#define INC_65C02_EMULATION_BUS_H

#include <fstream>
#include <iomanip>
#include "RAM.h"
#include "EEPROM.h"
#include "W65C22.h"
#include "W65C51.h"

class Bus {
public:
    Bus(RAM& ram, sdword ramMin, sdword ramMax, W65C22& registers, sdword regMin, sdword regMax,
        EEPROM& eeprom, sdword romMin, sdword romMax);
    void write(byte data, word address);
    byte read(word address);
    void tick(double cpuHz=1000000);
    void connectSerial(W65C51& chip) { serial=&chip; }
    bool hasSerial() const { return serial!=nullptr; }
    bool irqAsserted() const;
    bool openProgramOutFile(const std::string& progOutFile);

public:
    W65C22& registers;
    RAM& ram;
    EEPROM& eeprom;
    sdword ramMin, ramMax, regMin, regMax, romMin, romMax;
    W65C51* serial{};
    bool log{false};
    std::ofstream outFile;
};

#endif //INC_65C02_EMULATION_BUS_H
