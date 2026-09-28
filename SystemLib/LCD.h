#ifndef _65C02_SYSTEM_EMULATION_LCD_H
#define _65C02_SYSTEM_EMULATION_LCD_H

#include "vrEmuLcd.h"
#include "system_types.h"
#include <chrono>

// HD44780U-A00 controller attached to the board's 16x2, 5x8-dot module.
// PA7=E, PA6=R/W, PA5=RS; PB7..PB0=DB7..DB0. All time is emulated.
class LCD {
public:
    explicit LCD(double oscillatorHz = 270000);
    ~LCD();
    LCD(const LCD&) = delete;
    LCD& operator=(const LCD&) = delete;
    void powerOn();
    void tick(double cpuFrequencyHz);
    void advanceTime(std::chrono::nanoseconds elapsed);
    bool isBusy() const { return elapsedNs < busyUntilNs; }
    byte readAddress() const;
    byte readStatus() const;
    // Full-byte access helpers obey busy timing; pin-level users use PA/PB.
    void sendCommand(byte data);
    void writeByte(byte data);
    void updatePixels();
    int8_t pixelState(int x, int y) const;
    int numPixelsX() const;
    int numPixelsY() const;
    void portAWrite(byte data);
    byte portBRead() const;
    void portBWrite(byte data, byte outputMask = 0xff);
    byte dataOutputMask() const;

private:
    bool fourBit() const { return !(lcd->functionFlags & LCD_CMD_FUNCTION_8BIT); }
    void finishDataRead();
    void dataTransferDelay(byte oldAddress);
    uint64_t instructionDelay(uint64_t nominalNs) const;
    VrEmuLcd* lcd{};
    double oscillatorHz;
    uint64_t elapsedNs{}, busyUntilNs{}, addressReadyNs{};
    long double fractionalNs{};
    byte oldAddress{}, dataRegister{};
    byte hostData{}, hostMask{}, heldData{}, readLatch{}, writeHighNibble{};
    bool enable{}, readWrite{}, registerSelect{};
    bool phaseRead{}, phaseData{}, phaseAccepted{}, lowNibble{};
    bool firstNibbleData{}, firstNibbleRead{}, firstNibbleAccepted{};
    static constexpr int LCD_HEIGHT = 2;
    static constexpr int LCD_WIDTH = 16;
};
#endif
