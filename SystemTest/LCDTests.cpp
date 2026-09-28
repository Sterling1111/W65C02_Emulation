#include "gtest/gtest.h"
#include "LCD.h"
#include "System.h"
#include <memory>
#include <thread>

using namespace std::chrono;
namespace {
constexpr byte E = 0x80, RW = 0x40, RS = 0x20;
void pulseWrite(LCD& lcd, byte data, bool rs = false, byte mask = 0xff) {
    lcd.portAWrite(rs ? RS : 0);
    lcd.portBWrite(data, mask);
    lcd.portAWrite(byte(E | (rs ? RS : 0)));
    lcd.portAWrite(rs ? RS : 0);
}
byte pulseRead(LCD& lcd, bool rs = false) {
    const byte control = byte(RW | (rs ? RS : 0));
    lcd.portBWrite(0, 0);
    lcd.portAWrite(control); lcd.portAWrite(control | E);
    const byte value = lcd.portBRead();
    lcd.portAWrite(control);
    return value;
}
void write4(LCD& lcd, byte data, bool rs = false) {
    pulseWrite(lcd, data & 0xf0, rs, 0xf0);
    pulseWrite(lcd, byte(data << 4), rs, 0xf0);
}
byte read4(LCD& lcd, bool rs = false) {
    const byte high = pulseRead(lcd, rs) & 0xf0;
    return byte(high | (pulseRead(lcd, rs) >> 4));
}
void ready(LCD& lcd) { lcd.advanceTime(milliseconds(10)); }
}

class LcdCore : public testing::Test {
protected:
    std::unique_ptr<VrEmuLcd, decltype(&vrEmuLcdDestroy)> lcd{
        vrEmuLcdNew(16, 2, EmuLcdRomA00), vrEmuLcdDestroy};
    void command(byte value) { vrEmuLcdSendCommand(lcd.get(), value); }
    void put(byte value) { vrEmuLcdWriteByte(lcd.get(), value); }
    byte address() { return vrEmuLcdReadAddress(lcd.get()); }
    byte peek() { return vrEmuLcdReadByteNoInc(lcd.get()); }
    void SetUp() override { command(0x38); command(0x0c); }
};

TEST_F(LcdCore, DdramWrapsBothDirectionsAcrossBothFortyCharacterLines) {
    command(0xa7); put('A'); EXPECT_EQ(address(), 0x40);
    command(0xe7); put('B'); EXPECT_EQ(address(), 0);
    command(0x04); command(0xc0); put('C'); EXPECT_EQ(address(), 0x27);
    command(0x80); put('D'); EXPECT_EQ(address(), 0x67);
    command(0xa7); EXPECT_EQ(peek(), 'A'); command(0xe7); EXPECT_EQ(peek(), 'B');
}

TEST_F(LcdCore, OneLineAddressingUsesEightyBytesIndependentOfModuleSize) {
    command(0x30); command(0xcf); put('X'); EXPECT_EQ(address(), 0);
    command(0x04); command(0x80); put('Y'); EXPECT_EQ(address(), 0x4f);
    command(0x06); command(0xa7); put('Z'); EXPECT_EQ(address(), 0x28);
}

TEST_F(LcdCore, CgramIsSixtyFourRawBytesAndWrapsInBothDirections) {
    command(0x7f); put(0xed); EXPECT_EQ(address(), 0);
    put(0x95); command(0x04); command(0x40); put(0x72); EXPECT_EQ(address(), 63);
    EXPECT_EQ(peek(), 0xed); command(0x40); EXPECT_EQ(peek(), 0x72);
}

TEST_F(LcdCore, CustomCharacterAliasesAndPixelOrientation) {
    command(0x40);
    for (byte row : {byte(0x10), byte(8), byte(4), byte(2), byte(1), byte(0), byte(0), byte(0x1f)}) put(row);
    command(0x80); put(0); put(8); vrEmuLcdUpdatePixels(lcd.get());
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 5; ++x) {
            const int expected = (y < 5 && x == y) || y == 7;
            EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), x, y), expected);
            EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), x + 6, y), expected);
        }
}

