#ifndef _65C02_SYSTEM_EMULATION_CYCLES_H
#define _65C02_SYSTEM_EMULATION_CYCLES_H
#include "system_types.h"
#include <functional>
#include <utility>
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
    uint64_t getCycles() const;
    void setCycleDuration(double Mhz);
    void setTickCallback(std::function<void()> callback) { onTick = std::move(callback); }
    double getFrequencyHz() const { return frequencyHz; }

private:
    std::function<void()> onTick;
    uint64_t cycles{};
    double frequencyHz{1000000};
};



#endif //_65C02_SYSTEM_EMULATION_CYCLES_H
