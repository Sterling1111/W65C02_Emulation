#include "gtest/gtest.h"
#include "System.h"
#include <chrono>
#include <thread>
#include <vector>

using VIA = W65C22;
namespace {
void write(VIA& v, VIA::Register reg, byte data) { v.writeToRegisters(data, reg); }
byte read(VIA& v, VIA::Register reg) { return v.readFromRegisters(reg); }
void timer1(VIA& v, word count) {
    write(v, VIA::T1CL, byte(count)); write(v, VIA::T1CH, byte(count >> 8));
}
void timer2(VIA& v, word count) {
    write(v, VIA::T2CL, byte(count)); write(v, VIA::T2CH, byte(count >> 8));
}
word count1(VIA& v) { const byte hi = read(v, VIA::T1CH); return word((hi << 8) | read(v, VIA::T1CL)); }
word count2(VIA& v) { const byte hi = read(v, VIA::T2CH); return word((hi << 8) | read(v, VIA::T2CL)); }
}

TEST(ViaPorts, DirectionLatchesReadbackAndAddressMirrors) {
    VIA v;
    v.setPortAInput(0xa5, 0x0f); v.setPortBInput(0x5a, 0x0f);
    write(v, VIA::ORA, 0xc3); write(v, VIA::ORB, 0x69);
    write(v, VIA::DDRA, 0xf0); write(v, VIA::DDRB, 0xf0);
    EXPECT_EQ(read(v, VIA::DDRA), 0xf0); EXPECT_EQ(read(v, VIA::DDRB), 0xf0);
    EXPECT_EQ(read(v, VIA::ORA), 0xc5); EXPECT_EQ(read(v, VIA::ORB), 0x6a);
    v.writeToRegisters(0x39, 0x601f);
    EXPECT_EQ(v.readFromRegisters(0x6001), 0x35);
    EXPECT_EQ(v.portARead(), 0x35);
    // A reads the physical pins; B reads its output latch on output bits.
    v.setPortAInput(0, 0xff); v.setPortBInput(0, 0xff);
    EXPECT_EQ(read(v, VIA::ORA), 0);
    EXPECT_EQ(read(v, VIA::ORB), 0x60);
}

TEST(ViaPorts, InputLatchesCaptureEdgesAndBecomeTransparentAfterAcknowledgement) {
    VIA v;
    write(v, VIA::ACR, 3);
    v.setPortAInput(0x12); v.setPortBInput(0x34);
    v.setCA1(false); v.setCB1(false);
    v.setPortAInput(0xab); v.setPortBInput(0xcd);
    EXPECT_EQ(read(v, VIA::ORA_NO_HANDSHAKE), 0x12);
    EXPECT_EQ(read(v, VIA::IFR) & 0x12, 0x12);
    EXPECT_EQ(read(v, VIA::ORA), 0x12);
    EXPECT_EQ(read(v, VIA::ORB), 0x34);
    EXPECT_EQ(read(v, VIA::ORA), 0xab);
    EXPECT_EQ(read(v, VIA::ORB), 0xcd);
    EXPECT_EQ(read(v, VIA::IFR) & 0x12, 0);
}

TEST(ViaPorts, DisablingLatchesReturnsLiveInputAndBusHoldSurvivesRelease) {
    VIA v;
    v.setPortAInput(0x55); v.setPortBInput(0xaa);
    v.releasePortA(); v.releasePortB();
    EXPECT_EQ(v.portARead(), 0x55); EXPECT_EQ(v.portBRead(), 0xaa);
    write(v, VIA::ACR, 1); v.setCA1(false); v.setPortAInput(0xf0);
    write(v, VIA::ACR, 0);
    EXPECT_EQ(read(v, VIA::ORA), 0xf0);
    write(v, VIA::DDRA, 0xff); write(v, VIA::ORA, 0x36); v.releasePortA();
    write(v, VIA::DDRA, 0);
    EXPECT_EQ(v.portARead(), 0x36);
}

