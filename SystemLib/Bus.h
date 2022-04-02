#ifndef INC_65C02_EMULATION_BUS_H
#define INC_65C02_EMULATION_BUS_H

#include <fstream>
#include <iomanip>
#include "_65C02.h"
#include "RAM.h"
#include "EEPROM.h"
#include "W65C22S.h"

class Bus {
public:
    Bus(_65C02& cpu, RAM& ram, sdword ramMin, sdword ramMax, W65C22S& registers, sdword regMin, sdword regMax,
        EEPROM& eeprom, sdword romMin, sdword romMax) :
        cpu{cpu}, ram{ram}, ramMin{ramMin}, ramMax{ramMax}, registers{registers},
        regMin{regMin}, regMax{regMax}, eeprom{eeprom}, romMin{romMin}, romMax{romMax}
    {
        this->cpu.connectBus(this);
    }
    ~Bus() {
        if(outFile.is_open()) outFile.close();
    }
    void write(byte data, word address) {
        if(log and outFile.is_open()) {
            outFile << std::setfill('0') << std::setw(4) << std::hex << address << "  " << "W  ";
            outFile << std::setfill('0') << std::setw(2) << std::hex << static_cast<word>(data) << std::endl;
        }
        if(address >= ramMin && address <= ramMax)
            ram[address - ramMin] = data;
        else if(address >= regMin && address <= regMax);
            //65C22 write
        else if(address >= romMin && address <= romMax);
            //EEPROM can't write so does nothing
    }
    byte read(word address, bool readOnly = false) {
        if(address >= ramMin && address <= ramMax) {
            byte data{ram[address - ramMin]};
            if(log and outFile.is_open()) {
                outFile << std::setfill('0') << std::setw(4) << std::hex << address << "  " << "R  ";
                outFile << std::setfill('0') << std::setw(2) << std::hex << static_cast<word>(data) << std::endl;
            }
            return ram[address - ramMin];
        }
        else if(address >= regMin && address <= regMax) {

        }
        else if(address >= romMin && address <= romMax) {
            byte data{eeprom[address - romMin]};
            if(log) {
                outFile << std::setfill('0') << std::setw(4) << std::hex << address << "  " << "R  ";
                outFile << std::setfill('0') << std::setw(2) << std::hex << static_cast<word>(data) << std::endl;
            }
            return data;
        }
        return 0x00;
    }
    bool openProgramOutFile(const std::string& progOutFile) {
        outFile.open(progOutFile);
        if(!outFile.is_open()) {
            return false;
        } return true;
    }

public:
    _65C02& cpu;
    W65C22S& registers;
    RAM& ram;
    EEPROM& eeprom;
    sdword ramMin, ramMax, regMin, regMax, romMin, romMax;
    bool log{false};
    std::ofstream outFile;
};

#endif //INC_65C02_EMULATION_BUS_H
