#include "Cycles.h"

Cycles::Cycles() {
    cycles = 0;
    startTimePoint = 0;
    cycleDuration = 0;
    TCSFrequency = getTSCFrequency();
}

dword Cycles::getTSCFrequency() {
#ifdef __linux__
    std::ifstream inputFile("/proc/cpuinfo");
    if(inputFile.is_open()) {
        std::string line;
        while (inputFile.good()) {
            std::getline(inputFile, line);
            if (line.find("cpu MHz") != std::string::npos) {
                char const *digits = "0123456789";
                std::size_t const n = line.find_first_of(digits);
                std::size_t const m = line.find_first_not_of(digits, n);
                int MHz = stoi(line.substr(n, m != std::string::npos ? m - n : m));
                inputFile.close();
                return MHz;
            }
        }
    } return 2400;
#endif
#ifdef _WIN32
    int cpui[4];
    __cpuid(cpui, 0x16);
    return cpui[0] > 500 ? cpui[0] : 2400;  //just a hack so the program will still run if
                                            //eax does not contain TSC frequency. It only will
                                            //on newer intell processors....
#endif
exit(69420);
}

Cycles& Cycles::operator++() {
    static int reps{3};
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
uint64_t Cycles::getCycles() const {return cycles;}

void Cycles::setCycleDuration(double Mhz) {
    cycleDuration = (TCSFrequency - 65 * Mhz) / Mhz;
}
