#ifndef _65C02_SYSTEM_EMULATION_LCD_H
#define _65C02_SYSTEM_EMULATION_LCD_H


#include "vrEmuLcd.h"
#include "system_types.h"
#include "Cycles.h"

class LCD {
public:
    LCD();
    ~LCD();
    void sendCommand(uint8_t data);
    void writeByte(uint8_t data);
    void updatePixels();
    char pixelState(int x, int y);
    int numPixelsX();
    int numPixelsY();
    void portAWrite(byte data);
    byte portBRead() const;
    void portBWrite(byte data);

private:
    VrEmuLcd *lcd{nullptr};
    uint8_t data_lines{};
    bool RW{}, RS{};
    uint64_t lcdInstructionStartTimePoint{};
    uint64_t lcdFunctionDuration{Cycles::getTSCFrequency() * 37};
public:
    bool busy{};
private:
    struct Enabled {
        bool enable_line{false};
        bool toggled_on{false};
        Enabled& operator=(bool line) {
            if(!enable_line && line)
                toggled_on = true;
            else
                toggled_on = false;
            enable_line = line;
            return *this;
        }
    }; Enabled E{};
    static constexpr int LCD_HEIGHT = 2;
    static constexpr int LCD_WIDTH = 16;
};


#endif //_65C02_SYSTEM_EMULATION_LCD_H