class ViaControlInputs : public testing::TestWithParam<unsigned> {};
TEST_P(ViaControlInputs, PolarityIndependentFlagsAndNoHandshakeAlias) {
    const unsigned mode = GetParam();
    VIA v;
    write(v, VIA::PCR, byte((mode << 1) | (mode << 5)));
    const bool positive = (mode & 2) != 0;
    v.setCA2(!positive); v.setCB2(!positive);
    write(v, VIA::IFR, 0x7f);
    v.setCA2(positive); v.setCB2(positive);
    EXPECT_EQ(read(v, VIA::IFR) & 9, 9);
    write(v, VIA::ORA_NO_HANDSHAKE, 0); read(v, VIA::ORA_NO_HANDSHAKE);
    EXPECT_EQ(read(v, VIA::IFR) & 9, 9);
    read(v, VIA::ORA); write(v, VIA::ORB, 0);
    EXPECT_EQ(read(v, VIA::IFR) & 9, (mode & 1) ? 9 : 0);
    write(v, VIA::IFR, 9);
    EXPECT_EQ(read(v, VIA::IFR), 0);
}
INSTANTIATE_TEST_SUITE_P(AllInputModes, ViaControlInputs, testing::Values(0, 1, 2, 3));

TEST(ViaHandshake, BothControl1PolaritiesLatchAndRaiseFlags) {
    for (byte pcr : {byte(0), byte(0x11)}) {
        VIA v;
        write(v, VIA::PCR, pcr);
        const bool active = pcr != 0;
        v.setCA1(!active); v.setCB1(!active); write(v, VIA::IFR, 0x7f);
        v.setCA1(active); v.setCB1(active);
        EXPECT_EQ(read(v, VIA::IFR) & 0x12, 0x12);
    }
}

TEST(ViaHandshake, HandshakeReturnsHighOnControl1AndBReadDoesNotTriggerOutput) {
    VIA v;
    write(v, VIA::PCR, 0x88); // Both C2 pins handshake outputs.
    EXPECT_TRUE(v.ca2Level()); EXPECT_TRUE(v.cb2Level());
    read(v, VIA::ORA);
    EXPECT_FALSE(v.ca2Level());
    read(v, VIA::ORB);
    EXPECT_TRUE(v.cb2Level());
    write(v, VIA::ORB, 0x55);
    EXPECT_FALSE(v.cb2Level());
    v.tick(10);
    EXPECT_FALSE(v.ca2Level()); EXPECT_FALSE(v.cb2Level());
    v.setCA1(false); v.setCB1(false);
    EXPECT_TRUE(v.ca2Level()); EXPECT_TRUE(v.cb2Level());
    write(v, VIA::ORA_NO_HANDSHAKE, 0); read(v, VIA::ORA_NO_HANDSHAKE);
    EXPECT_TRUE(v.ca2Level());
}

TEST(ViaHandshake, PulseOutputsLastOnePhi2PeriodAndFixedOutputsIgnoreAccesses) {
    VIA v;
    write(v, VIA::PCR, 0xaa);
    write(v, VIA::ORA, 0); write(v, VIA::ORB, 0);
    EXPECT_FALSE(v.ca2Level()); EXPECT_FALSE(v.cb2Level());
    v.tick();
    EXPECT_TRUE(v.ca2Level()); EXPECT_TRUE(v.cb2Level());
    write(v, VIA::PCR, 0xcc);
    v.tick();
    EXPECT_FALSE(v.ca2Level()); EXPECT_FALSE(v.cb2Level());
    write(v, VIA::PCR, 0xee);
    read(v, VIA::ORA); write(v, VIA::ORB, 0); v.setCA1(false); v.setCB1(false);
    EXPECT_TRUE(v.ca2Level()); EXPECT_TRUE(v.cb2Level());
    EXPECT_EQ(read(v, VIA::PCR), 0xee);
    EXPECT_EQ(read(v, VIA::IFR) & 9, 0);
}