TEST_F(LcdCore, ClearSelectsDdramResetsIncrementButPreservesEntryShift) {
    command(0x80); put('A'); command(0x05); command(0x40); put(0x1f);
    command(0x01); EXPECT_EQ(address(), 0); EXPECT_EQ(peek(), ' ');
    EXPECT_EQ(lcd->entryModeFlags & 3, 3);
    put('B'); EXPECT_EQ(address(), 1); EXPECT_EQ(lcd->scrollOffset, 1);
    command(0x40); EXPECT_EQ(peek(), 0x1f);
}

TEST_F(LcdCore, HomeSelectsDdramPreservesMemoryAndEntryDirection) {
    command(0x80); put('A'); command(0x04); command(0x18); command(0x40);
    command(0x03); EXPECT_EQ(address(), 0); EXPECT_EQ(peek(), 'A');
    EXPECT_EQ(lcd->scrollOffset, 0); EXPECT_EQ(lcd->entryModeFlags & 2, 0);
}

TEST_F(LcdCore, EntryShiftOnlyAppliesToDdramWrites) {
    command(0x07); put('A'); EXPECT_EQ(lcd->scrollOffset, 1);
    vrEmuLcdReadByte(lcd.get()); EXPECT_EQ(address(), 2); EXPECT_EQ(lcd->scrollOffset, 1);
    command(0x40); put(0x1f); vrEmuLcdReadByte(lcd.get()); EXPECT_EQ(lcd->scrollOffset, 1);
    command(0x05); command(0x80); put('B'); EXPECT_EQ(lcd->scrollOffset, 0);
}

TEST_F(LcdCore, DisplayShiftsWrapEachLineWithoutMovingTheAddressCounter) {
    command(0x85); command(0x1c);
    EXPECT_EQ(address(), 5);
    EXPECT_EQ(vrEmuLcdGetDataOffset(lcd.get(), 0, 0), 0x27);
    EXPECT_EQ(vrEmuLcdGetDataOffset(lcd.get(), 1, 0), 0x67);
    command(0x18); EXPECT_EQ(vrEmuLcdGetDataOffset(lcd.get(), 0, 0), 0);
    command(0x14); EXPECT_EQ(address(), 6); command(0x10); EXPECT_EQ(address(), 5);
}

TEST_F(LcdCore, DisplayOffClearsEveryDotAndDisplayOnRestoresContents) {
    command(0x80);
    for (int i = 0; i < 40; ++i) put(0xff);
    command(0xc0);
    for (int i = 0; i < 40; ++i) put(0xff);
    vrEmuLcdUpdatePixels(lcd.get()); EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 4, 0), 1);
    command(0x08); vrEmuLcdUpdatePixels(lcd.get());
    for (int y = 0; y < 17; ++y)
        for (int x = 0; x < 95; ++x)
            EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), x, y), (y == 8 || x % 6 == 5) ? -1 : 0);
    command(0x0c); vrEmuLcdUpdatePixels(lcd.get()); EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 4, 0), 1);
}

TEST_F(LcdCore, CursorUnderlineAndBlinkUseEmulatedTime) {
    command(0x80); command(0x0e); vrEmuLcdUpdatePixels(lcd.get());
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 0, 0), 0);
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 0, 7), 1);
    command(0x0d); vrEmuLcdUpdatePixels(lcd.get());
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 0, 7), 0);
    vrEmuLcdAdvanceTime(lcd.get(), 379259258); vrEmuLcdUpdatePixels(lcd.get());
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 0, 0), 0);
    vrEmuLcdAdvanceTime(lcd.get(), 1); vrEmuLcdUpdatePixels(lcd.get());
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 0, 0), 1);
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 4, 7), 1);
    vrEmuLcdAdvanceTime(lcd.get(), 379259259); vrEmuLcdUpdatePixels(lcd.get());
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 0, 0), 0);
}

TEST_F(LcdCore, PixelCoordinatesCannotAliasAdjacentRows) {
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 95, 0), -1);
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), -1, 1), -1);
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 0, -1), -1);
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 0, 17), -1);
}

