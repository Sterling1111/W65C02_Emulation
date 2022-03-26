#include "gtest/gtest.h"
#include "System.h"

class StoreRegisterTests : public testing::Test {
public:
    System system{0x0000, 0xFFFF, -1, -1, -1, -1, .1};
    _65C02& cpu = system.cpu;
    RAM& ram = system.ram;
    virtual void SetUp() {
        cpu.reset();
        cpu.PC = 0xFFFC;
    }
    virtual void TearDown() {}

    void TestStoreRegisterZeroPage(byte, byte _65C02::*);
    void TestStoreRegisterZeroPageX(byte, byte _65C02::*);
    void TestStoreRegisterZeroPageXWhenItRaps(byte, byte _65C02::*);
    void TestStoreRegisterAbsolute(byte, byte _65C02::*);
};

void StoreRegisterTests::TestStoreRegisterZeroPage(byte opcode, byte _65C02::* Register) {
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = 0x42;
    ram[0x0042] = cpu.*Register + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 3;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.*Register, ram[0x0042]);
    EXPECT_EQ(cpu.PS, psCopy);
}

void StoreRegisterTests::TestStoreRegisterZeroPageX(byte opcode, byte _65C02::* Register) {
    cpu.X = 5;
    cpu.*Register = 0x69;
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = 0x42;
    ram[0x0047] = cpu.*Register + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 4;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.*Register, ram[0x0047]);
    EXPECT_EQ(cpu.PS, psCopy);
}

void StoreRegisterTests::TestStoreRegisterZeroPageXWhenItRaps(byte opcode, byte _65C02::* Register) {
    cpu.X = 0xFF;
    cpu.*Register = 0x69;
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = 0x80;
    ram[0x007F] = cpu.*Register + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 4;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.*Register, ram[0x007F]);
    EXPECT_EQ(cpu.PS, psCopy);
}

void StoreRegisterTests::TestStoreRegisterAbsolute(byte opcode, byte _65C02::* Register) {
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = 0x80;
    ram[0xFFFE] = 0x44;
    ram[0x4480] = cpu.*Register + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 4;
    constexpr dword EXPECTED_BYTES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.*Register, ram[0x4480]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StoreRegisterTests, STAZeroPageCanStoreARegisterIntoMemory) {
    TestStoreRegisterZeroPage(_65C02::INS_STA_ZP, &_65C02::A);
}

TEST_F(StoreRegisterTests, STXZeroPageCanStoreXRegisterIntoMemory) {
    TestStoreRegisterZeroPage(_65C02::INS_STX_ZP, &_65C02::X);
}

TEST_F(StoreRegisterTests, STYZeroPageCanStoreYRegisterIntoMemory) {
    TestStoreRegisterZeroPage(_65C02::INS_STY_ZP, &_65C02::Y);
}

TEST_F(StoreRegisterTests, STAZeroPageXCanStoreARegisterIntoMemory) {
    TestStoreRegisterZeroPageX(_65C02::INS_STA_ZPX, &_65C02::A);
}

TEST_F(StoreRegisterTests, STAZeroPageXCanStoreARegisterIntoMemoryWhenItRaps) {
    TestStoreRegisterZeroPageXWhenItRaps(_65C02::INS_STA_ZPX, &_65C02::A);
}

TEST_F(StoreRegisterTests, STYZeroPageXCanStoreYRegisterIntoMemory) {
    TestStoreRegisterZeroPageX(_65C02::INS_STY_ZPX, &_65C02::Y);
}

TEST_F(StoreRegisterTests, STYZeroPageXCanStoreYRegisterIntoMemoryWhenItRaps) {
    TestStoreRegisterZeroPageXWhenItRaps(_65C02::INS_STY_ZPX, &_65C02::Y);
}

TEST_F(StoreRegisterTests, STXZeroPageYCanStoreXRegisterIntoMemory) {
    cpu.Y = 5;
    ram[0xFFFC] = _65C02::INS_STX_ZPY;
    ram[0xFFFD] = 0x42;
    ram[0x0047] = cpu.X + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 4;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.X, ram[0x0047]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StoreRegisterTests, STXZeroPageYCanStoreXRegisterIntoMemoryWhenItRaps) {
    cpu.Y = 0xFF;
    ram[0xFFFC] = _65C02::INS_STX_ZPY;
    ram[0xFFFD] = 0x80;
    ram[0x007F] = cpu.X + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 5;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.X, ram[0x007F]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StoreRegisterTests, STAAbsoluteCanStoreARegisterIntoMemory) {
    TestStoreRegisterAbsolute(_65C02::INS_STA_ABS, &_65C02::A);
}

