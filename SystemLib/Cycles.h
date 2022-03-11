#ifndef _65C02_SYSTEM_EMULATION_CYCLES_H
#define _65C02_SYSTEM_EMULATION_CYCLES_H

struct Cycles {
    explicit Cycles(double Mhz = 1) : cycles{0} {
        setCycleDuration(Mhz);
    };
    static dword getTCSFrequency() {
        int eax{};
        __asm__("mov $0x16, %eax\n\t");
        __asm__("cpuid\n\t");
        __asm__("mov %%eax, %0\n\t":"=r" (eax));
        return eax;
    }
    Cycles&  operator++(){
        ++cycles;
        //busy wait. There is no other way.
        while((__builtin_ia32_rdtsc() - startTimePoint) < cycleDuration);
        startTimePoint = __builtin_ia32_rdtsc();
        return *this;
    }
    Cycles& operator+=(sdword num) {
        for (int i = 0; i < num; ++i) {
            this->operator++();
        }
        return *this;
    }
    bool operator> (sdword other) const {return cycles > other;}
    void reset() {
        cycles = 0;
        startTimePoint = __builtin_ia32_rdtsc();
    }
    sdword getCycles() const {return cycles;}
    void setCycleDuration(double Mhz) {
        cycleDuration = (getTCSFrequency() - (30 * Mhz)) / Mhz;
    }
private:
    sdword cycles;
    uint64_t startTimePoint;
    uint64_t cycleDuration;
};

#endif //_65C02_SYSTEM_EMULATION_CYCLES_H