TEST(ViaInterrupts, FlagsLatchWithoutEnableAndIerUsesSetClearSemantics) {
    VIA v;
    EXPECT_EQ(read(v, VIA::IER), 0x80);
    v.setCA1(false); v.setCB1(false);
    EXPECT_EQ(read(v, VIA::IFR), 0x12);
    EXPECT_FALSE(v.irqAsserted());
    write(v, VIA::IER, 0x82);
    EXPECT_EQ(read(v, VIA::IFR), 0x92); EXPECT_TRUE(v.irqAsserted());
    write(v, VIA::IER, 0x90); write(v, VIA::IER, 0x02);
    EXPECT_EQ(read(v, VIA::IER), 0x90); EXPECT_TRUE(v.irqAsserted());
    write(v, VIA::IFR, 0x80); EXPECT_TRUE(v.irqAsserted());
    write(v, VIA::IFR, 0x10);
    EXPECT_EQ(read(v, VIA::IFR), 2); EXPECT_FALSE(v.irqAsserted());
    write(v, VIA::IER, 0x82); EXPECT_TRUE(v.irqAsserted());
    read(v, VIA::ORA); EXPECT_FALSE(v.irqAsserted());
}

TEST(ViaInterrupts, SAndNHaveDistinctIrqDriveTypes) {
    VIA s, n(VIA::Variant::W65C22N);
    EXPECT_EQ(s.pins().irq, VIA::Drive::High);
    EXPECT_EQ(n.pins().irq, VIA::Drive::HighImpedance);
    for (auto v : {&s, &n}) {
        write(*v, VIA::IER, 0x82); v->setCA1(false);
        EXPECT_EQ(v->pins().irq, VIA::Drive::Low);
    }
}

TEST(ViaTimer1, ZeroCountUnderflowsOnNextTickAndLatchReadsDoNotAcknowledge) {
    VIA v;
    write(v, VIA::IER, 0xc0); timer1(v, 0);
    EXPECT_FALSE(v.irqAsserted()); v.tick();
    EXPECT_TRUE(v.irqAsserted());
    EXPECT_EQ(read(v, VIA::T1CH), 0xff);
    EXPECT_EQ(read(v, VIA::T1LL), 0); EXPECT_EQ(read(v, VIA::T1LH), 0);
    EXPECT_TRUE(v.irqAsserted());
    EXPECT_EQ(read(v, VIA::T1CL), 0xff); EXPECT_FALSE(v.irqAsserted());
    v.tick(100); EXPECT_EQ(read(v, VIA::IFR), 0);
}

TEST(ViaTimer1, OneShotPb7AndRetriggering) {
    VIA v;
    write(v, VIA::DDRB, 0x80); write(v, VIA::ORB, 0x80); write(v, VIA::ACR, 0x80);
    timer1(v, 2); EXPECT_EQ(v.portBRead() & 0x80, 0);
    v.tick(2); EXPECT_EQ(count1(v), 0); EXPECT_EQ(read(v, VIA::IFR), 0);
    v.tick(); EXPECT_EQ(read(v, VIA::IFR), 0x40); EXPECT_EQ(v.portBRead() & 0x80, 0x80);
    read(v, VIA::T1CL); v.tick(30); EXPECT_EQ(read(v, VIA::IFR), 0);
    timer1(v, 2); EXPECT_EQ(v.portBRead() & 0x80, 0);
    v.tick(); timer1(v, 3); v.tick(3); EXPECT_EQ(read(v, VIA::IFR), 0);
    v.tick(); EXPECT_EQ(read(v, VIA::IFR), 0x40);
}

