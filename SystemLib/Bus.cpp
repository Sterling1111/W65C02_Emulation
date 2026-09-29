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
    if(serial && address>=0x5000 && address<=0x5003)
        serial->write(data,address-0x5000);
    else if(address >= ramMin && address <= ramMax)
        ram[address - ramMin] = data;
    else if(address >= regMin && address <= regMax)
        registers.writeToRegisters(data, address - regMin);
    else if(address >= romMin && address <= romMax)
        return;
    //EEPROM can't write so does nothing
}
byte Bus::read(word address) {
    if(serial && address>=0x5000 && address<=0x5003) {
        const byte data=serial->read(address-0x5000);
        if(log && outFile.is_open()) {
            outFile << std::setfill('0') << std::setw(4) << std::hex << address << "  R  "
                    << std::setw(2) << static_cast<word>(data) << std::endl;
        }
        return data;
    }
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

void Bus::tick(double cpuHz) {
    if (regMin >= 0 && regMax >= regMin) registers.tick();
    if(serial) {
        const auto pins=registers.pins();
        // Ben Eater BIOS drives VIA PA0 high to stop the remote sender.
        serial->tick(cpuHz,!(pins.paOutputMask & pins.pa & 1));
    }
}

bool Bus::irqAsserted() const {
    return (serial && serial->irqAsserted()) ||
           (regMin >= 0 && regMax >= regMin && registers.irqAsserted());
}
