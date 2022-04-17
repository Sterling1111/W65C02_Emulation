#include "gtest/gtest.h"
#include "System.h"

static void VerifyUnmodifiedCPUFlagsFromLoadRegister(const std::bitset<W65C02::StatusFlags::numFlags>& ps, const std::bitset<W65C02::StatusFlags::numFlags>& psCopy) {
    EXPECT_EQ(ps.test(W65C02::StatusFlags::C), psCopy.test(W65C02::StatusFlags::C));
    EXPECT_EQ(ps.test(W65C02::StatusFlags::I), psCopy.test(W65C02::StatusFlags::I));
    EXPECT_EQ(ps.test(W65C02::StatusFlags::D), psCopy.test(W65C02::StatusFlags::D));
    EXPECT_EQ(ps.test(W65C02::StatusFlags::B), psCopy.test(W65C02::StatusFlags::B));
    EXPECT_EQ(ps.test(W65C02::StatusFlags::V), psCopy.test(W65C02::StatusFlags::V));
}

class RegisterTransferTests : public testing::TestWithParam<word> {
    System system{0x0000, 0xFFFF, -1, -1, -1, -1, .1};
    W65C02& cpu = system.cpu;
    RAM& ram = system.ram;

    virtual void SetUp() {
        cpu.reset();
        cpu.PC = 0xFFFC;
    }
    virtual void TearDown() {}
public:
    void TestTransferRegisterImplied(byte opcode, byte W65C02::* RegisterTo, byte W65C02::* RegisterFrom, byte value);
};

void RegisterTransferTests::TestTransferRegisterImplied(byte opcode, byte W65C02::* RegisterTo, byte W65C02::* RegisterFrom, byte value) {
    cpu.PS.set(W65C02::StatusFlags::Z, !value);
    cpu.PS.set(W65C02::StatusFlags::N, !(value & 0x80));

    cpu.*RegisterFrom = value;
    ram[0xFFFC] = opcode;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 2;
    constexpr dword EXPECTED_BYTES = 1;
    cpu.execute();
    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.*RegisterTo, cpu.*RegisterFrom);
    cpu.PS.set(W65C02::StatusFlags::Z, (bool)(value));
    cpu.PS.set(W65C02::StatusFlags::N, (bool)(value & 0x80));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_P(RegisterTransferTests, TAXImplied) {
    TestTransferRegisterImplied(W65C02::INS_TAX_IMP, &W65C02::X, &W65C02::A, GetParam());
}

TEST_P(RegisterTransferTests, TAYImplied) {
    TestTransferRegisterImplied(W65C02::INS_TAY_IMP, &W65C02::Y, &W65C02::A, GetParam());
}

TEST_P(RegisterTransferTests, TXAImplied) {
    TestTransferRegisterImplied(W65C02::INS_TXA_IMP, &W65C02::A, &W65C02::X, GetParam());
}

TEST_P(RegisterTransferTests, TYAImplied) {
    TestTransferRegisterImplied(W65C02::INS_TYA_IMP, &W65C02::A, &W65C02::Y, GetParam());
}

INSTANTIATE_TEST_SUITE_P(regTo_regFrom, RegisterTransferTests,
                         testing::Values(0x00, 0x7F, 0xFF));
