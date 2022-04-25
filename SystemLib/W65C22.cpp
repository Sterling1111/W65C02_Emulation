#include "W65C22.h"

W65C22::W65C22() {
    reset();
}

void W65C22::reset() {
    for (int i = 0; i < 16; ++i) {
        registers[i] = 0;
    }
}

byte W65C22::readByteFromRegisters(word address) {
    return registers[address & 0x0F];
}

void W65C22::write(byte data) {
    bus->write(data);
}

void W65C22::writeByteToRegisters(byte data, word address) {
    registers[address & 0x0F] = data;
    if((address & 0x0F) == 0) {
        byte num{};
        for (int i = 0; i < 8; ++i) {
            if(registers[2] & (1 << i)) {
                num |= (data & (1 << i));
            }
        }
        write(data);
    }
}

void W65C22::connectPortBus(PortBus* bus) {
    this->bus = bus;
}
