#include "gtest/gtest.h"
#include "System.h"
#include "CyclePacer.h"
#include <limits>

using Clock = CyclePacer::Clock;
using namespace std::chrono;

TEST(CyclePacer, CarriesInstructionOvershootAndLateWakeupsWithoutDrift) {
    CyclePacer pacer(1000000);
    const Clock::time_point start{};
    pacer.reset(100, start);
    EXPECT_EQ(pacer.deadline(1103), start + microseconds(1003));
    // Even after a late wakeup, the second deadline stays relative to start.
    EXPECT_EQ(pacer.deadline(2106), start + microseconds(2006));
    EXPECT_EQ(pacer.deadline(1000000100), start + seconds(1000));
}

TEST(CyclePacer, SupportsFractionalRatesAndRebasesAfterPause) {
    CyclePacer pacer(1500000);
    const Clock::time_point start{};
    pacer.reset(0, start);
    EXPECT_EQ(pacer.batchCycles(), 1500);
    EXPECT_EQ(pacer.deadline(3), start + microseconds(2));
    pacer.reset(10000, start + seconds(10));
    EXPECT_EQ(pacer.deadline(11500), start + seconds(10) + milliseconds(1));
    EXPECT_EQ(CyclePacer(0.5).batchCycles(), 1);
}

TEST(CyclePacer, RejectsInvalidFrequencies) {
    EXPECT_THROW(CyclePacer(0), std::invalid_argument);
    EXPECT_THROW(CyclePacer(-1), std::invalid_argument);
    EXPECT_THROW(CyclePacer(std::numeric_limits<double>::infinity()), std::invalid_argument);
    EXPECT_THROW(CyclePacer(std::numeric_limits<double>::quiet_NaN()), std::invalid_argument);
}

class CpuTimingTest : public testing::Test {
protected:
    System system{0, 0xFFFF, -1, -1, -1, -1};

    void SetUp() override {
        // The default EEPROM is filled with EA, so its reset vector is EAEA.
        // JMP $EAEA: a three-cycle loop, deliberately not a divisor of 1000.
        system.ram[0xEAEA] = W65C02::INS_JMP_ABS;
        system.ram[0xEAEB] = 0xEA;
        system.ram[0xEAEC] = 0xEA;
    }

    uint64_t cycles() {
        std::lock_guard<std::mutex> lock(system.cpu.stateMutex());
        return system.cpu.cycles.getCycles();
    }

    template<typename Predicate>
    bool waitFor(Predicate predicate) {
        const auto deadline = Clock::now() + seconds(1);
        do {
            {
                std::lock_guard<std::mutex> lock(system.cpu.stateMutex());
                if (predicate()) return true;
            }
            std::this_thread::sleep_for(milliseconds(1));
        } while (Clock::now() < deadline);
        return false;
    }
};

TEST_F(CpuTimingTest, MaintainsConfiguredAverageRateAndCatchesUpAfterDelay) {
    for (double mhz : {0.5, 1.0, 2.0}) {
        system.cpu.setCycleDuration(mhz);
        system.cpu.start();
        const auto beforeReset = cycles();
        system.cpu.reset(0xEAEA);
        ASSERT_TRUE(waitFor([&] { return system.cpu.cycles.getCycles() > beforeReset; }));
        const auto start = Clock::now();
        const auto initialCycles = cycles();
        // Simulate a stalled LCD snapshot: the worker must recover lost time.
        {
            std::lock_guard<std::mutex> lock(system.cpu.stateMutex());
            std::this_thread::sleep_for(milliseconds(30));
        }
        std::this_thread::sleep_until(start + milliseconds(500));
        const auto executed = cycles() - initialCycles;
        const double elapsed = duration<double>(Clock::now() - start).count();
        const double measuredHz = executed / elapsed;
        RecordProperty("measured_hz_" + std::to_string(mhz), std::to_string(measuredHz));
        // Allow scheduler noise on loaded CI hosts; arithmetic above is exact.
        EXPECT_NEAR(measuredHz, mhz * 1000000, mhz * 1000000 * 0.02);
        system.cpu.stop();
    }
}

TEST_F(CpuTimingTest, SleepsBeforeResetAndClocksPeripheralsAfterStop) {
    system.ram[0xEAEA] = 0xDB; // STP
    system.cpu.start();
    std::this_thread::sleep_for(milliseconds(20));
    EXPECT_EQ(cycles(), 0);
    system.cpu.reset(0xEAEA);
    ASSERT_TRUE(waitFor([&] { return system.cpu.STOP; }));
    const auto stoppedCycles = cycles();
    system.cpu.interrupt(false, true);
    std::this_thread::sleep_for(milliseconds(20));
    EXPECT_GT(cycles(), stoppedCycles);
    {
        std::lock_guard<std::mutex> lock(system.cpu.stateMutex());
        EXPECT_TRUE(system.cpu.STOP);
    }
    {
        std::lock_guard<std::mutex> lock(system.cpu.stateMutex());
        system.ram[0xEAEA] = W65C02::INS_JMP_ABS;
    }
    system.cpu.reset(0xEAEA);
    EXPECT_TRUE(waitFor([&] { return system.cpu.cycles.getCycles() > stoppedCycles + 1000; }));
}

TEST_F(CpuTimingTest, WaitKeepsClockRunningUntilInterrupt) {
    system.ram[0xEAEA] = 0xCB; // WAI (reset masks IRQ, which still wakes WAI)
    system.ram[0xEAEB] = 0xDB; // STP
    system.cpu.start();
    system.cpu.reset(0xEAEA);
    ASSERT_TRUE(waitFor([&] { return system.cpu.WAIT; }));
    const auto waitingCycles = cycles();
    std::this_thread::sleep_for(milliseconds(20));
    EXPECT_GT(cycles(), waitingCycles);
    {
        std::lock_guard<std::mutex> lock(system.cpu.stateMutex());
        EXPECT_TRUE(system.cpu.WAIT);
    }
    system.cpu.interrupt(false, true);
    EXPECT_TRUE(waitFor([&] { return system.cpu.STOP; }));
}

TEST_F(CpuTimingTest, ResetAndShutdownInterruptLongSleep) {
    system.cpu.setCycleDuration(0.000001); // 1 Hz: a JMP has a 3-second deadline.
    const auto start = Clock::now();
    {
        system.cpu.start();
        system.cpu.reset(0xEAEA);
        ASSERT_TRUE(waitFor([&] { return system.cpu.cycles.getCycles() >= 3; }));
        system.cpu.reset(0xEAEA);
        ASSERT_TRUE(waitFor([&] { return system.cpu.cycles.getCycles() >= 6; }));
        system.cpu.stop();
    }
    EXPECT_LT(Clock::now() - start, milliseconds(500));
}