TEST_F(LcdCore, OneLineModeDoesNotDrawSecondPhysicalRow) {
    command(0xc0); put(0xff); command(0x30); vrEmuLcdUpdatePixels(lcd.get());
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 0, 9), 0);
    command(0x38); vrEmuLcdUpdatePixels(lcd.get());
    EXPECT_EQ(vrEmuLcdPixelState(lcd.get(), 0, 9), 1);
}

TEST(LcdGeometry, FourRowsMapToTwoControllerLinesAndShiftAcrossFortyColumns) {
    auto* lcd = vrEmuLcdNew(16, 4, EmuLcdRomA00);
    ASSERT_NE(lcd, nullptr); vrEmuLcdSendCommand(lcd, 0x38);
    EXPECT_EQ(vrEmuLcdGetDataOffset(lcd, 2, 0), 0x10);
    EXPECT_EQ(vrEmuLcdGetDataOffset(lcd, 3, 0), 0x50);
    vrEmuLcdSendCommand(lcd, 0x1c);
    EXPECT_EQ(vrEmuLcdGetDataOffset(lcd, 0, 0), 0x27);
    EXPECT_EQ(vrEmuLcdGetDataOffset(lcd, 2, 0), 0x0f);
    vrEmuLcdDestroy(lcd);
}

TEST(LcdTiming, StartupBusyAndCommandDurationsUseOnlyEmulatedTime) {
    LCD lcd;
    EXPECT_EQ(pulseRead(lcd), 0x80);
    lcd.advanceTime(microseconds(9999)); EXPECT_TRUE(lcd.isBusy());
    lcd.advanceTime(microseconds(1)); EXPECT_FALSE(lcd.isBusy());
    pulseWrite(lcd, 0x38); EXPECT_TRUE(lcd.isBusy());
    std::this_thread::sleep_for(milliseconds(1)); EXPECT_TRUE(lcd.isBusy());
    lcd.advanceTime(microseconds(36)); EXPECT_TRUE(lcd.isBusy());
    lcd.advanceTime(microseconds(1)); EXPECT_FALSE(lcd.isBusy());
    for (byte instruction : {byte(1), byte(2), byte(3)}) {
        pulseWrite(lcd, instruction); lcd.advanceTime(microseconds(1519)); EXPECT_TRUE(lcd.isBusy());
        lcd.advanceTime(microseconds(1)); EXPECT_FALSE(lcd.isBusy());
    }
}

TEST(LcdTiming, OscillatorAndCpuClockAreIndependentAndScaleCorrectly) {
    LCD lcd(250000); ready(lcd); lcd.sendCommand(0x38);
    lcd.advanceTime(microseconds(39)); EXPECT_TRUE(lcd.isBusy());
    lcd.advanceTime(microseconds(1)); EXPECT_FALSE(lcd.isBusy());
    LCD normal; ready(normal); normal.sendCommand(0x38);
    for (int i = 0; i < 73; ++i) normal.tick(2000000);
    EXPECT_TRUE(normal.isBusy()); normal.tick(2000000); EXPECT_FALSE(normal.isBusy());
}

TEST(LcdBus, WritesLatchOnFallingEnableWithDataStableAtThatEdge) {
    LCD lcd; ready(lcd); pulseWrite(lcd, 0x38); lcd.advanceTime(microseconds(37));
    lcd.portBWrite('A'); lcd.portAWrite(RS); lcd.portAWrite(RS | E);
    EXPECT_FALSE(lcd.isBusy()); EXPECT_EQ(lcd.readAddress(), 0);
    lcd.portBWrite('B'); lcd.portAWrite(RS); EXPECT_TRUE(lcd.isBusy());
    lcd.advanceTime(microseconds(43)); pulseWrite(lcd, 0x80); lcd.advanceTime(microseconds(37));
    EXPECT_EQ(pulseRead(lcd, true), 'B');
}

