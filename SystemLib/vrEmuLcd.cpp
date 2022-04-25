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
#include <ctime>


/*
 * Function:  increment
 * --------------------
 * increments the ddRam pointer of a VrEmuLcd
 *
 * automatically skips to the correct line and
 * rolls back to the start
 */
static void increment(VrEmuLcd* lcd)
{
    ++lcd->ddPtr;

    // find pointer offset from start
    ptrdiff_t offset = lcd->ddPtr - lcd->ddRam;

    if (lcd->gdPtr)// && !lcd->graphicsMode)
        {
        if (offset >= 0x40) lcd->ddPtr = lcd->ddRam;
        }
    else if (lcd->rows > 1)
    {
        if (offset == 0x28) lcd->ddPtr = lcd->ddRam + 0x40;
        else if (offset == 0x68 || offset >= DDRAM_SIZE) lcd->ddPtr = lcd->ddRam;
    }
    else if (offset >= DDRAM_VISIBLE_SIZE)
    {
        lcd->ddPtr = lcd->ddRam;
    }
}

/*
 * Function:  decrement
 * --------------------
 * decrements the ddRam pointer of a VrEmuLcd
 *
 * automatically skips to the correct line and
 * rolls back to the end
 */
static void decrement(VrEmuLcd* lcd)
{
    --lcd->ddPtr;

    // find pointer offset from start
    ptrdiff_t offset = lcd->ddPtr - lcd->ddRam;

    if (lcd->gdPtr)
    {
        if (offset == -1) lcd->ddPtr = lcd->ddRam + 0x3f;
    }
    else if (lcd->rows > 1)
    {
        if (offset == -1) lcd->ddPtr = lcd->ddRam + 0x67;
        else if (offset == 0x39) lcd->ddPtr = lcd->ddRam  + 0x27;
    }

    if (offset == -1)
    {
        lcd->ddPtr += DDRAM_VISIBLE_SIZE;
    }
    else if (offset >= DDRAM_SIZE)
    {
        lcd->ddPtr = lcd->ddRam;
    }
}

/*
 * Function:  doShiftDdram
 * --------------------
 * shift the cursor or display as required
 * by the current entry mode flags
 */
static void doShift(VrEmuLcd* lcd)
{
    // if we're looking at cgram, shift the cg pointer
    if (lcd->cgPtr)
    {
        if (lcd->entryModeFlags & LCD_CMD_ENTRY_MODE_INCREMENT)
        {
            ++lcd->cgPtr;
            if (lcd->cgPtr >= (uint8_t*)lcd->cgRam + sizeof(lcd->cgRam))
            {
                lcd->cgPtr = (uint8_t*)lcd->cgRam;
            }
        }
        else
        {
            --lcd->cgPtr;
            if (lcd->cgPtr < (uint8_t*)lcd->cgRam)
            {
                lcd->cgPtr = (uint8_t*)lcd->cgRam + sizeof(lcd->cgRam) - 1;
            }
        }
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
        if (lcd->entryModeFlags & LCD_CMD_ENTRY_MODE_SHIFT)
        {
            if (lcd->entryModeFlags & LCD_CMD_ENTRY_MODE_INCREMENT)
            {
                ++lcd->scrollOffset;
            }
            else
            {
                --lcd->scrollOffset;
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

        memset(lcd->cgRam, DEFAULT_CGRAM_BYTE, sizeof(lcd->cgRam));

        if (lcd->pixels != NULL)
        {
            memset(lcd->pixels, -1, lcd->numPixels);
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
                --lcd->scrollOffset;
            }
            else
            {
                ++lcd->scrollOffset;
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
        lcd->scrollOffset = 0;
    }
    else if (command & LCD_CMD_CLEAR)
    {
        if (lcd->ddRam != NULL)
        {
            memset(lcd->ddRam, ' ', DDRAM_SIZE);
        }
        lcd->ddPtr = lcd->ddRam;
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
        // find row offset
        int row = (lcd->cgPtr - (uint8_t*)lcd->cgRam) % CHAR_HEIGHT_PX;

        // find starting uint8_t for the current character
        uint8_t* startAddr = lcd->cgPtr - row;

        for (int i = 0; i < CHAR_WIDTH_PX; ++i)
        {
            uint8_t bit = data & ((0x01 << (CHAR_WIDTH_PX - 1)) >> i);
            if (bit)
            {
                *(startAddr + i) |= (0x80 >> row);
            }
            else
            {
                *(startAddr + i) &= ~(0x80 >> row);
            }
        }
    }
    else if (lcd->graphicsMode)
    {
        *lcd->gdPtr = data;
    }
    else
    {
        *lcd->ddPtr = data;
    }
    doShift(lcd);
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

    doShift(lcd);

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
    uint8_t data = 0;

    if (lcd->cgPtr)
    {
        // find row offset
        int row = (lcd->cgPtr - (uint8_t*)lcd->cgRam) % 8;

        // find starting uint8_t for the current character
        uint8_t* startAddr = lcd->cgPtr - row;

        for (int i = 0; i < CHAR_WIDTH_PX; ++i)
        {
            if (*(startAddr + i) & (0x80 >> row))
            {
                data |= ((0x01 << (CHAR_WIDTH_PX - 1)) >> i);
            }
        }
    }
    else
    {
        data = *(lcd->ddPtr);
    }

    return data;
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
        return lcd->cgRam[c];
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
    // adjust for display scroll offset
    if (row >= lcd->rows) row = lcd->rows - 1;

    while (lcd->scrollOffset < 0)
    {
        lcd->scrollOffset += lcd->dataWidthCols;
    }

    int dataCol = (col + lcd->scrollOffset) % lcd->dataWidthCols;
    int rowOffset = row * lcd->dataWidthCols;

    if (lcd->rows > 1)
    {
        if (lcd->gdPtr)
        {
            rowOffset = rowOffsetsGfx[row] * 2;
        }
        else
        {
            rowOffset = rowOffsets[row];
        }
    }

    return rowOffset + dataCol;
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
                if (((int)(clock() * CLOCK_TO_MS) % CURSOR_BLINK_CYCLE_MS)
                < CURSOR_BLINK_PERIOD_MS)
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
                    int drawCursor = cursorOn && (ddPtr == lcd->ddPtr);

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
                                *pixel = -1;
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
            if (((int)(clock() * CLOCK_TO_MS) % CURSOR_BLINK_CYCLE_MS)
            < CURSOR_BLINK_PERIOD_MS)
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
                int drawCursor = cursorOn && (ddPtr == lcd->ddPtr);

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
                        if (!displayOn)
                        {
                            *pixel = -1;
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
char vrEmuLcdPixelState(VrEmuLcd* lcd, int x, int y)
{
    int offset = y * lcd->pixelsWidth + x;
    if (offset >= 0 && offset < lcd->numPixels)
        return lcd->pixels[offset];
    return -1;
}