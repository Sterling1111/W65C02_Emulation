/*
 * Troy's HD44780U Lcd Display Emulator
 *
 * Copyright (c) 2020 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/VrEmuLcd
 *
 */

#include "vrEmuLcd.h"
#include <cstdlib>
#include <cstddef>
#include <memory.h>
#include <algorithm>


/*
 * Function:  increment
 * --------------------
 * increments the ddRam pointer of a VrEmuLcd
 *
 * automatically skips to the correct line and
 * rolls back to the start
 */
static void moveDdram(VrEmuLcd* lcd, bool forward)
{
    int address = int(lcd->ddPtr - lcd->ddRam);
    if (lcd->gdRam) {
        address = (address + (forward ? 1 : 63)) % 64;
    } else if (lcd->functionFlags & LCD_CMD_FUNCTION_LCD_2LINE) {
        if (forward) {
            address = address == 0x27 ? 0x40 : address == 0x67 ? 0 : (address + 1) & 0x7f;
        } else {
            address = address == 0 ? 0x67 : address == 0x40 ? 0x27 : (address - 1) & 0x7f;
        }
    } else {
        address = forward ? (address == 0x4f ? 0 : (address + 1) & 0x7f)
                          : (address == 0 ? 0x4f : (address - 1) & 0x7f);
    }
    lcd->ddPtr = lcd->ddRam + address;
}

static void increment(VrEmuLcd* lcd) { moveDdram(lcd, true); }
static void decrement(VrEmuLcd* lcd) { moveDdram(lcd, false); }

static void shiftDisplay(VrEmuLcd* lcd, int amount) {
    const int width = lcd->gdRam ? lcd->dataWidthCols :
        (lcd->functionFlags & LCD_CMD_FUNCTION_LCD_2LINE ? 40 : 80);
    lcd->scrollOffset = (lcd->scrollOffset + amount + width) % width;
}

/*
 * Function:  doShiftDdram
 * --------------------
 * shift the cursor or display as required
 * by the current entry mode flags
 */
static void doShift(VrEmuLcd* lcd, bool writing)
{
    // if we're looking at cgram, shift the cg pointer
    if (lcd->cgPtr)
    {
        const int address = int(lcd->cgPtr - reinterpret_cast<uint8_t*>(lcd->cgRam));
        const int delta = lcd->entryModeFlags & LCD_CMD_ENTRY_MODE_INCREMENT ? 1 : 63;
        lcd->cgPtr = reinterpret_cast<uint8_t*>(lcd->cgRam) + (address + delta) % 64;
    }
    // otherwise, shift the ddram pointer or scroll offset
    else if (lcd->graphicsMode)
    {
        ++lcd->gdPtr;
        if (lcd->gdPtr >= (uint8_t*)lcd->gdRam + GDRAM_SIZE)
        {
            lcd->gdPtr = (uint8_t*)lcd->gdRam;
        }
    }
    else
    {
        if (writing && (lcd->entryModeFlags & LCD_CMD_ENTRY_MODE_SHIFT))
        {
            if (lcd->entryModeFlags & LCD_CMD_ENTRY_MODE_INCREMENT)
            {
                shiftDisplay(lcd, 1);
            }
            else
            {
                shiftDisplay(lcd, -1);
            }
        }

        if (lcd->entryModeFlags & LCD_CMD_ENTRY_MODE_INCREMENT)
        {
            increment(lcd);
        }
        else
        {
            decrement(lcd);
        }
    }
}


/* Function:  vrEmuLcdNew
 * --------------------
 * create a new LCD
 *
 * cols: number of display columns  (8 to 40)
 * rows: number of display rows (1, 2 or 4)
 * rom:  character rom to load
 */