TEST(LcdBus, StatusIncludesAddressAndDoesNotAdvanceIt) {
    LCD lcd; ready(lcd); pulseWrite(lcd, 0xc5);
    EXPECT_EQ(pulseRead(lcd), 0xc5); EXPECT_EQ(lcd.readAddress(), 0x45);
    lcd.advanceTime(microseconds(37)); EXPECT_EQ(pulseRead(lcd), 0x45);
    EXPECT_EQ(pulseRead(lcd), 0x45);
}

TEST(LcdBus, BusyWritesAreIgnoredAndAddressReadbackHasDocumentedDelay) {
    LCD lcd; ready(lcd); pulseWrite(lcd, 'A', true); pulseWrite(lcd, 'B', true);
    EXPECT_EQ(pulseRead(lcd), 0x80);
    lcd.advanceTime(microseconds(37)); EXPECT_EQ(pulseRead(lcd), 0);
    lcd.advanceTime(nanoseconds(5555)); EXPECT_EQ(lcd.readAddress(), 0);
    lcd.advanceTime(nanoseconds(1)); EXPECT_EQ(lcd.readAddress(), 1);
    pulseWrite(lcd, 0x80); lcd.advanceTime(microseconds(37)); EXPECT_EQ(pulseRead(lcd, true), 'A');
    lcd.advanceTime(microseconds(43)); EXPECT_EQ(pulseRead(lcd, true), ' ');
}

TEST(LcdBus, DataReadsAdvanceOnceAndDoNotScrollEvenWithEntryShiftEnabled) {
    LCD lcd; ready(lcd);
    for (byte c : {byte('A'), byte('B')}) { pulseWrite(lcd, c, true); lcd.advanceTime(microseconds(43)); }
    pulseWrite(lcd, 0x80); lcd.advanceTime(microseconds(37));
    lcd.portBWrite(0, 0); lcd.portAWrite(RW | RS); lcd.portAWrite(RW | RS | E);
    EXPECT_EQ(lcd.portBRead(), 'A'); EXPECT_EQ(lcd.portBRead(), 'A'); EXPECT_EQ(lcd.readAddress(), 0);
    lcd.portAWrite(RW | RS); lcd.advanceTime(microseconds(43)); EXPECT_EQ(lcd.readAddress(), 1);
    EXPECT_EQ(pulseRead(lcd, true), 'B');
}

TEST(LcdBus, DataRegisterAfterWriteIsStaleUntilAddressSetOrAReadPrefetches) {
    LCD lcd; ready(lcd); pulseWrite(lcd, 'X', true); lcd.advanceTime(microseconds(43));
    EXPECT_EQ(pulseRead(lcd, true), 'X'); // DR still holds the last write, not DDRAM[1].
    lcd.advanceTime(microseconds(43)); EXPECT_EQ(pulseRead(lcd, true), ' ');
    lcd.advanceTime(microseconds(43)); pulseWrite(lcd, 0x80); lcd.advanceTime(microseconds(37));
    EXPECT_EQ(pulseRead(lcd, true), 'X');
}

TEST(LcdBus, ReleasesDataPinsOutsideReadEnableAndPreservesExternalDriveMask) {
    LCD lcd; ready(lcd); lcd.portBWrite(0x55); EXPECT_EQ(lcd.portBRead(), 0x55);
    EXPECT_EQ(lcd.dataOutputMask(), 0);
    lcd.portBWrite(0, 0); lcd.portAWrite(RW | E); EXPECT_EQ(lcd.dataOutputMask(), 0xff);
    EXPECT_EQ(lcd.portBRead(), 0); lcd.portAWrite(RW); EXPECT_EQ(lcd.dataOutputMask(), 0);
}

