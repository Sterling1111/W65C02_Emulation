#ifndef _65C02_SYSTEM_EMULATION_W65C22_H
#define _65C02_SYSTEM_EMULATION_W65C22_H

#include "system_types.h"
#include "PortBus.h"

class W65C22 {
public:
    W65C22();
    byte readByteFromRegisters(word address);
    void writeByteToRegisters(byte data, word address);
    void write(byte data);
    void reset();
    void connectPortBus(PortBus* bus);
private:

    byte registers[16];
    PortBus* bus{nullptr};
};

#endif //_65C02_SYSTEM_EMULATION_W65C22_H