TEST(ViaTimer1, FreeRunningPeriodIsLatchPlusTwoAndLatchWritesAffectNextPeriod) {
    VIA v;
    write(v, VIA::ACR, 0xc0); write(v, VIA::DDRB, 0x80); timer1(v, 2);
    v.tick(3); EXPECT_EQ(v.portBRead() & 0x80, 0x80);
    write(v, VIA::T1LL, 4); write(v, VIA::T1LH, 0);
    EXPECT_EQ(read(v, VIA::IFR), 0);
    v.tick(); EXPECT_EQ(count1(v), 4);
    v.tick(4); EXPECT_EQ(read(v, VIA::IFR), 0);
    v.tick(); EXPECT_EQ(read(v, VIA::IFR), 0x40); EXPECT_EQ(v.portBRead() & 0x80, 0);
    read(v, VIA::T1CL); v.tick(5); EXPECT_EQ(read(v, VIA::IFR), 0);
    v.tick(); EXPECT_EQ(read(v, VIA::IFR), 0x40);
}

TEST(ViaTimer1, LatchAndCounterAreIndependentAndPb7HonorsDirection) {
    VIA v;
    timer1(v, 0x1234); write(v, VIA::T1LL, 0x78); write(v, VIA::T1LH, 0x56);
    EXPECT_EQ(count1(v), 0x1234);
    EXPECT_EQ(read(v, VIA::T1LL), 0x78); EXPECT_EQ(read(v, VIA::T1LH), 0x56);
    write(v, VIA::ACR, 0x80); v.setPortBInput(0x80);
    timer1(v, 1);
    EXPECT_EQ(v.pins().pbOutputMask, 0); EXPECT_EQ(v.portBRead(), 0x80);
}

TEST(ViaTimer2, OneShotContinuesCountingAndOnlyHighWriteRearms) {
    VIA v;
    timer2(v, 1); v.tick(); EXPECT_EQ(count2(v), 0);
    v.tick(); EXPECT_EQ(read(v, VIA::IFR), 0x20); EXPECT_EQ(count2(v), 0xffff);
    v.tick(65536); EXPECT_EQ(read(v, VIA::IFR), 0);
    write(v, VIA::T2CL, 0); v.tick(3); EXPECT_EQ(read(v, VIA::IFR), 0);
    write(v, VIA::T2CH, 0); v.tick(); EXPECT_EQ(read(v, VIA::IFR), 0x20);
    write(v, VIA::T2CH, 0); EXPECT_EQ(read(v, VIA::IFR), 0);
}

TEST(ViaTimer2, CountsSampledNegativePb6EdgesAndNotClockOrOtherPins) {
    VIA v;
    v.setPortBInput(0x40); write(v, VIA::ACR, 0x20); timer2(v, 1);
    v.tick(10); EXPECT_EQ(count2(v), 1);
    v.setPortBInput(0); v.tick(); EXPECT_EQ(count2(v), 0);
    v.tick(10); EXPECT_EQ(read(v, VIA::IFR), 0);
    v.setPortBInput(0x40); v.tick(); v.setPortBInput(0); v.tick();
    EXPECT_EQ(read(v, VIA::IFR), 0x20); EXPECT_EQ(count2(v), 0xffff);
}

