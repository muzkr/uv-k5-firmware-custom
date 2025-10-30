/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <stdint.h>
#include <stdio.h>     // NULL

#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/spi.h"
#include "driver/gpio.h"
#include "driver/spi.h"
#include "driver/st7565.h"
#include "driver/system.h"
#include "misc.h"

uint8_t gStatusLine[LCD_WIDTH];
uint8_t gFrameBuffer[FRAME_LINES][LCD_WIDTH];

static void DrawLine(uint8_t column, uint8_t line, const uint8_t * lineBuffer, unsigned size_defVal)
{   
    ST7565_SelectColumnAndLine(column + 4, line);
    GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_A0);
    for (unsigned i = 0; i < size_defVal; i++) {
        while ((SPI0->FIFOST & SPI_FIFOST_TFF_MASK) != SPI_FIFOST_TFF_BITS_NOT_FULL) {}
        SPI0->WDR = lineBuffer ? lineBuffer[i] : size_defVal;
    }
    SPI_WaitForUndocumentedTxFifoStatusBit();
}

void ST7565_DrawLine(const unsigned int Column, const unsigned int Line, const uint8_t *pBitmap, const unsigned int Size)
{
    SPI_ToggleMasterMode(&SPI0->CR, false);
    DrawLine(Column, Line, pBitmap, Size);
    SPI_ToggleMasterMode(&SPI0->CR, true);
}


#ifdef ENABLE_FEAT_F4HWN
    // Optimization
    //
    // ST7565_BlitScreen(0) = ST7565_BlitStatusLine()
    // ST7565_BlitScreen(1..7) = ST7565_BlitLine()
    // ST7565_BlitScreen(8) = ST7565_BlitFullScreen()
    //

    static void ST7565_BlitScreen(uint8_t line)
    {
        SPI_ToggleMasterMode(&SPI0->CR, false);
        ST7565_WriteByte(0x40);

        if(line == 0)
        {
            DrawLine(0, 0, gStatusLine, LCD_WIDTH);
        }
        else if(line <= FRAME_LINES)
        {
            DrawLine(0, line, gFrameBuffer[line - 1], LCD_WIDTH);
        }
        else
        {
            for (line = 1; line <= FRAME_LINES; line++) {
                DrawLine(0, line, gFrameBuffer[line - 1], LCD_WIDTH);
            }
        }

        SPI_ToggleMasterMode(&SPI0->CR, true);
    }

    void ST7565_BlitFullScreen(void)
    {
        ST7565_BlitScreen(8);
    }

    void ST7565_BlitLine(unsigned line)
    {
        ST7565_BlitScreen(line + 1);
    }

    void ST7565_BlitStatusLine(void)
    {
        ST7565_BlitScreen(0);
    }
#else
    void ST7565_BlitFullScreen(void)
    {
        SPI_ToggleMasterMode(&SPI0->CR, false);
        ST7565_WriteByte(0x40);
        for (unsigned line = 0; line < FRAME_LINES; line++) {
            DrawLine(0, line+1, gFrameBuffer[line], LCD_WIDTH);
        }
        SPI_ToggleMasterMode(&SPI0->CR, true);
    }

    void ST7565_BlitLine(unsigned line)
    {
        SPI_ToggleMasterMode(&SPI0->CR, false);
        ST7565_WriteByte(0x40);    // start line ?
        DrawLine(0, line+1, gFrameBuffer[line], LCD_WIDTH);
        SPI_ToggleMasterMode(&SPI0->CR, true);
    }

    void ST7565_BlitStatusLine(void)
    {   // the top small text line on the display
        SPI_ToggleMasterMode(&SPI0->CR, false);
        ST7565_WriteByte(0x40);    // start line ?
        DrawLine(0, 0, gStatusLine, LCD_WIDTH);
        SPI_ToggleMasterMode(&SPI0->CR, true);
    }
#endif

void ST7565_FillScreen(uint8_t value)
{
    SPI_ToggleMasterMode(&SPI0->CR, false);
    for (unsigned i = 0; i < 8; i++) {
        DrawLine(0, i, NULL, value);
    }
    SPI_ToggleMasterMode(&SPI0->CR, true);
}

