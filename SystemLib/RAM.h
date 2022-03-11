#ifndef INC_65C02_EMULATION_RAM_H
#define INC_65C02_EMULATION_RAM_H

#include "system_types.h"
#include <limits>

class RAM {
public:
    RAM() { initialize(); }
    void initialize() {
        for(dword i{}; i < MAX_MEM; ++i)
            data[i] = 0xea; //nop
    }
    byte operator[](word address) const { return data[address]; }
    byte& operator[](word address) { return data[address]; }

private:
    static const dword MAX_MEM = std::numeric_limits<word>::max() + 1;
    byte data[MAX_MEM];
};

#endif //INC_65C02_EMULATION_RAM_H
