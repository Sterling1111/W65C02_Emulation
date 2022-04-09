#include "gtest/gtest.h"
#include "System.h"

static void VerifyUnmodifiedCPUFlagsFromLogicalOperation(const std::bitset<W65C02::StatusFlags::numFlags>& ps, const std::bitset<W65C02::StatusFlags::numFlags>& psCopy, bool bit) {
    EXPECT_EQ(ps.test(W65C02::StatusFlags::C), psCopy.test(W65C02::StatusFlags::C));
    EXPECT_EQ(ps.test(W65C02::StatusFlags::I), psCopy.test(W65C02::StatusFlags::I));
    EXPECT_EQ(ps.test(W65C02::StatusFlags::D), psCopy.test(W65C02::StatusFlags::D));
    EXPECT_EQ(ps.test(W65C02::StatusFlags::B), psCopy.test(W65C02::StatusFlags::B));
    if(!bit)
        EXPECT_EQ(ps.test(W65C02::StatusFlags::V), psCopy.test(W65C02::StatusFlags::V));
}

class Logical_Im_Abs_ZP : public testing::TestWithParam<std::tuple<byte, byte>> {
    System system{0x0000, 0xFFFF, -1, -1, -1, -1, .1};
    W65C02& cpu = system.cpu;
    RAM& ram = system.ram;

    virtual void SetUp() {
        cpu.reset();
        cpu.PC = 0xFFFC;
    }
    virtual void TearDown() {}
public:
    void TestLogicalOperationImmediate(byte, byte, byte, const std::function<byte(byte, byte)>&);
    void TestLogicalOperationAbsolute(byte, byte, byte, const std::function<byte(byte, byte)>&);
    void TestLogicalOperationZeroPage(byte, byte, byte, const std::function<byte(byte, byte)>&);
};

class Logical_IndY_XInd : public testing::TestWithParam<std::tuple<byte, byte, word, byte, byte>> {
    System system{0x0000, 0xFFFF, -1, -1, -1, -1, .1};
    W65C02& cpu = system.cpu;
    RAM& ram = system.ram;

    virtual void SetUp() {
        cpu.reset();
        cpu.PC = 0xFFFC;
    }
    virtual void TearDown() {}
public:
    void TestLogicalOperationIndirectIndexed(byte, byte, byte, word, byte, byte, const std::function<byte(byte, byte)>&);
    void TestLogicalOperationIndexedIndirect(byte opcode, byte xVal, byte zpAddr, word zpValue, byte, byte memVal, const std::function<byte(byte, byte)>&);
};

class Logical_ZPX : public testing::TestWithParam<std::tuple<byte, byte, byte, byte>> {
    System system{0x0000, 0xFFFF, -1, -1, -1, -1, .1};
    W65C02& cpu = system.cpu;
    RAM& ram = system.ram;

    virtual void SetUp() {
        cpu.reset();
        cpu.PC = 0xFFFC;
    }
    virtual void TearDown() {}

public:
    void TestLogicalOperationZeroPageX(byte, byte, word, byte, byte, const std::function<byte(byte, byte)>&);
};

class Logical_AbsX_AbsY : public testing::TestWithParam<std::tuple<byte, byte, byte, byte>> {
    System system{0x0000, 0xFFFF, -1, -1, -1, -1, .1};
    W65C02& cpu = system.cpu;
    RAM& ram = system.ram;

    virtual void SetUp() {
        cpu.reset();
        cpu.PC = 0xFFFC;
    }
    virtual void TearDown() {}

public:
    void TestLogicalOperationAbsoluteX(byte, byte, word, byte, byte, const std::function<byte(byte, byte)>&);
    void TestLogicalOperationAbsoluteY(byte, byte, word, byte, byte, const std::function<byte(byte, byte)>&);
};

class Logical_ZPInd : public testing::TestWithParam<std::tuple<byte, word, byte, byte>> {
    System system{0x0000, 0xFFFF, -1, -1, -1, -1, .1};
    W65C02& cpu = system.cpu;
    RAM& ram = system.ram;

    virtual void SetUp() {
        cpu.reset();
        cpu.PC = 0xFFFC;
    }
    virtual void TearDown() {}

public:
    void TestLogicalOperationZeroPageIndirect(byte, byte, word, byte, byte, const std::function<byte(byte, byte)>&);
};

