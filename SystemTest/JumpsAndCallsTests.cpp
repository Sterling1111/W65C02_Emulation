#include "gtest/gtest.h"
#include "System.h"

class JumpsAndCallsTests : public testing::Test {
public:
    System system{0x0000, 0xFFFF, -1, -1, -1, -1, .1};
    _65C02& cpu = system.cpu;
    RAM& ram = system.ram;
    virtual void SetUp() {
        cpu.reset();
        cpu.PC = 0xFFFC;
    }
    virtual void TearDown() {}
};

TEST_F(JumpsAndCallsTests, JSRCanJumpToSubroutine) {
    ram[0xFFFC] = _65C02::INS_JSR;     //6 cycles
    ram[0xFFFD] = 0x00;
    ram[0xFFFE] = 0x80;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 6;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC, 0x8000);
    EXPECT_EQ(cpu.SP, 0xFD);
    EXPECT_EQ(ram[0x1FF], 0xFF);
    EXPECT_EQ(ram[0x1FE], 0xFE);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(JumpsAndCallsTests, RTSCanReturnFromSubroutine) {
    cpu.PC = 0x8000;
    ram[0x1FF] = 0xFF;
    ram[0x1FE] = 0x02;
    cpu.SP = 0xFD;
    ram[0x8000] = _65C02::INS_RTS;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 6;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.SP, 0xFF);
    EXPECT_EQ(cpu.PC, 0xFF03);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(JumpsAndCallsTests, JSRAndRTSCanJumpToAndReturnFromSubroutine) {
    ram[0xFFFC] = _65C02::INS_JSR;     //6 cycles
    ram[0xFFFD] = 0x00;
    ram[0xFFFE] = 0x80;
    ram[0x8000] = _65C02::INS_LDA_IM;  //2 cycles
    ram[0x8001] = 0x42;
    ram[0x8002] = _65C02::INS_RTS;     //6 cycles
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 14;
    constexpr byte INSTRUCTIONS = 3;
    cpu.execute(INSTRUCTIONS);

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cpu.PC, 0xFFFF);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(JumpsAndCallsTests, JMPAbsoluteCanJumpToANewAddress) {
    ram[0xFFFC] = _65C02::INS_JMP_ABS;
    ram[0xFFFD] = 0x00;
    ram[0xFFFE] = 0x80;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC, 0x8000);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(JumpsAndCallsTests, JMPIndirectCanJumpToANewAddress) {
    ram[0xFFFC] = _65C02::INS_JMP_IND;
    ram[0xFFFD] = 0x20;
    ram[0xFFFE] = 0x01;
    ram[0x0120] = 0xFC;
    ram[0x0121] = 0xBA;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 5;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC, 0xBAFC);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(JumpsAndCallsTests, JMPIndirectCanJumpToANewAddressOnPageBoundery) {
    ram[0xFFFC] = _65C02::INS_JMP_IND;
    ram[0xFFFD] = 0xFF;
    ram[0xFFFE] = 0x01;
    ram[0x01FF] = 0xFC;
    ram[0x0100] = 0xBA;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 5;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC, 0xBAFC);
    EXPECT_EQ(cpu.PS, psCopy);
}

