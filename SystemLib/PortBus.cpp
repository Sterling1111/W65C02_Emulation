#include "PortBus.h"

//PortBus::PortBus(Lights& lights) : lights{lights} {}
PortBus::PortBus(LCD &lcd) : lcd{lcd} {};

void PortBus::portAWrite(byte data) {
    lcd.portAWrite(data);
}

void PortBus::portBWrite(byte data) {
    lcd.portBWrite(data);
}

byte PortBus::portBRead() const {
    return lcd.portBRead();
}
