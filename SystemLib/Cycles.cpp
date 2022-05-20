#include "Cycles.h"

Cycles::Cycles() {
    cycles = 0;
    startTimePoint = 0;
    cycleDuration = 0;
    TCSFrequency = getTSCFrequency();
}

dword Cycles::getTSCFrequency() {
    unsigned eax{}, ebx{}, ecx{}, edx{};
    __get_cpuid(0x16, &eax, &ebx, &ecx, &edx);
    return eax > 500 ? eax : 2400; //just a hack so the program will still run if
                                            //eax does not contain TSC frequency. It only will
                                            //on newer intell processors....
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
    cycleDuration = (TCSFrequency - 65 * Mhz) / Mhz;
}