TEST_F(StoreRegisterTests, STXAbsoluteCanStoreXRegisterIntoMemory) {
    TestStoreRegisterAbsolute(_65C02::INS_STX_ABS, &_65C02::X);
}

TEST_F(StoreRegisterTests, STYAbsoluteCanStoreYRegisterIntoMemory) {
    TestStoreRegisterAbsolute(_65C02::INS_STY_ABS, &_65C02::Y);
}

TEST_F(StoreRegisterTests, STAAbsoluteXCanStoreARegisterIntoMemory) {
    cpu.X = 1;
    ram[0xFFFC] = _65C02::INS_STA_ABSX;
    ram[0xFFFD] = 0x80;
    ram[0xFFFE] = 0x44;
    ram[0x4481] = cpu.A + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 5;
    constexpr dword EXPECTED_BYTES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.A, ram[0x4481]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StoreRegisterTests, STAAbsoluteXCanStoreARegisterIntoMemoryWhenPageBounderyCrossed) {
    cpu.X = 0xFF;
    ram[0xFFFC] = _65C02::INS_STA_ABSX;
    ram[0xFFFD] = 0x02;
    ram[0xFFFE] = 0x44;
    ram[0x4401] = cpu.A + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 5;
    constexpr dword EXPECTED_BYTES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.A, ram[0x4401]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StoreRegisterTests, STAAbsoluteYCanStoreARegisterIntoMemory) {
    cpu.Y = 1;
    ram[0xFFFC] = _65C02::INS_STA_ABSY;
    ram[0xFFFD] = 0x80;
    ram[0xFFFE] = 0x44;
    ram[0x4481] = cpu.A + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 5;
    constexpr dword EXPECTED_BYTES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.A, ram[0x4481]);
    EXPECT_FALSE(cpu.PS.test(_65C02::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(_65C02::StatusFlags::N));
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StoreRegisterTests, STAAbsoluteYCanStoreARegisterIntoMemoryWhenPageBounderyCrossed) {
    cpu.X = 0xFF;
    ram[0xFFFC] = _65C02::INS_STA_ABSX;
    ram[0xFFFD] = 0x02;
    ram[0xFFFE] = 0x44;
    ram[0x4401] = cpu.A + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 5;
    constexpr dword EXPECTED_BYTES = 3;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.A, ram[0x4401]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StoreRegisterTests, STAXIndirectCanStoreARegisterIntoMemory) {
    cpu.X = 0x04;
    ram[0xFFFC] = _65C02::INS_STA_XIND;
    ram[0xFFFD] = 0x02;
    ram[0x0006] = 0x00;
    ram[0x0007] = 0x80;
    ram[0x8000] = cpu.A + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 6;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.A, ram[0x8000]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StoreRegisterTests, STAXIndirectCanStoreARegisterIntoMemoryWhenItRaps) {
    cpu.PS.set(_65C02::StatusFlags::Z, true);
    cpu.PS.set(_65C02::StatusFlags::N, true);
    cpu.X = 0xFF;
    ram[0xFFFC] = _65C02::INS_STA_XIND;
    ram[0xFFFD] = 0x00;
    ram[0x00FF] = 0x00;
    ram[0x0000] = 0x80;
    ram[0x8000] = cpu.A + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 6;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.A, ram[0x8000]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StoreRegisterTests, STAIndirectYCanStoreARegisterIntoMemory) {
    cpu.Y = 0x04;
    ram[0xFFFC] = _65C02::INS_STA_INDY;
    ram[0xFFFD] = 0x02;
    ram[0x0002] = 0x00;
    ram[0x0003] = 0x80;
    ram[0x8004] = cpu.A + 1;
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 5;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.A, ram[0x8004]);
    EXPECT_FALSE(cpu.PS.test(_65C02::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(_65C02::StatusFlags::N));
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(StoreRegisterTests, STAIndirectYCanStoreARegisterIntoMemoryWhenPageBounderyCrossed) {
    cpu.Y = 0xFF;
    ram[0xFFFC] = _65C02::INS_STA_INDY;
    ram[0xFFFD] = 0x02;
    ram[0x0002] = 0x02;
    ram[0x0003] = 0x80;
    ram[0x8101] = cpu.A + 1; //0x8002 + 0xFF
    auto psCopy = cpu.PS;
    auto pcCopy = cpu.PC;
    constexpr dword EXPECTED_CYCLES = 6;
    constexpr dword EXPECTED_BYTES = 2;
    cpu.execute();

    EXPECT_EQ(cpu.cycles.getCycles(), EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC - pcCopy, EXPECTED_BYTES);
    EXPECT_EQ(cpu.A, ram[0x8101]);
    EXPECT_EQ(cpu.PS, psCopy);
}
