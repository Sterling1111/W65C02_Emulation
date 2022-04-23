#ifndef _65C02_SYSTEM_EMULATION_CYCLES_H
#define _65C02_SYSTEM_EMULATION_CYCLES_H
#include "system_types.h"
#include <intrin.h>

class Cycles {
public:
    Cycles();
    static dword getTSCFrequency();
    Cycles& operator++();
    Cycles& operator+=(sdword);
    bool operator> (sdword) const;
    void reset();
    sdword getCycles() const;
    void setCycleDuration(double Mhz);

private:
    sdword cycles{};
    uint64_t startTimePoint{};
    uint64_t cycleDuration{};
    uint64_t TCSFrequency{};
};



#endif //_65C02_SYSTEM_EMULATION_CYCLES_H
