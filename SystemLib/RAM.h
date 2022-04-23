#ifndef INC_65C02_EMULATION_RAM_H
#define INC_65C02_EMULATION_RAM_H

#include <limits>
#include "system_types.h"

class RAM {
public:
    RAM();
    void initialize();
    byte operator[](word address) const;
    byte& operator[](word address);
private:
    static const dword MAX_MEM = std::numeric_limits<word>::max() + 1;
    byte data[MAX_MEM];
};

#endif //INC_65C02_EMULATION_RAM_H
