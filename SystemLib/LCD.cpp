#include <iostream>
#include "LCD.h"

LCD::LCD() {
    lcd = vrEmuLcdNew(LCD_WIDTH, LCD_HEIGHT, EmuLcdRomA00);
    /*vrEmuLcdSendCommand(lcd, LCD_CMD_FUNCTION | LCD_CMD_FUNCTION_LCD_2LINE | 0x10);
    vrEmuLcdSendCommand(lcd, LCD_CMD_CLEAR);
    vrEmuLcdSendCommand(lcd, LCD_CMD_HOME);
    vrEmuLcdSendCommand(lcd, LCD_CMD_DISPLAY | LCD_CMD_DISPLAY_ON);
    vrEmuLcdSendCommand(lcd, LCD_CMD_DISPLAY_CURSOR);*/
}

LCD::~LCD() {
    vrEmuLcdDestroy(lcd);
}

void LCD::sendCommand(uint8_t data) {
    busy = true;
    lcdInstructionStartTimePoint = __builtin_ia32_rdtsc();
    vrEmuLcdSendCommand(lcd, data);
}

void LCD::writeByte(uint8_t data) {
    busy = true;
    lcdInstructionStartTimePoint = __builtin_ia32_rdtsc();
    vrEmuLcdWriteByte(lcd, data);
}

void LCD::updatePixels() {
    vrEmuLcdUpdatePixels(lcd);
}

char LCD::pixelState(int x, int y) {
    return vrEmuLcdPixelState(lcd, x, y);
}

int LCD::numPixelsX() {
    return vrEmuLcdNumPixelsX(lcd);
}

int LCD::numPixelsY() {
    return vrEmuLcdNumPixelsY(lcd);
}

void LCD::portAWrite(byte data) {
    E = data & 0b10000000;
    RW = data & 0b01000000;
    RS = data & 0b00100000;
    if(E.toggled_on && !busy) {
        if(!RS && !RW) {
            sendCommand(data_lines);
        } else if(!RS) {
            data_lines = 0x00;
        } else if(!RW) {
            writeByte(data_lines);
        } else {
            //unknown behavior
        }
    } else if(E.toggled_on && busy) {
        if(!RS && RW) {
            data_lines = 0b10000000;
        }
    }
    if((__builtin_ia32_rdtsc() - lcdInstructionStartTimePoint) > lcdFunctionDuration)
        busy = false;
}

void LCD::portBWrite(byte data) {
    data_lines = data;
}

byte LCD::portBRead() const {
    return data_lines;
}