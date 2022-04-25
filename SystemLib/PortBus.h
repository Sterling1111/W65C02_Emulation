#ifndef _65C02_SYSTEM_EMULATION_LCDBUS_H
#define _65C02_SYSTEM_EMULATION_LCDBUS_H
#include "Lights.h"
#include "system_types.h"

class PortBus {
public:
    explicit PortBus(Lights& lights);
    Lights& lights;
    void write(byte data);
    byte read() const;
};


#endif //_65C02_SYSTEM_EMULATION_LCDBUS_H