TEST(LcdFourBit, ByteTransfersBusyStatusAndMemoryReads) {
    LCD lcd; ready(lcd); pulseWrite(lcd, 0x20, false, 0xf0); lcd.advanceTime(microseconds(37));
    write4(lcd, 0x28); lcd.advanceTime(microseconds(37));
    write4(lcd, 0xc5); EXPECT_EQ(read4(lcd), 0xc5); lcd.advanceTime(microseconds(37));
    EXPECT_EQ(read4(lcd), 0x45);
    pulseWrite(lcd, 0x40, true, 0xf0); EXPECT_FALSE(lcd.isBusy()); EXPECT_EQ(lcd.readAddress(), 0x45);
    pulseWrite(lcd, 0x10, true, 0xf0); EXPECT_TRUE(lcd.isBusy()); lcd.advanceTime(microseconds(43));
    write4(lcd, 0xc5); lcd.advanceTime(microseconds(37)); EXPECT_EQ(read4(lcd, true), 'A');
    lcd.advanceTime(microseconds(43)); EXPECT_EQ(lcd.readAddress(), 0x46);
}

TEST(LcdFourBit, StatusPairIsCoherentAndOnlyHighDataPinsAreDriven) {
    LCD lcd; ready(lcd); pulseWrite(lcd, 0x20); lcd.advanceTime(microseconds(37));
    write4(lcd, 0x85);
    const byte high = pulseRead(lcd) & 0xf0;
    lcd.advanceTime(microseconds(100));
    const byte low = pulseRead(lcd) >> 4;
    EXPECT_EQ(high | low, 0x85);
    EXPECT_EQ(read4(lcd), 5);
    lcd.portAWrite(RW | E); EXPECT_EQ(lcd.dataOutputMask(), 0xf0);
}

TEST(LcdFourBit, StandardInitializationRecoversNibblePhaseAndCanReturnToEightBits) {
    LCD lcd; ready(lcd);
    // Standard 3,3,3,2 initialization, sent on DB7..DB4.
    for (byte nibble : {byte(0x30), byte(0x30), byte(0x30), byte(0x20)}) {
        pulseWrite(lcd, nibble, false, 0xf0); lcd.advanceTime(milliseconds(5));
    }
    write4(lcd, 0x28); lcd.advanceTime(microseconds(37));
    // Deliberately leave a nibble pending, then perform the recovery sequence.
    pulseWrite(lcd, 0x30, false, 0xf0);
    for (byte nibble : {byte(0x30), byte(0x30), byte(0x30), byte(0x20)}) {
        pulseWrite(lcd, nibble, false, 0xf0); lcd.advanceTime(milliseconds(5));
    }
    write4(lcd, 0x28); lcd.advanceTime(microseconds(37));
    write4(lcd, 0x38); lcd.advanceTime(microseconds(37));
    pulseWrite(lcd, 0xc2); lcd.advanceTime(microseconds(37)); EXPECT_EQ(pulseRead(lcd), 0x42);
}

TEST(LcdIntegration, CpuClockRunsLcdAndViaDirectionTurnaroundSupportsStatusReads) {
    System s{0, 0x3fff, 0x6000, 0x7fff, 0x8000, 0xffff};
    s.cpu.STOP = true; s.cpu.execute(10000); EXPECT_FALSE(s.lcd.isBusy());
    s.cpu.writeByte(0xe0, 0x6003); s.cpu.writeByte(0xff, 0x6002);
    s.cpu.writeByte(0xc4, 0x6000); s.cpu.writeByte(0, 0x6001);
    s.cpu.writeByte(E, 0x6001); s.cpu.writeByte(0, 0x6001);
    s.cpu.writeByte(0, 0x6002); s.cpu.writeByte(RW, 0x6001); s.cpu.writeByte(RW | E, 0x6001);
    EXPECT_EQ(s.cpu.readByte(0x6000), 0xc4);
    s.cpu.writeByte(RW, 0x6001); s.cpu.execute(40);
    s.cpu.writeByte(RW | E, 0x6001); EXPECT_EQ(s.cpu.readByte(0x6000), 0x44);
}

TEST(LcdIntegration, BoardResetDoesNotPowerCycleOrClearTheLcd) {
    System s{0, 0x3fff, 0x6000, 0x7fff, 0x8000, 0xffff};
    ready(s.lcd); s.lcd.sendCommand(0xc5); s.lcd.advanceTime(microseconds(37));
    s.reset(); s.cpu.stop();
    EXPECT_EQ(s.lcd.readAddress(), 0x45);
}