VrEmuLcd* vrEmuLcdNew(int cols, int rows, vrEmuLcdCharacterRom rom)
{
    int graphicsLCD = 0;

    if (cols == GRAPHICS_WIDTH_PX && rows == GRAPHICS_HEIGHT_PX)
    {
        cols = 16;
        rows = 4;
        graphicsLCD = 1;
    }

    // validate display size
    if (cols < DISPLAY_MIN_COLS) cols = DISPLAY_MIN_COLS;
    else if (cols > DISPLAY_MAX_COLS) cols = DISPLAY_MAX_COLS;

    if (rows < DISPLAY_MIN_ROWS) rows = DISPLAY_MIN_ROWS;
    else if (rows > DISPLAY_MAX_ROWS) rows = DISPLAY_MAX_ROWS;
    if (rows == 3) rows = 2;

    // build lcd data structure
    VrEmuLcd* lcd = (VrEmuLcd*)malloc(sizeof(VrEmuLcd));
    if (lcd != NULL)
    {
        lcd->cols = cols;
        lcd->rows = rows;
        lcd->characterRom = rom;

        lcd->ddRam = (uint8_t*)malloc(DDRAM_SIZE);
        lcd->ddPtr = lcd->ddRam;
        lcd->entryModeFlags = LCD_CMD_ENTRY_MODE_INCREMENT;
        lcd->displayFlags = 0x00;
        lcd->functionFlags = LCD_CMD_FUNCTION_8BIT;
        lcd->elapsedNanoseconds = 0;
        lcd->blinkHalfPeriodNanoseconds = 379259259; // 102400 / 270 kHz seconds.
        lcd->scrollOffset = 0x00;
        lcd->cgPtr = NULL;

        lcd->graphicsMode = 0;
        lcd->extendedMode = 0;
        lcd->graphicsVAddr = 0xff;
        lcd->gdRam = NULL;
        if (graphicsLCD)
        {
            lcd->gdRam = (uint8_t*)malloc(GDRAM_SIZE);
            lcd->pixelsWidth = GRAPHICS_WIDTH_PX;
            lcd->pixelsHeight = GRAPHICS_HEIGHT_PX;
        }
        else
        {
            lcd->pixelsWidth = lcd->cols * (CHAR_WIDTH_PX + 1) - 1;
            lcd->pixelsHeight = lcd->rows * (CHAR_HEIGHT_PX + 1) - 1;
        }
        lcd->gdPtr = lcd->gdRam;
        lcd->numPixels = lcd->pixelsWidth * lcd->pixelsHeight;
        lcd->pixels = (uint8_t*)malloc(lcd->numPixels);

        switch (lcd->rows)
        {
            case 1:
                lcd->dataWidthCols = DATA_WIDTH_CHARS_1ROW;
                break;

                case 2:
                    lcd->dataWidthCols = DATA_WIDTH_CHARS_2ROW;
                    break;

                    case 4:
                        lcd->dataWidthCols = DATA_WIDTH_CHARS_4ROW;
                        break;
        }

        if (graphicsLCD)
        {
            lcd->dataWidthCols = DATA_WIDTH_CHARS_GFX;
        }

        // fill arrays with default data
        if (lcd->ddRam != NULL)
        {
            memset(lcd->ddRam, ' ', DDRAM_SIZE);
        }
        if (lcd->gdRam != NULL)
        {
            memset(lcd->gdRam, 0, GDRAM_SIZE);
        }

        memset(lcd->cgRam, 0, sizeof(lcd->cgRam));

        if (lcd->pixels != NULL)
        {
            memset(lcd->pixels, -1, lcd->numPixels);
        }

        if (!lcd->ddRam || !lcd->pixels || (graphicsLCD && !lcd->gdRam)) {
            vrEmuLcdDestroy(lcd);
            return nullptr;
        }
        vrEmuLcdUpdatePixels(lcd);
    }
    return lcd;
}

/*
 * Function:  vrEmuLcdDestroy
 * --------------------
 * destroy an LCD
 *
 * lcd: lcd object to destroy / clean up
 */
void vrEmuLcdDestroy(VrEmuLcd* lcd)
{
    if (lcd)
    {
        free(lcd->ddRam);
        free(lcd->pixels);
        free(lcd->gdRam);
        memset(lcd, 0, sizeof(VrEmuLcd));
        free(lcd);
    }
}

