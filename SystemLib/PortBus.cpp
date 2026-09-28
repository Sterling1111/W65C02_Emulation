#include "PortBus.h"

//PortBus::PortBus(Lights& lights) : lights{lights} {}
PortBus::PortBus(LCD &lcd) : lcd{lcd} {};

void PortBus::portAWrite(byte data) {
    lcd.portAWrite(data);
}

void PortBus::portBWrite(byte data, byte outputMask) {
    lcd.portBWrite(data, outputMask);
}

byte PortBus::portBRead() const {
    return lcd.portBRead();
}