//for immediate mode bit instruction only sets the zero flag and does nothing else
void Logical_Im_Abs_ZP::TestLogicalOperationImmediate(byte opcode, byte aVal, byte memVal, const std::function<byte(byte, byte)> &op) {
    bool bit = opcode == 0x89;
    cpu.PS.set(W65C02::StatusFlags::Z, (bool)op(aVal, memVal));
    if(!bit) {
        cpu.PS.set(W65C02::StatusFlags::N, !(op(aVal, memVal) & 0x80));
    }
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = memVal;
    cpu.A = aVal;
    auto psCopy = cpu.PS;
    auto PCCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 2;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - PCCopy, EXPECTED_BYTES);
    if(!bit) {
        EXPECT_EQ(cpu.A, op(aVal, memVal));
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool)(op(aVal, memVal) & 0x80));
    } else {
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), psCopy.test(W65C02::StatusFlags::N));
    }
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::Z), !op(aVal, memVal));
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy, bit);
}

void Logical_Im_Abs_ZP::TestLogicalOperationAbsolute(byte opcode, byte aVal, byte memVal, const std::function<byte(byte, byte)> &op) {
    bool bit = opcode == 0x2c;
    cpu.PS.set(W65C02::StatusFlags::Z, (bool)op(aVal, memVal));
    if(bit) {
        cpu.PS.set(W65C02::StatusFlags::N, !(memVal & 0x80));
        cpu.PS.set(W65C02::StatusFlags::V, !(memVal & 0x40));
    } else {
        cpu.PS.set(W65C02::StatusFlags::N, !(op(aVal, memVal) & 0x80));
    }
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = 0x00;
    ram[0xFFFE] = 0x90;
    ram[0x9000] = memVal;
    cpu.A = aVal;
    auto psCopy = cpu.PS;
    auto PCCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 4;
    constexpr dword EXPECTED_BYTES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - PCCopy, EXPECTED_BYTES);
    if(!bit) {
        EXPECT_EQ(cpu.A, op(aVal, memVal));
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool)(op(aVal, memVal) & 0x80));
    } else {
         EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::V), (bool) (memVal & 0x40));
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool) (memVal & 0x80));
    }
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::Z), !op(aVal, memVal));
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy, bit);
}

void Logical_Im_Abs_ZP::TestLogicalOperationZeroPage(byte opcode, byte aVal, byte memVal, const std::function<byte(byte, byte)> &op) {
    bool bit = opcode == 0x24;
    cpu.PS.set(W65C02::StatusFlags::Z, (bool)op(aVal, memVal));
    if(bit) {
        cpu.PS.set(W65C02::StatusFlags::N, !(memVal & 0x80));
        cpu.PS.set(W65C02::StatusFlags::V, !(memVal & 0x40));
    } else {
        cpu.PS.set(W65C02::StatusFlags::N, !(op(aVal, memVal) & 0x80));
    }
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = 0x42;
    ram[0x0042] = memVal;
    cpu.A = aVal;
    auto psCopy = cpu.PS;
    auto PCCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 3;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - PCCopy, EXPECTED_BYTES);
    if(!bit) {
        EXPECT_EQ(cpu.A, op(aVal, memVal));
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool)(op(aVal, memVal) & 0x80));
    } else {
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::V), (bool) (memVal & 0x40));
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool) (memVal & 0x80));
    }
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::Z), !op(aVal, memVal));
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy, bit);
}

void Logical_IndY_XInd::TestLogicalOperationIndirectIndexed(byte opcode, byte yVAl, byte zpAddr, word zpVal, byte aVal, byte memVal, const std::function<byte(byte, byte)>& op) {
    word valAddress = zpVal + yVAl;
    cpu.PS.set(W65C02::StatusFlags::Z, (bool)op(aVal, memVal));
    cpu.PS.set(W65C02::StatusFlags::N, !(op(aVal, memVal) & 0x80));
    cpu.Y = yVAl;
    cpu.A = aVal;
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = zpAddr;
    ram[zpAddr] = (zpVal & 0xFF);         //low byte
    ram[(byte)(zpAddr + 1)] = zpVal >> 8;   //high byte
    ram[zpVal + yVAl] = memVal;
    auto psCopy = cpu.PS;
    auto PCCopy = cpu.PC;
    dword EXPECTED_CYCLES = 5 + (((zpVal & 0xFF) + yVAl) > 0xFF);
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - PCCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.A, op(aVal, memVal));
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool)(op(aVal, memVal) & 0x80));
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::Z), !op(aVal, memVal));
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy, false);
}