class ViaSerialOutput : public testing::TestWithParam<unsigned> {};
TEST_P(ViaSerialOutput, MsbFirstRecirculationCompletionAndClockOwnership) {
    const unsigned mode = GetParam();
    VIA v;
    write(v, VIA::T2CL, 1); write(v, VIA::ACR, byte(mode << 2));
    std::vector<bool> bits;
    bool previousClock = true;
    v.setPinCallback([&](const VIA::Pins&) {
        const bool clock = v.cb1Level();
        if (previousClock && !clock) bits.push_back(v.cb2Level());
        previousClock = clock;
    });
    write(v, VIA::SR, 0xa6);
    if (mode == 7) {
        for (int i = 0; i < 8; ++i) { v.setCB1(false); v.tick(); v.setCB1(true); v.tick(); }
    } else {
        v.tick(mode == 6 ? 17 : 48);
    }
    ASSERT_EQ(bits.size(), 8u);
    EXPECT_EQ(bits, (std::vector<bool>{1,0,1,0,0,1,1,0}));
    EXPECT_EQ(read(v, VIA::IFR) & 4, mode == 4 ? 0 : 4);
    EXPECT_FALSE(v.cb2Level());
    if (mode == 4) {
        v.tick(6); ASSERT_EQ(bits.size(), 9u); EXPECT_TRUE(bits.back());
        EXPECT_EQ(read(v, VIA::IFR) & 4, 0);
    } else if (mode != 7) {
        v.tick(100); EXPECT_EQ(bits.size(), 8u); EXPECT_FALSE(v.cb2Level());
    } else {
        write(v, VIA::IFR, 0x7f);
        for (int i = 0; i < 8; ++i) { v.setCB1(false); v.setCB1(true); }
        EXPECT_EQ(read(v, VIA::IFR) & 4, 4); EXPECT_EQ(bits.size(), 16u);
    }
}
INSTANTIATE_TEST_SUITE_P(AllOutputModes, ViaSerialOutput, testing::Values(4, 5, 6, 7));

class ViaSerialInput : public testing::TestWithParam<unsigned> {};
TEST_P(ViaSerialInput, SamplesAfterRisingEdgesAndReportsEightBits) {
    const unsigned mode = GetParam();
    VIA v;
    write(v, VIA::ACR, byte(mode << 2)); write(v, VIA::T2CL, 0);
    unsigned bit = 0;
    bool previousClock = true;
    v.setPinCallback([&](const VIA::Pins&) {
        const bool clock = v.cb1Level();
        const bool falling = previousClock && !clock;
        previousClock = clock;
        if (falling) v.setCB2((0xb3 & (0x80 >> (bit++ % 8))) != 0);
    });
    write(v, VIA::SR, 0);
    if (mode == 3) {
        for (int i = 0; i < 8; ++i) {
            v.setCB1(false); v.tick(); v.setCB1(true);
            if (i == 7) EXPECT_EQ(read(v, VIA::IFR) & 4, 0);
            v.tick();
        }
    } else v.tick(mode == 2 ? 18 : 33);
    EXPECT_EQ(read(v, VIA::IFR) & 4, 4);
    EXPECT_EQ(read(v, VIA::SR), 0xb3);
    EXPECT_EQ(read(v, VIA::IFR) & 4, 0); // Reading starts the next transfer.
}
INSTANTIATE_TEST_SUITE_P(AllInputModes, ViaSerialInput, testing::Values(1, 2, 3));

TEST(ViaSerial, DisabledModeAndModeChangesReleaseControlPins) {
    VIA v;
    write(v, VIA::SR, 0xa5); v.tick(100);
    EXPECT_EQ(read(v, VIA::SR), 0xa5); EXPECT_EQ(read(v, VIA::IFR) & 4, 0);
    write(v, VIA::PCR, 0xc0); write(v, VIA::ACR, 0x18); write(v, VIA::SR, 0xff);
    v.tick(17); EXPECT_EQ(read(v, VIA::IFR) & 4, 4); EXPECT_TRUE(v.cb2Level());
    write(v, VIA::ACR, 0);
    EXPECT_EQ(read(v, VIA::IFR) & 4, 0);
    EXPECT_FALSE(v.cb2Level()); EXPECT_EQ(v.pins().cb1, VIA::Drive::HighImpedance);
}

TEST(ViaSerial, Phi2ShiftingLeavesTimer2IndependentAndT2ModeSharesLowCounter) {
    VIA v;
    timer2(v, 100); write(v, VIA::ACR, 0x18); write(v, VIA::SR, 0xaa);
    v.tick(17); EXPECT_EQ(count2(v), 83);
    write(v, VIA::ACR, 0x14); write(v, VIA::T2CL, 2); write(v, VIA::SR, 0xaa);
    EXPECT_EQ(read(v, VIA::T2CL), 2);
    v.tick(3); EXPECT_EQ(read(v, VIA::T2CL), 0xff); EXPECT_TRUE(v.cb1Level());
    v.tick(); EXPECT_EQ(read(v, VIA::T2CL), 2); EXPECT_FALSE(v.cb1Level());
}

