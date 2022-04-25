#ifndef _65C02_SYSTEM_EMULATION_LIGHTS_H
#define _65C02_SYSTEM_EMULATION_LIGHTS_H

#include <bitset>

class Lights {
public:
    Lights();
    std::bitset<8> lights;
};


#endif //_65C02_SYSTEM_EMULATION_LIGHTS_H