void Logical_IndY_XInd::TestLogicalOperationIndexedIndirect(byte opcode, byte xVal, byte zpAddr, word zpVal, byte aVal, byte memVal, const std::function<byte(byte, byte)>& op) {
    cpu.PS.set(W65C02::StatusFlags::Z, (bool)op(aVal, memVal));
    cpu.PS.set(W65C02::StatusFlags::N, !(op(aVal, memVal) & 0x80));
    cpu.X = xVal;
    cpu.A = aVal;
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = zpAddr;
    ram[(byte)(zpAddr + xVal)] = (zpVal & 0xFF);
    ram[(byte)(zpAddr + xVal + 1)] = zpVal >> 8;
    ram[zpVal] = memVal;
    auto psCopy = cpu.PS;
    auto PCCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 6;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - PCCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.A, op(aVal, memVal));
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool)(op(aVal, memVal) & 0x80));
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::Z), !op(aVal, memVal));
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy, false);
}

void Logical_ZPX::TestLogicalOperationZeroPageX(byte opcode, byte xVal, word zpAddr, byte aVal, byte memVal, const std::function<byte(byte, byte)>& op) {
    bool bit = opcode == 0x34;
    cpu.PS.set(W65C02::StatusFlags::Z, (bool)op(aVal, memVal));
    if(bit) {
        cpu.PS.set(W65C02::StatusFlags::N, !(memVal & 0x80));
        cpu.PS.set(W65C02::StatusFlags::V, !(memVal & 0x40));
    } else {
        cpu.PS.set(W65C02::StatusFlags::N, !(op(aVal, memVal) & 0x80));
    }
    cpu.X = xVal;
    cpu.A = aVal;
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = zpAddr;
    ram[(byte)(zpAddr + xVal)] = memVal;
    auto psCopy = cpu.PS;
    auto PCCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 4;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - PCCopy, EXPECTED_BYTES);
    if(!bit) {
        EXPECT_EQ(cpu.A, op(aVal, memVal));
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool)(op(aVal, memVal) & 0x80));
    } else {
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::V), (bool) (memVal & 0x40));
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool) (memVal & 0x80));
    }
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::Z), !op(aVal, memVal));
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy, bit);
}

void Logical_AbsX_AbsY::TestLogicalOperationAbsoluteX(byte opcode, byte xVal, word addr, byte aVal, byte memVal, const std::function<byte(byte, byte)>& op) {
    bool bit = opcode == 0x3c;
    cpu.PS.set(W65C02::StatusFlags::Z, (bool)op(aVal, memVal));
    if(bit) {
        cpu.PS.set(W65C02::StatusFlags::N, !(memVal & 0x80));
        cpu.PS.set(W65C02::StatusFlags::V, !(memVal & 0x40));
    } else {
        cpu.PS.set(W65C02::StatusFlags::N, !(op(aVal, memVal) & 0x80));
    }
    cpu.X = xVal;
    cpu.A = aVal;
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = addr & 0xFF;
    ram[0xFFFE] = addr >> 8;
    ram[addr + xVal] = memVal;
    auto psCopy = cpu.PS;
    auto PCCopy = cpu.PC;
    dword EXPECTED_CYCLES = 4 + ((addr & 0xFF) + xVal > 0xFF);
    constexpr dword EXPECTED_BYTES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - PCCopy, EXPECTED_BYTES);
    if(!bit) {
        EXPECT_EQ(cpu.A, op(aVal, memVal));
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool)(op(aVal, memVal) & 0x80));
    } else {
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::V), (bool) (memVal & 0x40));
        EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool) (memVal & 0x80));
    }
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::Z), !op(aVal, memVal));
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy, bit);
}

