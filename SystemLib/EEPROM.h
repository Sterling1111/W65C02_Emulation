#ifndef _65C02_SYSTEM_EMULATION_EEPROM_H
#define _65C02_SYSTEM_EMULATION_EEPROM_H


#include "system_types.h"
#include "fstream"

class EEPROM {
public:
    EEPROM();
    void initialize();
    void loadProgram(const std::string& programObjFile);
    byte operator[](word address) const;
private:
    static constexpr word MAX_MEM = 1024 * 32;
    byte data[MAX_MEM];
};



#endif //_65C02_SYSTEM_EMULATION_EEPROM_H