/*
 * Function:  vrEmuLcdSendCommand
 * --------------------
 * send a command to the lcd (RS is low)
 *
 * command: the data (DB0 -> DB7) to send
 */
void vrEmuLcdSendCommand(VrEmuLcd* lcd, uint8_t command)
{
    if (command & LCD_CMD_SET_DRAM_ADDR)
    {
        // ddram address in remaining 7 bits
        size_t offset = (command & 0x7f);
        if (lcd->graphicsMode)
        {
            if (lcd->graphicsVAddr == 0xff)
            {
                lcd->graphicsVAddr = offset & 0x1f;
            }
            else
            {
                size_t hAddr = offset & 0x0f;
                lcd->gdPtr = lcd->gdRam + (((size_t)lcd->graphicsVAddr * 32) + (hAddr * 2));
                lcd->graphicsVAddr = 0xff;
            }
        }
        else if (lcd->gdPtr)
        {
            lcd->ddPtr = lcd->ddRam + (offset & 0x3f) * 2;
        }
        else
        {
            lcd->ddPtr = lcd->ddRam + offset;
        }
        lcd->cgPtr = NULL;
    }
    else if (command & LCD_CMD_SET_CGRAM_ADDR)
    {
        // cgram address in remaining 6 bits
        lcd->cgPtr = (uint8_t*)lcd->cgRam + (command & 0x3f);
    }
    else if (command & LCD_CMD_FUNCTION)
    {
        lcd->functionFlags = command & 0x1c;
        if (lcd->gdRam)
        {
            lcd->extendedMode = (command & LCD_CMD_FUNCTION_EXT_MODE) ? 1 : 0;

            if (lcd->extendedMode)
            {
                lcd->graphicsMode = (command & LCD_CMD_EXT_FUNCTION_GFX) ? 1 : 0;
            }
        }
    }
    else if (command & LCD_CMD_SHIFT)
    {
        if (command & LCD_CMD_SHIFT_DISPLAY)
        {
            if (command & LCD_CMD_SHIFT_RIGHT)
            {
                shiftDisplay(lcd, -1);
            }
            else
            {
                shiftDisplay(lcd, 1);
            }
        }
        else
        {
            if (command & LCD_CMD_SHIFT_RIGHT)
            {
                increment(lcd);
            }
            else
            {
                decrement(lcd);
            }
        }
    }
    else if (command & LCD_CMD_DISPLAY)
    {
        lcd->displayFlags = command;
    }
    else if (command & LCD_CMD_ENTRY_MODE)
    {
        lcd->entryModeFlags = command;
    }
    else if (command & LCD_CMD_HOME)
    {
        lcd->ddPtr = lcd->ddRam;
        lcd->cgPtr = nullptr;
        lcd->scrollOffset = 0;
    }
    else if (command & LCD_CMD_CLEAR)
    {
        if (lcd->ddRam != NULL)
        {
            memset(lcd->ddRam, ' ', DDRAM_SIZE);
        }
        lcd->ddPtr = lcd->ddRam;
        lcd->cgPtr = nullptr;
        lcd->entryModeFlags |= LCD_CMD_ENTRY_MODE_INCREMENT;
        lcd->scrollOffset = 0;
    }
}

/*
 * Function:  vrEmuLcdWriteByte
 * --------------------
 * write a byte to the lcd (RS is high)
 *
 * data: the data (DB0 -> DB7) to send
 */
void vrEmuLcdWriteByte(VrEmuLcd* lcd, uint8_t data)
{
    if (lcd->cgPtr)
    {
        *lcd->cgPtr = data;
    }
    else if (lcd->graphicsMode)
    {
        *lcd->gdPtr = data;
    }
    else
    {
        *lcd->ddPtr = data;
    }
    doShift(lcd, true);
}


/*
 * Function:  vrEmuLcdReadByte
 * --------------------
 * read a byte from the lcd (RS is high)
 *
 * returns: the data (DB0 -> DB7) at the current address
 */
uint8_t vrEmuLcdReadByte(VrEmuLcd* lcd)
{
    uint8_t data = vrEmuLcdReadByteNoInc(lcd);

    doShift(lcd, false);

    return data;
}


