#ifndef _65C02_SYSTEM_EMULATION_W65C22_H
#define _65C02_SYSTEM_EMULATION_W65C22_H

#include "system_types.h"
#include "PortBus.h"

class W65C22 {
public:
    W65C22();
    byte readFromRegisters(word address);
    void writeToRegisters(byte data, word address);
    void portAWrite(byte data);
    void portBWrite(byte data);
    //byte portARead();
    byte portBRead();
    void reset();
    void connectPortBus(PortBus* bus);
private:

    byte registers[16];
    PortBus* bus{nullptr};
};

#endif //_65C02_SYSTEM_EMULATION_W65C22_H
