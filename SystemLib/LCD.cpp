#include "LCD.h"
#include <cmath>
#include <new>
#include <stdexcept>

LCD::LCD(double oscillatorHz) : oscillatorHz(oscillatorHz) {
    if (!std::isfinite(oscillatorHz) || oscillatorHz < 100000 || oscillatorHz > 1000000)
        throw std::invalid_argument("LCD oscillator must be between 100 kHz and 1 MHz");
    powerOn();
}

LCD::~LCD() { vrEmuLcdDestroy(lcd); }

void LCD::powerOn() {
    auto* replacement = vrEmuLcdNew(LCD_WIDTH, LCD_HEIGHT, EmuLcdRomA00);
    if (!replacement) throw std::bad_alloc();
    vrEmuLcdDestroy(lcd);
    lcd = replacement;
    lcd->blinkHalfPeriodNanoseconds = uint64_t(std::llround(102400.0e9 / oscillatorHz));
    elapsedNs = addressReadyNs = 0;
    busyUntilNs = 10000000; // Internal power-on reset, 10 ms at 5 V.
    fractionalNs = 0;
    oldAddress = dataRegister = hostData = hostMask = heldData = readLatch = writeHighNibble = 0;
    enable = readWrite = registerSelect = phaseRead = phaseData = phaseAccepted = lowNibble = false;
    firstNibbleData = firstNibbleRead = firstNibbleAccepted = false;
}

void LCD::tick(double cpuFrequencyHz) {
    fractionalNs += 1.0e9L / cpuFrequencyHz;
    const auto wholeNs = uint64_t(fractionalNs);
    fractionalNs -= wholeNs;
    advanceTime(std::chrono::nanoseconds(wholeNs));
}

void LCD::advanceTime(std::chrono::nanoseconds elapsed) {
    if (elapsed.count() < 0) throw std::invalid_argument("LCD time cannot run backwards");
    elapsedNs += uint64_t(elapsed.count());
    vrEmuLcdAdvanceTime(lcd, uint64_t(elapsed.count()));
}

uint64_t LCD::instructionDelay(uint64_t nominalNs) const {
    return uint64_t(std::ceil(nominalNs * 270000.0 / oscillatorHz));
}

byte LCD::readAddress() const {
    return elapsedNs < addressReadyNs ? oldAddress : vrEmuLcdReadAddress(lcd);
}
byte LCD::readStatus() const { return byte((isBusy() ? 0x80 : 0) | readAddress()); }

void LCD::dataTransferDelay(byte previousAddress) {
    oldAddress = previousAddress;
    busyUntilNs = elapsedNs + instructionDelay(37000);
    // Figure 10: AC readback changes tADD = 1.5 / fOSC after BF falls.
    addressReadyNs = busyUntilNs + uint64_t(std::ceil(1.5e9 / oscillatorHz));
}

void LCD::sendCommand(byte data) {
    if (isBusy() || data == 0) return;
    vrEmuLcdSendCommand(lcd, data);
    addressReadyNs = elapsedNs;
    const bool clearOrHome = data == 1 || (data & 0xfe) == 2;
    busyUntilNs = elapsedNs + instructionDelay(clearOrHome ? 1520000 : 37000);
    // Address set and DDRAM cursor shifts prefetch the addressed data into DR.
    if ((data & 0xc0) || ((data & 0xf0) == 0x10 && !(data & 8)))
        dataRegister = vrEmuLcdReadByteNoInc(lcd);
}

void LCD::writeByte(byte data) {
    if (isBusy()) return;
    const byte previousAddress = vrEmuLcdReadAddress(lcd);
    dataRegister = data;
    vrEmuLcdWriteByte(lcd, data);
    dataTransferDelay(previousAddress);
}

void LCD::finishDataRead() {
    const byte previousAddress = vrEmuLcdReadAddress(lcd);
    vrEmuLcdReadByte(lcd);
    dataRegister = vrEmuLcdReadByteNoInc(lcd);
    dataTransferDelay(previousAddress);
}

void LCD::portAWrite(byte data) {
    heldData = portBRead();
    const bool previousEnable = enable;
    enable = (data & 0x80) != 0;
    readWrite = (data & 0x40) != 0;
    registerSelect = (data & 0x20) != 0;
    if (!previousEnable && enable) {
        phaseRead = readWrite;
        phaseData = registerSelect;
        phaseAccepted = !isBusy() || (phaseRead && !phaseData);
        // Changing RS/RW halfway through a nibble pair is outside the protocol;
        // discard that partial transaction instead of mixing unrelated bytes.
        if (lowNibble && (phaseRead != firstNibbleRead || phaseData != firstNibbleData))
            lowNibble = false;
        if (!fourBit() || !lowNibble) {
            firstNibbleRead = phaseRead;
            firstNibbleData = phaseData;
            firstNibbleAccepted = phaseAccepted;
            if (phaseRead) readLatch = phaseData ? (phaseAccepted ? dataRegister : 0xff) : readStatus();
        }
    } else if (previousEnable && !enable) {
        const bool paired = fourBit();
        if (paired && !lowNibble) {
            if (!phaseRead) writeHighNibble = hostData & 0xf0;
            lowNibble = true;
            return;
        }
        const byte value = paired ? byte(writeHighNibble | ((hostData >> 4) & 0x0f)) : hostData;
        lowNibble = false;
        if (phaseRead) {
            if (phaseData && phaseAccepted && (!paired || firstNibbleAccepted) && !isBusy())
                finishDataRead();
        } else if (!isBusy() && (!paired || firstNibbleAccepted)) {
            // The HD44780 latches write data on E's falling edge.
            if (phaseData) writeByte(value);
            else sendCommand(value);
        }
    }
}

byte LCD::dataOutputMask() const {
    return enable && readWrite ? (fourBit() ? 0xf0 : 0xff) : 0;
}

byte LCD::portBRead() const {
    const byte deviceMask = dataOutputMask();
    const byte deviceData = fourBit() ? byte(lowNibble ? readLatch << 4 : readLatch & 0xf0) : readLatch;
    // Explicit external drive takes priority in invalid contention scenarios.
    return byte((hostData & hostMask) | (deviceData & deviceMask & ~hostMask) |
                (heldData & ~(hostMask | deviceMask)));
}

void LCD::portBWrite(byte data, byte outputMask) {
    heldData = portBRead();
    hostData = byte((data & outputMask) | (hostData & ~outputMask));
    hostMask = outputMask;
}

void LCD::updatePixels() { vrEmuLcdUpdatePixels(lcd); }
int8_t LCD::pixelState(int x, int y) const { return vrEmuLcdPixelState(lcd, x, y); }
int LCD::numPixelsX() const { return vrEmuLcdNumPixelsX(lcd); }
int LCD::numPixelsY() const { return vrEmuLcdNumPixelsY(lcd); }