/*
 * Function:  vrEmuLcdReadByteNoInc
 * --------------------
 * read a byte from the lcd (RS is high)
 * don't update the address/scroll
 *
 * returns: the data (DB0 -> DB7) at the current address
 */
uint8_t vrEmuLcdReadByteNoInc(VrEmuLcd* lcd)
{
    if (lcd->cgPtr) return *lcd->cgPtr;
    if (lcd->graphicsMode) return *lcd->gdPtr;
    return *lcd->ddPtr;
}


/* Function:  vrEmuLcdReadAddress
 * --------------------
 * read the current address offset (RS is high, R/W is high)
 *
 * returns: the current address
 */
uint8_t vrEmuLcdReadAddress(VrEmuLcd* lcd)
{
    if (lcd->cgPtr)
    {
        return (lcd->cgPtr - (uint8_t*)lcd->cgRam) & 0x3f;
    }

    uint8_t addr = (lcd->ddPtr - lcd->ddRam) & 0x7f;
    if (lcd->gdPtr) addr >>= 1;

    return addr;
}

/*
 * Function:  vrEmuLcdWriteString
 * ----------------------------------------
 * write a string to the lcd
 * iterates over the characters and sends them individually
 *
 * str: the string to write.
 */
void vrEmuLcdWriteString(VrEmuLcd* lcd, const char* str)
{
    const char* ddPtr = str;
    while (*ddPtr != '\0')
    {
        vrEmuLcdWriteByte(lcd, *ddPtr);
        ++ddPtr;
    }
}

/*
 * Function:  vrEmuLcdCharBits
 * ----------------------------------------
 * return a character's pixel data
 *
 * pixel data consists of 5 bytes where each is
 * a vertical row of bits for the character
 *
 * c: character index
 *    0 - 15   cgram
 *    16 - 255 rom
 */
const uint8_t* vrEmuLcdCharBits(VrEmuLcd* lcd, uint8_t c)
{
    if (lcd->gdRam) // graphic LCD?
        {
        if (c < CGRAM_STORAGE_CHARS) c = CGRAM_STORAGE_CHARS;
        {
            return fontGfx[c - CGRAM_STORAGE_CHARS];
        }
        }

    if (c < CGRAM_STORAGE_CHARS)
    {
        // Codes 00..07 and 08..0f alias the same eight CGRAM glyphs.
        for (int x = 0; x < CHAR_WIDTH_PX; ++x) {
            uint8_t bits = 0;
            for (int y = 0; y < CHAR_HEIGHT_PX; ++y)
                if (lcd->cgRam[c & 7][y] & (0x10 >> x)) bits |= 0x80 >> y;
            lcd->characterColumns[x] = bits;
        }
        return lcd->characterColumns;
    }

    const int characterRomIndex = c - CGRAM_STORAGE_CHARS;

    switch (lcd->characterRom)
    {
        case EmuLcdRomA00:
            return fontA00[characterRomIndex];

            case EmuLcdRomA02:
                default:
                    return fontA02[characterRomIndex];
    }
}

/*
 * Function:  vrEmuLcdGetDataOffset
 * ----------------------------------------
 * return the character offset in ddram for a given
 * row and column.
 *
 * can be used to set the current cursor address
 */
int vrEmuLcdGetDataOffset(VrEmuLcd* lcd, int row, int col)
{
    row = std::max(0, std::min(row, lcd->rows - 1));
    if (lcd->gdRam) {
        const int dataCol = ((col + lcd->scrollOffset) % lcd->dataWidthCols + lcd->dataWidthCols) % lcd->dataWidthCols;
        return rowOffsetsGfx[row] * 2 + dataCol;
    }
    const bool twoLine = (lcd->functionFlags & LCD_CMD_FUNCTION_LCD_2LINE) != 0;
    const int width = twoLine ? 40 : 80;
    // A four-row module splits each 40-character controller line in two.
    const int start = row >= 2 ? lcd->cols : 0;
    const int dataCol = ((col + start + lcd->scrollOffset) % width + width) % width;
    return ((twoLine && (row & 1)) ? 0x40 : 0) + dataCol;
}

