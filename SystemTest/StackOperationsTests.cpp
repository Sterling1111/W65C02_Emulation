#include "gtest/gtest.h"
#include "System.h"

class StackOperationsTests : public testing::Test {
public:
    System system{0x0000, 0xFFFF, -1, -1, -1, -1, .1};
    W65C02& cpu = system.cpu;
    RAM& ram = system.ram;
    virtual void SetUp() {
        cpu.reset();
        cpu.PC = 0xFFFC;
        cpu.SP = 0xFE;
    }
    virtual void TearDown() {}
};

static void VerifyUnmodifiedCPUFlagsFromLoadRegister(const std::bitset<W65C02::StatusFlags::numFlags>& ps, const std::bitset<W65C02::StatusFlags::numFlags>& psCopy) {
    EXPECT_EQ(ps.test(W65C02::StatusFlags::C), psCopy.test(W65C02::StatusFlags::C));
    EXPECT_EQ(ps.test(W65C02::StatusFlags::I), psCopy.test(W65C02::StatusFlags::I));
    EXPECT_EQ(ps.test(W65C02::StatusFlags::D), psCopy.test(W65C02::StatusFlags::D));
    EXPECT_EQ(ps.test(W65C02::StatusFlags::B), psCopy.test(W65C02::StatusFlags::B));
    EXPECT_EQ(ps.test(W65C02::StatusFlags::U), psCopy.test(W65C02::StatusFlags::U));
    EXPECT_EQ(ps.test(W65C02::StatusFlags::V), psCopy.test(W65C02::StatusFlags::V));
}

TEST_F(StackOperationsTests, TXSCanTransferXToStackPointer) {
    cpu.X = 0x42;
    ram[0xFFFC] = W65C02::INS_TXS_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.SP, cpu.X);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StackOperationsTests, TSXCanTransferXToStackPointer) {
    cpu.SP = 0x42;
    ram[0xFFFC] = W65C02::INS_TSX_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.SP, cpu.X);
    EXPECT_FALSE(cpu.PS.test(W65C02::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(W65C02::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(StackOperationsTests, TSXCanSetTheZeroFlag) {
    cpu.SP = 0x00;
    ram[0xFFFC] = W65C02::INS_TSX_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.SP, cpu.X);
    EXPECT_TRUE(cpu.PS.test(W65C02::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(W65C02::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(StackOperationsTests, TSXCanSetTheNegativeFlag) {
    cpu.SP = 0b10000000;
    ram[0xFFFC] = W65C02::INS_TSX_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.SP, cpu.X);
    EXPECT_FALSE(cpu.PS.test(W65C02::StatusFlags::Z));
    EXPECT_TRUE(cpu.PS.test(W65C02::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(StackOperationsTests, PHACanPushAccumulatorOnStack) {
    ram[0xFFFC] = W65C02::INS_PHA_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(ram[0x01FE], cpu.A);
    EXPECT_EQ(cpu.SP, 0x00FD);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StackOperationsTests, PHPCanPushPSOnStack) {
    ram[0xFFFC] = W65C02::INS_PHP_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(ram[0x01FE], static_cast<byte>(cpu.PS.to_ulong()));
    EXPECT_EQ(cpu.SP, 0x00FD);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StackOperationsTests, PHXCanPushXRegisterOnStack) {
    ram[0xFFFC] = W65C02::INS_PHX_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(ram[0x01FE], cpu.X);
    EXPECT_EQ(cpu.SP, 0x00FD);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StackOperationsTests, PHYCanPushYRegisterOnStack) {
    ram[0xFFFC] = W65C02::INS_PHY_IMP;
    ram[0x01FE] = cpu.Y + 1;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(ram[0x01FE], cpu.Y);
    EXPECT_EQ(cpu.SP, 0x00FD);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StackOperationsTests, PLACanPullAccumulatorFromStack) {
    ram[0x01FF] = 0x42;
    ram[0xFFFC] = W65C02::INS_PLA_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 4;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cpu.SP, 0x00FF);
    EXPECT_FALSE(cpu.PS.test(W65C02::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(W65C02::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(StackOperationsTests, PLACanSetTheZeroFlag) {
    ram[0x01FF] = 0x00;
    ram[0xFFFC] = W65C02::INS_PLA_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 4;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_EQ(cpu.SP, 0x00FF);
    EXPECT_TRUE(cpu.PS.test(W65C02::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(W65C02::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(StackOperationsTests, PLPCanPullPSFromStack) {
    ram[0x01FF] = 0x42;
    ram[0xFFFC] = W65C02::INS_PLP_IMP;
    constexpr byte EXPECTED_CYCLES = 4;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(static_cast<byte>(cpu.PS.to_ulong()), 0x42);
    EXPECT_EQ(cpu.SP, 0x00FF);
}

TEST_F(StackOperationsTests, PLPCanSetTheNegativeFlag) {
    ram[0x01FF] = 0b10000000;
    ram[0xFFFC] = W65C02::INS_PLA_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 4;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0b10000000);
    EXPECT_EQ(cpu.SP, 0x00FF);
    EXPECT_FALSE(cpu.PS.test(W65C02::StatusFlags::Z));
    EXPECT_TRUE(cpu.PS.test(W65C02::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(StackOperationsTests, PLPSetsStatusFlagsFromStack) {
    bool diff{false};
    for (byte i{0}; i < 0xFF; ++i) {
        ram[0x01FF] = i;
        ram[0xFFFC] = W65C02::INS_PLP_IMP;
        cpu.execute();
        //only non-zero if they are different
        diff |= static_cast<byte>(cpu.PS.to_ulong()) xor i;
        cpu.reset(0xFFFC);
        cpu.SP = 0xFE;
    }
    EXPECT_FALSE(diff);
}

TEST_F(StackOperationsTests, PLXCanPullXRegisterFromStack) {
    ram[0x01FF] = 0x42;
    ram[0xFFFC] = W65C02::INS_PLX_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 4;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.X, 0x42);
    EXPECT_EQ(cpu.SP, 0x00FF);
    EXPECT_FALSE(cpu.PS.test(W65C02::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(W65C02::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(StackOperationsTests, PLYCanPullYRegisterFromStack) {
    ram[0x01FF] = 0x42;
    ram[0xFFFC] = W65C02::INS_PLY_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 4;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.Y, 0x42);
    EXPECT_EQ(cpu.SP, 0x00FF);
    EXPECT_FALSE(cpu.PS.test(W65C02::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(W65C02::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST(ProgramLoggingOutput, StackOperationsLogging) {
    System system{0x0000, 0x3FFF, 0x6000, 0x7FFF, 0x8000, 0xFFFF, .001};
    system.executeProgram("EmulationOutFiles//emulation_stack_operations.out", 43, true,
                          "EmulationLogFiles//emulation_stack_operations.txt");
    std::ifstream emulation_logging("EmulationLogFiles//emulation_stack_operations.txt"),
    _65C02_logging("65C02LogFiles//65C02_stack_operations.txt");
    std::stringstream emulation_buffer, _65C02_buffer;
    emulation_buffer << emulation_logging.rdbuf();
    _65C02_buffer << _65C02_logging.rdbuf();
    EXPECT_STREQ(_65C02_buffer.str().c_str(), emulation_buffer.str().c_str());
}
