#include "PortBus.h"

PortBus::PortBus(Lights& lights) : lights{lights} {}

byte PortBus::read() const {
    return lights.lights.to_ulong();
}

void PortBus::write(byte data) {
    for(int i = 0; i < 8; i++) {
        lights.lights.set(i, data & (1 << i));
    }

}