void Logical_AbsX_AbsY::TestLogicalOperationAbsoluteY(byte opcode, byte yVal, word addr, byte aVal, byte memVal, const std::function<byte(byte, byte)>& op) {
    cpu.PS.set(W65C02::StatusFlags::Z, (bool)op(aVal, memVal));
    cpu.PS.set(W65C02::StatusFlags::N, !(op(aVal, memVal) & 0x80));
    cpu.Y = yVal;
    cpu.A = aVal;
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = addr & 0xFF;
    ram[0xFFFE] = addr >> 8;
    ram[addr + yVal] = memVal;
    auto psCopy = cpu.PS;
    auto PCCopy = cpu.PC;
    dword EXPECTED_CYCLES = 4 + ((addr & 0xFF) + yVal > 0xFF);
    constexpr dword EXPECTED_BYTES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - PCCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.A, op(aVal, memVal));
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool)(op(aVal, memVal) & 0x80));
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::Z), !op(aVal, memVal));
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy, false);
}

void Logical_ZPInd::TestLogicalOperationZeroPageIndirect(byte opcode, byte zpAddr, word zpVal, byte aVal, byte memVal, const std::function<byte(byte, byte)>& op) {
    cpu.PS.set(W65C02::StatusFlags::Z, (bool)op(aVal, memVal));
    cpu.PS.set(W65C02::StatusFlags::N, !(op(aVal, memVal) & 0x80));
    cpu.A = aVal;
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = zpAddr;
    ram[zpAddr] = zpVal & 0xFF;
    ram[(byte)(zpAddr + 1)] = zpVal >> 8;
    ram[zpVal] = memVal;

    auto psCopy = cpu.PS;
    auto PCCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 5;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - PCCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.A, op(aVal, memVal));
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::N), (bool)(op(aVal, memVal) & 0x80));
    EXPECT_EQ(cpu.PS.test(W65C02::StatusFlags::Z), !op(aVal, memVal));
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy, false);
}

TEST_P(Logical_Im_Abs_ZP, ANDImmediate) {
    TestLogicalOperationImmediate(W65C02::INS_AND_IM, std::get<0>(GetParam()), std::get<1>(GetParam()),std::bit_and<>());
}

TEST_P(Logical_Im_Abs_ZP, EORImmediate) {
    TestLogicalOperationImmediate(W65C02::INS_EOR_IM, std::get<0>(GetParam()), std::get<1>(GetParam()),std::bit_xor<>());
}

TEST_P(Logical_Im_Abs_ZP, ORAImmediate) {
    TestLogicalOperationImmediate(W65C02::INS_ORA_IM, std::get<0>(GetParam()), std::get<1>(GetParam()),std::bit_or<>());
}

TEST_P(Logical_Im_Abs_ZP, BITImmediate) {
    TestLogicalOperationImmediate(W65C02::INS_BIT_IM, std::get<0>(GetParam()), std::get<1>(GetParam()),std::bit_and<>());
}

TEST_P(Logical_Im_Abs_ZP, ANDAbsolute) {
    TestLogicalOperationAbsolute(W65C02::INS_AND_ABS, std::get<0>(GetParam()), std::get<1>(GetParam()),std::bit_and<>());
}

TEST_P(Logical_Im_Abs_ZP, EORAbsolute) {
    TestLogicalOperationAbsolute(W65C02::INS_EOR_ABS, std::get<0>(GetParam()), std::get<1>(GetParam()),std::bit_xor<>());
}

TEST_P(Logical_Im_Abs_ZP, ORAAbsolute) {
    TestLogicalOperationAbsolute(W65C02::INS_ORA_ABS, std::get<0>(GetParam()), std::get<1>(GetParam()),std::bit_or<>());
}

TEST_P(Logical_Im_Abs_ZP, BITAbsolute) {
    TestLogicalOperationAbsolute(W65C02::INS_BIT_ABS, std::get<0>(GetParam()), std::get<1>(GetParam()),std::bit_and<>());
}

TEST_P(Logical_Im_Abs_ZP, ANDZeroPage) {
    TestLogicalOperationZeroPage(W65C02::INS_AND_ZP, std::get<0>(GetParam()), std::get<1>(GetParam()),std::bit_and<>());
}

TEST_P(Logical_Im_Abs_ZP, EORZeroPage) {
    TestLogicalOperationZeroPage(W65C02::INS_EOR_ZP, std::get<0>(GetParam()), std::get<1>(GetParam()),std::bit_xor<>());
}