// Software reset
const uint8_t ST7565_CMD_SOFTWARE_RESET = 0xE2;
// Bias Select
// 1 0 1 0 0 0 1 BS
// Select bias setting 0=1/9; 1=1/7 (at 1/65 duty)
const uint8_t ST7565_CMD_BIAS_SELECT = 0xA2;
// COM Direction
// 1 1 0 0 MY - - -
// Set output direction of COM
// MY=1, reverse direction
// MY=0, normal direction
const uint8_t ST7565_CMD_COM_DIRECTION = 0xC0;
// SEG Direction
// 1 0 1 0 0 0 0 MX
// Set scan direction of SEG
// MX=1, reverse direction
// MX=0, normal direction
const uint8_t ST7565_CMD_SEG_DIRECTION = 0xA0;
// Inverse Display
// 1 0 1 0 0 1 1 INV
// INV =1, inverse display
// INV =0, normal display
const uint8_t ST7565_CMD_INVERSE_DISPLAY = 0xA6;
// All Pixel ON
// 1 0 1 0 0 1 0 AP
// AP=1, set all pixel ON
// AP=0, normal display
const uint8_t ST7565_CMD_ALL_PIXEL_ON = 0xA4;
// Regulation Ratio
// 0 0 1 0 0 RR2 RR1 RR0
// This instruction controls the regulation ratio of the built-in regulator
const uint8_t ST7565_CMD_REGULATION_RATIO = 0x20;
// Double command!! Set electronic volume (EV) level
// Send next: 0 0 EV5 EV4 EV3 EV2 EV1 EV0  contrast 0-63
const uint8_t ST7565_CMD_SET_EV = 0x81;
// Control built-in power circuit ON/OFF - 0 0 1 0 1 VB VR VF
// VB: Built-in Booster
// VR: Built-in Regulator
// VF: Built-in Follower
const uint8_t ST7565_CMD_POWER_CIRCUIT = 0x28;
// Set display start line 0-63
// 0 0 0 1 S5 S4 S3 S2 S1 S0 
const uint8_t ST7565_CMD_SET_START_LINE = 0x40;
// Display ON/OFF 
// 0 0 1 0 1 0 1 1 1 D 
// D=1, display ON
// D=0, display OFF
const uint8_t ST7565_CMD_DISPLAY_ON_OFF = 0xAE;

uint8_t cmds[] = {
    ST7565_CMD_BIAS_SELECT | 0,             // Select bias setting: 1/9
    ST7565_CMD_COM_DIRECTION  | (0 << 3),   // Set output direction of COM: normal
    ST7565_CMD_SEG_DIRECTION | 1,           // Set scan direction of SEG: reverse
    ST7565_CMD_INVERSE_DISPLAY | 0,         // Inverse Display: false
    ST7565_CMD_ALL_PIXEL_ON | 0,            // All Pixel ON: false - normal display
    ST7565_CMD_REGULATION_RATIO | (4 << 0), // Regulation Ratio 5.0

    ST7565_CMD_SET_EV,                      // Set contrast
    31,

    ST7565_CMD_POWER_CIRCUIT | 0b111,       // Built-in power circuit ON/OFF: VB=1 VR=1 VF=1 
    ST7565_CMD_SET_START_LINE | 0,          // Set Start Line: 0
    ST7565_CMD_DISPLAY_ON_OFF | 1,          // Display ON/OFF: ON
};

