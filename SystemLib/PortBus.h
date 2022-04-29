#ifndef _65C02_SYSTEM_EMULATION_LCDBUS_H
#define _65C02_SYSTEM_EMULATION_LCDBUS_H
#include "Lights.h"
#include "LCD.h"
#include "system_types.h"

class PortBus {
public:
    explicit PortBus(LCD& lcd);
    //explicit PortBus(Lights& lights);
    //Lights& lights;
    LCD& lcd;
    void portAWrite(byte data);
    void portBWrite(byte data);
    //byte portARead() const;
    byte portBRead() const;
};


#endif //_65C02_SYSTEM_EMULATION_LCDBUS_H