TEST_P(Logical_Im_Abs_ZP, ORAZeroPage) {
    TestLogicalOperationZeroPage(W65C02::INS_ORA_ZP, std::get<0>(GetParam()), std::get<1>(GetParam()),std::bit_or<>());
}

TEST_P(Logical_Im_Abs_ZP, BITZeroPage) {
    TestLogicalOperationZeroPage(W65C02::INS_BIT_ZP, std::get<0>(GetParam()), std::get<1>(GetParam()),std::bit_and<>());
}

TEST_P(Logical_IndY_XInd, ANDIndirecY) {
    TestLogicalOperationIndirectIndexed(W65C02::INS_AND_INDY, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::get<4>(GetParam()), std::bit_and<>());
}

TEST_P(Logical_IndY_XInd, EORIndirectY) {
    TestLogicalOperationIndirectIndexed(W65C02::INS_EOR_INDY, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::get<4>(GetParam()), std::bit_xor<>());
}

TEST_P(Logical_IndY_XInd, ORAIndirectY) {
    TestLogicalOperationIndirectIndexed(W65C02::INS_ORA_INDY, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::get<4>(GetParam()), std::bit_or<>());
}

TEST_P(Logical_IndY_XInd, ANDXIndirect) {
    TestLogicalOperationIndexedIndirect(W65C02::INS_AND_XIND, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::get<4>(GetParam()), std::bit_and<>());
}

TEST_P(Logical_IndY_XInd, EORXIndirect) {
    TestLogicalOperationIndexedIndirect(W65C02::INS_EOR_XIND, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::get<4>(GetParam()), std::bit_xor<>());
}

TEST_P(Logical_IndY_XInd, ORAXIndirect) {
    TestLogicalOperationIndexedIndirect(W65C02::INS_ORA_XIND, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::get<4>(GetParam()), std::bit_or<>());
}

TEST_P(Logical_ZPX, ANDZeroPageX) {
    TestLogicalOperationZeroPageX(W65C02::INS_AND_ZPX, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_and<>());
}

TEST_P(Logical_ZPX, EORZeroPageX) {
    TestLogicalOperationZeroPageX(W65C02::INS_EOR_ZPX, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_xor<>());
}

TEST_P(Logical_ZPX, ORAZeroPageX) {
    TestLogicalOperationZeroPageX(W65C02::INS_ORA_ZPX, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_or<>());
}

TEST_P(Logical_ZPX, BITZeroPageX) {
    TestLogicalOperationZeroPageX(W65C02::INS_BIT_ZPX, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_and<>());
}

TEST_P(Logical_AbsX_AbsY, ANDAbsoluteX) {
    TestLogicalOperationAbsoluteX(W65C02::INS_AND_ABSX, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_and<>());
}

TEST_P(Logical_AbsX_AbsY, EORAbsoluteX) {
    TestLogicalOperationAbsoluteX(W65C02::INS_EOR_ABSX, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_xor<>());
}

TEST_P(Logical_AbsX_AbsY, ORAAbsoluteX) {
    TestLogicalOperationAbsoluteX(W65C02::INS_ORA_ABSX, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_or<>());
}

TEST_P(Logical_AbsX_AbsY, BITAbsoluteX) {
    TestLogicalOperationAbsoluteX(W65C02::INS_BIT_ABSX, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_and<>());
}

TEST_P(Logical_AbsX_AbsY, ANDAbsoluteY) {
    TestLogicalOperationAbsoluteY(W65C02::INS_AND_ABSY, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_and<>());
}

TEST_P(Logical_AbsX_AbsY, EORAbsoluteY) {
    TestLogicalOperationAbsoluteY(W65C02::INS_EOR_ABSY, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_xor<>());
}

TEST_P(Logical_AbsX_AbsY, ORAAbsoluteY) {
    TestLogicalOperationAbsoluteY(W65C02::INS_ORA_ABSY, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_or<>());
}

TEST_P(Logical_ZPInd, ANDZPIndirect) {
    TestLogicalOperationZeroPageIndirect(W65C02::INS_AND_ZPIND, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_and<>());
}

TEST_P(Logical_ZPInd, EORZPIndirect) {
    TestLogicalOperationZeroPageIndirect(W65C02::INS_EOR_ZPIND, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_xor<>());
}

