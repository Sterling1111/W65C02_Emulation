#include "Bus.h"

Bus::Bus(RAM& ram, sdword ramMin, sdword ramMax, W65C22& registers, sdword regMin, sdword regMax,
         EEPROM& eeprom, sdword romMin, sdword romMax) :
        ram{ram}, ramMin{ramMin}, ramMax{ramMax}, registers{registers},
        regMin{regMin}, regMax{regMax}, eeprom{eeprom}, romMin{romMin}, romMax{romMax} {}

void Bus::write(byte data, word address) {
    if(log and outFile.is_open()) {
        outFile << std::setfill('0') << std::setw(4) << std::hex << address << "  " << "W  ";
        outFile << std::setfill('0') << std::setw(2) << std::hex << static_cast<word>(data) << std::endl;
    }
    if(address >= ramMin && address <= ramMax)
        ram[address - ramMin] = data;
    else if(address >= regMin && address <= regMax)
        registers.writeToRegisters(data, address - regMin);
    else if(address >= romMin && address <= romMax)
        return;
    //EEPROM can't write so does nothing
}
byte Bus::read(word address) {
    if(address >= ramMin && address <= ramMax) {
        byte data{ram[address - ramMin]};
        if(log and outFile.is_open()) {
            outFile << std::setfill('0') << std::setw(4) << std::hex << address << "  " << "R  ";
            outFile << std::setfill('0') << std::setw(2) << std::hex << static_cast<word>(data) << std::endl;
        }
        return ram[address - ramMin];
    }
    else if(address >= regMin && address <= regMax) {
        return registers.readFromRegisters(address - regMin);
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
bool Bus::openProgramOutFile(const std::string& progOutFile) {
    outFile.open(progOutFile);
    if(!outFile.is_open()) {
        return false;
    } return true;
}