/*
 * Function:  vrEmuLcdUpdatePixels
 * ----------------------------------------
 * updates the display's pixel data
 * changes are only reflected in the pixel data when this function is called
 */
void vrEmuLcdUpdatePixels(VrEmuLcd* lcd)
{
    if (lcd->gdRam) // is a graphics LCD
        {
        if (lcd->graphicsMode)
        {
            uint8_t* p = lcd->pixels - 1;

            for (size_t yPos = 0; yPos < GRAPHICS_HEIGHT_PX; ++yPos)
            {
                size_t offset = (yPos & 0x1f) * 32;
                if (yPos & 0x20) offset += 16;

                for (size_t xPos = 0; xPos < GRAPHICS_WIDTH_PX / 8; ++xPos)
                {
                    uint8_t b = lcd->gdRam[offset + xPos];
                    for (int i = 0; i < 8; ++i)
                    {
                        *(++p) = b & 0x80 ? 1 : 0;
                        b <<= 1;
                    }
                }
            }
        }
        else
        {
            // determine cursor blink state
            int cursorOn = lcd->displayFlags & CURSOR_MASK;
            if (lcd->displayFlags & LCD_CMD_DISPLAY_CURSOR_BLINK)
            {
                if ((lcd->elapsedNanoseconds / lcd->blinkHalfPeriodNanoseconds) % 2 == 0)
                {
                    cursorOn &= ~LCD_CMD_DISPLAY_CURSOR_BLINK;
                }
            }

            int displayOn = lcd->displayFlags & LCD_CMD_DISPLAY_ON;

            // /cycle through each row of the display
            for (int row = 0; row < lcd->rows; ++row)
            {
                for (int col = 0; col < lcd->cols; ++col)
                {
                    // find top-left pixel for the current display character position
                    uint8_t* charTopLeft = lcd->pixels + (row * (GFX_CHAR_HEIGHT_PX) * lcd->pixelsWidth) + col * (GFX_CHAR_WIDTH_PX);

                    // find current character in ddram
                    uint8_t* ddPtr = lcd->ddRam + vrEmuLcdGetDataOffset(lcd, row, col);

                    // only draw cursor if the data pointer is pointing at this character
                    int drawCursor = cursorOn && !lcd->cgPtr && (ddPtr == lcd->ddPtr);

                    // get the character data (bits) for the current character
                    const uint8_t* bits = vrEmuLcdCharBits(lcd, *ddPtr);

                    // apply its bits to the pixel data
                    for (int y = 0; y < GFX_CHAR_HEIGHT_PX; ++y)
                    {
                        // set pixel pointer
                        uint8_t* pixel = charTopLeft + y * lcd->pixelsWidth;
                        for (int x = 0; x < GFX_CHAR_WIDTH_PX; ++x)
                        {
                            // is the display on?
                            if (!displayOn)
                            {
                                *pixel++ = 0;
                                continue;
                            }

                            // set the pixel data from the character bits
                            *pixel = (bits[y] & (0x80 >> x)) ? 1 : 0;

                            // override with cursor data if appropriate
                            if (drawCursor)
                            {
                                if ((cursorOn & LCD_CMD_DISPLAY_CURSOR_BLINK) ||
                                ((cursorOn & LCD_CMD_DISPLAY_CURSOR) && y == GFX_CHAR_HEIGHT_PX - 1))
                                {
                                    *pixel = 1;
                                }
                            }

                            // next pixel
                            ++pixel;
                        }
                    }
                }
            }
        }
        }
    else
    {
        // determine cursor blink state
        int cursorOn = lcd->displayFlags & CURSOR_MASK;
        if (lcd->displayFlags & LCD_CMD_DISPLAY_CURSOR_BLINK)
        {
            if ((lcd->elapsedNanoseconds / lcd->blinkHalfPeriodNanoseconds) % 2 == 0)
            {
                cursorOn &= ~LCD_CMD_DISPLAY_CURSOR_BLINK;
            }
        }

        int displayOn = lcd->displayFlags & LCD_CMD_DISPLAY_ON;

        // /cycle through each row of the display
        for (int row = 0; row < lcd->rows; ++row)
        {
            for (int col = 0; col < lcd->cols; ++col)
            {
                // find top-left pixel for the current display character position
                uint8_t* charTopLeft = lcd->pixels + (row * (CHAR_HEIGHT_PX + 1) * lcd->pixelsWidth) + col * (CHAR_WIDTH_PX + 1);

                // find current character in ddram
                uint8_t* ddPtr = lcd->ddRam + vrEmuLcdGetDataOffset(lcd, row, col);

                // only draw cursor if the data pointer is pointing at this character
                int drawCursor = cursorOn && !lcd->cgPtr && (ddPtr == lcd->ddPtr);

                // get the character data (bits) for the current character
                const uint8_t* bits = vrEmuLcdCharBits(lcd, *ddPtr);

                // apply its bits to the pixel data
                for (int y = 0; y < CHAR_HEIGHT_PX; ++y)
                {
                    // set pixel pointer
                    uint8_t* pixel = charTopLeft + y * lcd->pixelsWidth;
                    for (int x = 0; x < CHAR_WIDTH_PX; ++x)
                    {
                        // is the display on?
                        if (!displayOn || (!(lcd->functionFlags & LCD_CMD_FUNCTION_LCD_2LINE) && row != 0))
                        {
                            *pixel++ = 0;
                            continue;
                        }

                        // set the pixel data from the character bits
                        *pixel = (bits[x] & (0x80 >> y)) ? 1 : 0;

                        // override with cursor data if appropriate
                        if (drawCursor)
                        {
                            if ((cursorOn & LCD_CMD_DISPLAY_CURSOR_BLINK) ||
                            ((cursorOn & LCD_CMD_DISPLAY_CURSOR) && y == CHAR_HEIGHT_PX - 1))
                            {
                                *pixel = 1;
                            }
                        }

                        // next pixel
                        ++pixel;
                    }
                }
            }
        }
    }
}