TEST_P(Logical_ZPInd, ORAZPIndirect) {
    TestLogicalOperationZeroPageIndirect(W65C02::INS_ORA_ZPIND, std::get<0>(GetParam()), std::get<1>(GetParam()), std::get<2>(GetParam()), std::get<3>(GetParam()), std::bit_or<>());
}

INSTANTIATE_TEST_SUITE_P(AVal_Val_Params, Logical_Im_Abs_ZP,
                         testing::Values(
                                 std::make_tuple(0x00, 0x00),
                                 std::make_tuple(0x00, 0xff),
                                 std::make_tuple(0x0f, 0xf0),
                                 std::make_tuple(0xf0, 0x0f),
                                 std::make_tuple(0xff, 0xff)
                                 ));

INSTANTIATE_TEST_SUITE_P(regVal_ZPAddr_ZPVal_aVal_memVal_Params, Logical_IndY_XInd,
                         testing::Values(
                                 std::make_tuple(0x04, 0x00, 0x9000, 0x00, 0x00),
                                 std::make_tuple(0x04, 0x00, 0x9000, 0x00, 0xff),
                                 std::make_tuple(0xff, 0x00, 0x9001, 0x0f, 0xf0),
                                 std::make_tuple(0xff, 0xff, 0x4200, 0xf0, 0x0f),
                                 std::make_tuple(0xff, 0x00, 0x01FF, 0xFF, 0xff),
                                 std::make_tuple(0xff, 0xff, 0xFFFF, 0x7F, 0xff)
                                 ));

INSTANTIATE_TEST_SUITE_P(xVal_ZPAddr_aVal_memVal_Params, Logical_ZPX,
                         testing::Values(
                                 std::make_tuple(0x00, 0x00, 0x00, 0x00),
                                 std::make_tuple(0x00, 0x00, 0x00, 0xff),
                                 std::make_tuple(0x00, 0x00, 0x0f, 0xf0),
                                 std::make_tuple(0x00, 0x00, 0xf0, 0x0f),
                                 std::make_tuple(0x5, 0x42, 0xff, 0xff)
                                 ));

INSTANTIATE_TEST_SUITE_P(RegVal_Addr_aVal_memVal_Params, Logical_AbsX_AbsY,
                         testing::Values(
                                 std::make_tuple(0x00, 0x4480, 0x00, 0x00),
                                 std::make_tuple(0x00, 0x4480, 0x00, 0xff),
                                 std::make_tuple(0x00, 0x4480, 0x0f, 0xf0),
                                 std::make_tuple(0x00, 0x4480, 0xf0, 0x0f),
                                 std::make_tuple(0x00, 0x4480, 0xFF, 0xff),
                                 std::make_tuple(0xFF, 0x4480, 0x00, 0xff)
                                 ));

INSTANTIATE_TEST_SUITE_P(ZPAddr_ZPVal_aVal_memVal_Params, Logical_ZPInd,
                         testing::Values(
                                 std::make_tuple(0x00, 0x9000, 0x00, 0x00),
                                 std::make_tuple(0x00, 0x9000, 0x00, 0xff),
                                 std::make_tuple(0x00, 0x9000, 0xf0, 0x0f),
                                 std::make_tuple(0x00, 0x9000, 0xff, 0xff),
                                 std::make_tuple(0xFF, 0x9000, 0x0f, 0xf0)
                                 ));

TEST(ProgramLoggingOutput, LogicalOperationsLogging) {
    System system{0x0000, 0x3FFF, 0x6000, 0x7FFF, 0x8000, 0xFFFF, .001};
    system.executeProgram("EmulationOutFiles//emulation_logical_operations.out", 643, true,
                          "EmulationLogFiles//emulation_logical_operations.txt");
    std::ifstream emulation_logging("EmulationLogFiles//emulation_logical_operations.txt"),
    _65C02_logging("65C02LogFiles//65C02_logical_operations.txt");
    std::stringstream emulation_buffer, _65C02_buffer;
    emulation_buffer << emulation_logging.rdbuf();
    _65C02_buffer << _65C02_logging.rdbuf();
    EXPECT_STREQ(_65C02_buffer.str().c_str(), emulation_buffer.str().c_str());
}