#ifdef ENABLE_FEAT_F4HWN
    static void ST7565_Cmd(uint8_t i)
    {
        switch(i) {
            case 3:
                ST7565_WriteByte(ST7565_CMD_INVERSE_DISPLAY | gSetting_set_inv);
                break;
            case 7:
                ST7565_WriteByte(21 + gSetting_set_ctr);
                break;
            default:
                ST7565_WriteByte(cmds[i]);
        }
    }

    #if defined(ENABLE_FEAT_F4HWN_CTR) || defined(ENABLE_FEAT_F4HWN_INV)
    void ST7565_ContrastAndInv(void)
    {
        SPI_ToggleMasterMode(&SPI0->CR, false);
        ST7565_WriteByte(ST7565_CMD_SOFTWARE_RESET);   // software reset

        for(uint8_t i = 0; i < 8; i++)
        {
            ST7565_Cmd(i);
        }
    }
    #endif

    int16_t map(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    #if !defined(ENABLE_SPECTRUM) || !defined(ENABLE_FMRADIO)
    void ST7565_Gauge(uint8_t line, uint8_t min, uint8_t max, uint8_t value)
    {
        gFrameBuffer[line][54] = 0x0c;
        gFrameBuffer[line][55] = 0x12;

        gFrameBuffer[line][121] = 0x12;
        gFrameBuffer[line][122] = 0x0c;

        uint8_t filled = map(value, min, max, 56, 120);

        for (uint8_t i = 56; i <= 120; i++) {
            gFrameBuffer[line][i] = (i <= filled) ? 0x2d : 0x21;
        }
    }
    #endif
#endif
    
void ST7565_Init(void)
{
    SPI0_Init();
    ST7565_HardwareReset();
    SPI_ToggleMasterMode(&SPI0->CR, false);
    ST7565_WriteByte(ST7565_CMD_SOFTWARE_RESET);   // software reset
    SYSTEM_DelayMs(120);

    for(uint8_t i = 0; i < 8; i++)
    {
#ifdef ENABLE_FEAT_F4HWN
        ST7565_Cmd(i);
#else
        ST7565_WriteByte(cmds[i]);
#endif
    }

    ST7565_WriteByte(ST7565_CMD_POWER_CIRCUIT | 0b011);   // VB=0 VR=1 VF=1
    SYSTEM_DelayMs(1);
    ST7565_WriteByte(ST7565_CMD_POWER_CIRCUIT | 0b110);   // VB=1 VR=1 VF=0
    SYSTEM_DelayMs(1);

    for(uint8_t i = 0; i < 4; i++) // why 4 times?
        ST7565_WriteByte(ST7565_CMD_POWER_CIRCUIT | 0b111);   // VB=1 VR=1 VF=1

    SYSTEM_DelayMs(40);
    
    ST7565_WriteByte(ST7565_CMD_SET_START_LINE | 0);   // line 0
    ST7565_WriteByte(ST7565_CMD_DISPLAY_ON_OFF | 1);   // D=1
    SPI_WaitForUndocumentedTxFifoStatusBit();
    SPI_ToggleMasterMode(&SPI0->CR, true);

    ST7565_FillScreen(0x00);
}

#ifdef ENABLE_FEAT_F4HWN_SLEEP
    void ST7565_ShutDown(void)
    {
        SPI_ToggleMasterMode(&SPI0->CR, false);
        ST7565_WriteByte(ST7565_CMD_POWER_CIRCUIT | 0b000);   // VB=0 VR=1 VF=1
        ST7565_WriteByte(ST7565_CMD_SET_START_LINE | 0);   // line 0
        ST7565_WriteByte(ST7565_CMD_DISPLAY_ON_OFF | 0);   // D=1
        SPI_ToggleMasterMode(&SPI0->CR, true);
    }
#endif

void ST7565_FixInterfGlitch(void)
{
    SPI_ToggleMasterMode(&SPI0->CR, false);
    for(uint8_t i = 0; i < ARRAY_SIZE(cmds); i++)
#ifdef ENABLE_FEAT_F4HWN
        ST7565_Cmd(i);
#else
        ST7565_WriteByte(cmds[i]);
#endif
    SPI_WaitForUndocumentedTxFifoStatusBit();
    SPI_ToggleMasterMode(&SPI0->CR, true);
}

void ST7565_HardwareReset(void)
{
    GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES);
    SYSTEM_DelayMs(1);
    GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES);
    SYSTEM_DelayMs(20);
    GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES);
    SYSTEM_DelayMs(120);
}

void ST7565_SelectColumnAndLine(uint8_t Column, uint8_t Line)
{
    GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_ST7565_A0);
    while ((SPI0->FIFOST & SPI_FIFOST_TFF_MASK) != SPI_FIFOST_TFF_BITS_NOT_FULL) {}
    SPI0->WDR = Line + 176;
    while ((SPI0->FIFOST & SPI_FIFOST_TFF_MASK) != SPI_FIFOST_TFF_BITS_NOT_FULL) {}
    SPI0->WDR = ((Column >> 4) & 0x0F) | 0x10;
    while ((SPI0->FIFOST & SPI_FIFOST_TFF_MASK) != SPI_FIFOST_TFF_BITS_NOT_FULL) {}
    SPI0->WDR = ((Column >> 0) & 0x0F);
    SPI_WaitForUndocumentedTxFifoStatusBit();
}

void ST7565_WriteByte(uint8_t Value)
{
    GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_ST7565_A0);
    while ((SPI0->FIFOST & SPI_FIFOST_TFF_MASK) != SPI_FIFOST_TFF_BITS_NOT_FULL) {}
    SPI0->WDR = Value;
}