TEST(ViaReset, ClearsControlAndInterruptsButPreservesTimersLatchesAndSr) {
    VIA v;
    timer1(v, 0x1234); timer2(v, 0x5678); write(v, VIA::SR, 0xa5);
    write(v, VIA::DDRA, 0xff); write(v, VIA::DDRB, 0xff);
    write(v, VIA::IER, 0xff); write(v, VIA::PCR, 0xee);
    v.setRESB(false); v.tick(100); write(v, VIA::DDRA, 0xff);
    EXPECT_EQ(v.pins().paOutputMask, 0); EXPECT_EQ(v.pins().pbOutputMask, 0);
    EXPECT_EQ(v.pins().ca2, VIA::Drive::HighImpedance); EXPECT_FALSE(v.irqAsserted());
    v.setRESB(true);
    EXPECT_EQ(read(v, VIA::ACR), 0); EXPECT_EQ(read(v, VIA::PCR), 0);
    EXPECT_EQ(read(v, VIA::IER), 0x80); EXPECT_EQ(read(v, VIA::IFR), 0);
    EXPECT_EQ(count1(v), 0x1234); EXPECT_EQ(count2(v), 0x5678); EXPECT_EQ(read(v, VIA::SR), 0xa5);
    EXPECT_EQ(read(v, VIA::T1LL), 0x34); EXPECT_EQ(read(v, VIA::T1LH), 0x12);
    v.tick(100); EXPECT_EQ(count1(v), 0x1234); EXPECT_EQ(count2(v), 0x5678);
}

TEST(ViaBus, DeselectedReadsHaveNoSideEffectsAndTimersRunWithoutChipSelect) {
    VIA v;
    timer1(v, 0); v.tick();
    EXPECT_FALSE(v.busCycle(false, false, true, VIA::T1CL).driving);
    EXPECT_FALSE(v.busCycle(true, true, true, VIA::T1CL).driving);
    EXPECT_EQ(read(v, VIA::IFR), 0x40);
    EXPECT_TRUE(v.busCycle(true, false, true, VIA::T1CL).driving);
    EXPECT_EQ(read(v, VIA::IFR), 0);
    v.busCycle(false, false, false, VIA::DDRA, 0xff); EXPECT_EQ(read(v, VIA::DDRA), 0);
    v.busCycle(true, false, false, VIA::DDRA, 0xff); EXPECT_EQ(read(v, VIA::DDRA), 0xff);
    v.setRESB(false); EXPECT_FALSE(v.busCycle(true, false, true, VIA::IER).driving);
}

TEST(ViaIntegration, CpuCyclesClockTimersAndViaIrqWakesWaitAndVectors) {
    System s{0, 0x3fff, 0x6000, 0x600f, 0x8000, 0xffff};
    // Use RAM for vectors to avoid filesystem fixtures.
    s.bus.romMin = -1; s.bus.romMax = -1; s.bus.ramMax = 0xffff;
    // RAM priority requires direct VIA programming in this fixture.
    s.ram[0xfffe] = 0; s.ram[0xffff] = 3;
    s.ram[0x200] = 0xcb; // WAI
    s.ram[0x201] = 0xea;
    s.ram[0x300] = 0xa9; s.ram[0x301] = 0x42; // LDA #$42
    s.cpu.reset(0x200); s.cpu.PS.reset(W65C02::I);
    write(s.registers, VIA::IER, 0xc0); timer1(s.registers, 10);
    s.cpu.execute(); EXPECT_TRUE(s.cpu.WAIT);
    for (int i = 0; i < 30 && s.cpu.A != 0x42; ++i) s.cpu.execute();
    EXPECT_FALSE(s.cpu.WAIT); EXPECT_EQ(s.cpu.A, 0x42);
    EXPECT_TRUE(s.cpu.PS.test(W65C02::I));
    EXPECT_TRUE(s.registers.irqAsserted());
    EXPECT_FALSE(s.cpu.IRQB); // VIA never overwrites the separate manual IRQ source.
}

