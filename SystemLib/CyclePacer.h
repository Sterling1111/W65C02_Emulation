#ifndef W65C02_CYCLE_PACER_H
#define W65C02_CYCLE_PACER_H

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <stdexcept>

// Absolute deadlines retain instruction overshoot and scheduler delays instead
// of adding a fresh sleep after every batch (which would accumulate drift).
class CyclePacer {
public:
    using Clock = std::chrono::steady_clock;

    explicit CyclePacer(double frequencyHz) : frequencyHz(frequencyHz) {
        if (!std::isfinite(frequencyHz) || frequencyHz <= 0 || frequencyHz > 1e12)
            throw std::invalid_argument("CPU frequency must be in (0, 1e12] Hz");
    }

    void reset(uint64_t cycles, Clock::time_point now = Clock::now()) {
        originCycles = cycles;
        origin = now;
    }

    uint64_t batchCycles() const {
        return std::max<uint64_t>(1, static_cast<uint64_t>(frequencyHz / 1000));
    }

    Clock::time_point deadline(uint64_t cycles) const {
        return origin + std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>((cycles - originCycles) / frequencyHz));
    }

private:
    double frequencyHz;
    uint64_t originCycles{};
    Clock::time_point origin{};
};

#endif
