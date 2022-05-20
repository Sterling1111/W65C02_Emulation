#include "W65C22.h"

W65C22::W65C22() {
    reset();
}

void W65C22::reset() {
    for (int i = 0; i < 16; ++i) {
        registers[i] = 0;
    }
}

void W65C22::writeToRegisters(byte data, word address) {
    registers[address & 0x0F] = data;
    if((address & 0x0F) == 0) {     //IORB
        byte num{};
        for (int i = 0; i < 8; ++i) {
            if(registers[2] & (1 << i)) {   //DDRB
                num |= (data & (1 << i));
            }
        } portBWrite(num);
    } else if((address & 0x0F) == 1) {  //IORA
        byte num{};
        for (int i = 0; i < 8; ++i) {
            if(registers[3] & (1 << i)) {   //DDRA
                num |= (data & (1 << i));
            }
        } portAWrite(num);
    }
}

byte W65C22::readFromRegisters(word address) {
    if((address & 0xF) == 0) //IORB
        return portBRead();
    return 0;
}

void W65C22::portAWrite(byte data) {
    bus->portAWrite(data);
}

void W65C22::portBWrite(byte data) {
    bus->portBWrite(data);
}

/*byte W65C22::portARead() {
    return bus->portARead();
}*/

byte W65C22::portBRead() {
    return bus->portBRead();
}

void W65C22::connectPortBus(PortBus* bus) {
    this->bus = bus;
}