TEST(ViaIntegration, MemoryMappedWritesReadsAndIdleClocks) {
    System s{0, 0x3fff, 0x6000, 0x7fff, 0x8000, 0xffff};
    s.cpu.writeByte(0xff, 0x6002);
    s.cpu.writeByte(0x5a, 0x6000);
    EXPECT_EQ(s.cpu.readByte(0x6010), 0x5a);
    s.cpu.writeByte(2, 0x6004); s.cpu.writeByte(0, 0x6005);
    EXPECT_EQ(count1(s.registers), 2); // The load doesn't consume an extra timer tick.
    s.cpu.STOP = true; s.cpu.execute(3);
    EXPECT_EQ(read(s.registers, VIA::IFR), 0x40);
    EXPECT_TRUE(s.cpu.STOP);
}

TEST(ViaIntegration, PacedWorkerWakesFromViaTimerWithoutExternalInput) {
    System s{0, 0x3fff, 0x6000, 0x7fff, 0x8000, 0xffff};
    s.ram[0x200] = 0xcb; s.ram[0x201] = 0xdb; // WAI then STP; IRQ is masked.
    write(s.registers, VIA::IER, 0xc0); timer1(s.registers, 10000);
    s.cpu.start(); s.cpu.reset(0x200);
    bool stopped = false;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    while (std::chrono::steady_clock::now() < deadline) {
        {
            std::lock_guard<std::mutex> lock(s.cpu.stateMutex());
            stopped = s.cpu.STOP;
        }
        if (stopped) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EXPECT_TRUE(stopped);
}

class ViaTimerBoundaries : public testing::TestWithParam<unsigned> {};
TEST_P(ViaTimerBoundaries, BothTimersTimeoutAfterFullSixteenBitCount) {
    const auto n = GetParam();
    VIA v;
    timer1(v, word(n)); timer2(v, word(n));
    v.tick(n);
    EXPECT_EQ(read(v, VIA::IFR) & 0x60, 0);
    EXPECT_EQ(count1(v), 0); EXPECT_EQ(count2(v), 0);
    v.tick();
    EXPECT_EQ(read(v, VIA::IFR) & 0x60, 0x60);
    EXPECT_EQ(count1(v), 0xffff); EXPECT_EQ(count2(v), 0xffff);
}
INSTANTIATE_TEST_SUITE_P(ZeroByteCarryAndMaximum, ViaTimerBoundaries,
                        testing::Values(0, 1, 255, 256, 65535));

class ViaTimer1Modes : public testing::TestWithParam<unsigned> {};
TEST_P(ViaTimer1Modes, RepetitionAndPb7SelectionAreIndependent) {
    const auto mode = GetParam();
    VIA v;
    write(v, VIA::DDRB, 0x80); write(v, VIA::ORB, 0x80);
    write(v, VIA::ACR, byte(mode << 6)); timer1(v, 0);
    EXPECT_EQ(v.portBRead() & 0x80, (mode & 2) ? 0 : 0x80);
    v.tick(); EXPECT_EQ(read(v, VIA::IFR) & 0x40, 0x40);
    read(v, VIA::T1CL); v.tick(2);
    EXPECT_EQ(read(v, VIA::IFR) & 0x40, (mode & 1) ? 0x40 : 0);
    EXPECT_EQ(v.portBRead() & 0x80, mode == 3 ? 0 : 0x80);
}
INSTANTIATE_TEST_SUITE_P(AllModes, ViaTimer1Modes, testing::Values(0, 1, 2, 3));

TEST(ViaSerial, ExternalInputContinuesAfterFirstByteWithoutSrAccess) {
    VIA v;
    write(v, VIA::ACR, 0x0c); write(v, VIA::SR, 0);
    for (byte value : {byte(0x96), byte(0x3c)}) {
        write(v, VIA::IFR, 0x7f);
        for (int bit = 7; bit >= 0; --bit) {
            v.setCB1(false); v.setCB2((value & (1 << bit)) != 0);
            v.setCB1(true); v.tick();
        }
        EXPECT_EQ(read(v, VIA::IFR) & 4, 4);
    }
    EXPECT_EQ(read(v, VIA::SR), 0x3c);
}

TEST(ViaSerial, ReadRestartsInternalOutputAndDoesNotLoseData) {
    VIA v;
    write(v, VIA::ACR, 0x18); write(v, VIA::SR, 0x53);
    v.tick(17); EXPECT_EQ(read(v, VIA::IFR) & 4, 4);
    EXPECT_EQ(read(v, VIA::SR), 0x53);
    EXPECT_EQ(read(v, VIA::IFR) & 4, 0);
    v.tick(16); EXPECT_EQ(read(v, VIA::IFR) & 4, 0);
    v.tick(); EXPECT_EQ(read(v, VIA::IFR) & 4, 4);
    EXPECT_EQ(read(v, VIA::SR), 0x53);
}

TEST(ViaSerial, ExternalClockCannotOverrideInternalClockAndCb2InputFlagsAreSuppressed) {
    VIA v;
    write(v, VIA::ACR, 0x18); write(v, VIA::SR, 0);
    v.setCB1(false); v.setCB2(false);
    EXPECT_TRUE(v.cb1Level()); EXPECT_EQ(read(v, VIA::IFR), 0);
    v.tick(17); EXPECT_EQ(read(v, VIA::IFR), 4);
}

TEST(ViaReset, ResetPreservesHeldPortLevelsAndT2LowLatch) {
    VIA v;
    write(v, VIA::DDRA, 0xff); write(v, VIA::ORA, 0xa5);
    write(v, VIA::DDRB, 0xff); write(v, VIA::ORB, 0x5a);
    write(v, VIA::T2CL, 0x67); v.reset();
    EXPECT_EQ(v.portARead(), 0xa5); EXPECT_EQ(v.portBRead(), 0x5a);
    write(v, VIA::T2CH, 0x12); EXPECT_EQ(count2(v), 0x1267);
}

TEST(ViaIntegration, LcdDataBusIsNotOverwrittenWhenVIAReleasesIt) {
    LCD lcd;
    PortBus bus(lcd);
    lcd.portBWrite(0xaa);
    bus.portBWrite(0x50, 0xf0);
    EXPECT_EQ(lcd.portBRead(), 0x5a);
    bus.portBWrite(0, 0);
    EXPECT_EQ(lcd.portBRead(), 0x5a);
}

TEST(ViaIntegration, ActiveViaKeepsConfiguredAverageClockRate) {
    using Clock = std::chrono::steady_clock;
    System s{0, 0x3fff, 0x6000, 0x7fff, 0x8000, 0xffff};
    s.ram[0x200] = 0x4c; s.ram[0x201] = 0; s.ram[0x202] = 2; // JMP $0200
    write(s.registers, VIA::ACR, 0xc0); timer1(s.registers, 1000);
    s.cpu.start(); s.cpu.reset(0x200);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    auto sample = [&] {
        std::lock_guard<std::mutex> lock(s.cpu.stateMutex());
        return std::make_pair(Clock::now(), s.cpu.cycles.getCycles());
    };
    const auto start = sample();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    const auto end = sample();
    const double hz = (end.second - start.second) / std::chrono::duration<double>(end.first - start.first).count();
    RecordProperty("measured_hz_with_via", std::to_string(hz));
    EXPECT_NEAR(hz, 1000000, 20000);
}