/*
 * Function:  vrEmuLcdNumPixels
 * ----------------------------------------
 * get the number of pixels for the entire display
 */
void vrEmuLcdNumPixels(VrEmuLcd* lcd, int* cols, int* rows)
{
    if (cols)* cols = vrEmuLcdNumPixelsX(lcd);
    if (rows)* rows = vrEmuLcdNumPixelsY(lcd);
}

/*
 * Function:  vrEmuLcdNumPixelsX
 * ----------------------------------------
 * returns: number of horizontal pixels in the display
 */
int vrEmuLcdNumPixelsX(VrEmuLcd* lcd)
{
    return lcd->pixelsWidth;
}

/*
 * Function:  vrEmuLcdNumPixelsY
 * ----------------------------------------
 * returns: number of vertical pixels in the display
 */
int vrEmuLcdNumPixelsY(VrEmuLcd* lcd)
{
    return lcd->pixelsHeight;
}

/*
 * Function:  charvrEmuLcdPixelState
 * ----------------------------------------
 * returns: pixel state at the given location
 *
 * -1 = no pixel (character borders)
 *  0 = pixel off
 *  1 = pixel on
 *
 */
int8_t vrEmuLcdPixelState(VrEmuLcd* lcd, int x, int y)
{
    if (x < 0 || y < 0 || x >= lcd->pixelsWidth || y >= lcd->pixelsHeight) return -1;
    int offset = y * lcd->pixelsWidth + x;
    if (offset < lcd->numPixels)
        return static_cast<int8_t>(lcd->pixels[offset]);
    return -1;
}

void vrEmuLcdAdvanceTime(VrEmuLcd* lcd, uint64_t nanoseconds) {
    const uint64_t period = 2 * lcd->blinkHalfPeriodNanoseconds;
    lcd->elapsedNanoseconds = (lcd->elapsedNanoseconds + nanoseconds % period) % period;
}
