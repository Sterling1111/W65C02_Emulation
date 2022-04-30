#include "Cycles.h"

Cycles::Cycles() {
    cycles = 0;
    startTimePoint = 0;
    cycleDuration = 0;
    TCSFrequency = getTSCFrequency();
}

dword Cycles::getTSCFrequency() {
    int cpui[4];
    __cpuid(cpui, 0x16);
    return cpui[0]; //eax which contains TSC frequency in Mhz;
}

Cycles& Cycles::operator++() {
    ++cycles;
    //busy wait. There is no other way.
    while((__builtin_ia32_rdtsc() - startTimePoint) < cycleDuration);
    startTimePoint = __builtin_ia32_rdtsc();
    return *this;
}

Cycles& Cycles::operator+=(sdword num) {
    for (int i = 0; i < num; ++i) {
        this->operator++();
    }
    return *this;
}

bool Cycles::operator> (sdword other) const {return cycles > other;}

void Cycles::reset() {
    cycles = 0;
    startTimePoint = __builtin_ia32_rdtsc();
}
sdword Cycles::getCycles() const {return cycles;}

void Cycles::setCycleDuration(double Mhz) {
    cycleDuration = TCSFrequency / Mhz;
}
