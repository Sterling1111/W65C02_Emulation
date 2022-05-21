#ifndef _65C02_SYSTEM_EMULATION_CYCLES_H
#define _65C02_SYSTEM_EMULATION_CYCLES_H
#include "system_types.h"
#ifdef __linux__
#include <x86intrin.h>
#include <fstream>
#endif
#ifdef _WIN32
#include <intrin.h>
#endif

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
