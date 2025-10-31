/* Copyright 2025 muzkr https://github.com/muzkr
 * Copyright 2023 Dual Tachyon
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

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/systick.h"

#define PIN_SCL     GPIO_MAKE_PIN(GPIOA, LL_GPIO_PIN_5)
#define PIN_SDA     GPIO_MAKE_PIN(GPIOA, LL_GPIO_PIN_6)

static inline void SCL_Set()
{
    GPIO_SetOutputPin(PIN_SCL);
}

static inline void SCL_Reset()
{
    GPIO_ResetOutputPin(PIN_SCL);
}

static inline void SDA_Set()
{
    GPIO_SetOutputPin(PIN_SDA);
}

static inline void SDA_Reset()
{
    GPIO_ResetOutputPin(PIN_SDA);
}

static inline void SDA_SetDir(bool Output)
{
    LL_GPIO_SetPinMode(GPIO_PORT(PIN_SDA), GPIO_PIN_MASK(PIN_SDA), Output ? LL_GPIO_MODE_OUTPUT : LL_GPIO_MODE_INPUT);
}

static inline bool SDA_IsSet()
{
    return GPIO_IsInputPinSet(PIN_SDA);
}

void I2C_Start(void)
{
    SDA_Set();
    SYSTICK_DelayUs(1);
    SCL_Set();
    SYSTICK_DelayUs(1);
    SDA_Reset();
    SYSTICK_DelayUs(1);
    SCL_Reset();
    SYSTICK_DelayUs(1);
}

void I2C_Stop(void)
{
    SDA_Reset();
    SYSTICK_DelayUs(1);
    SCL_Reset();
    SYSTICK_DelayUs(1);
    SCL_Set();
    SYSTICK_DelayUs(1);
    SDA_Set();
    SYSTICK_DelayUs(1);
}

uint8_t I2C_Read(bool bFinal)
{
    uint8_t i, Data;

    SDA_SetDir(false);

    Data = 0;
    for (i = 0; i < 8; i++) {
        SCL_Reset();
        SYSTICK_DelayUs(1);
        SCL_Set();
        SYSTICK_DelayUs(1);
        Data <<= 1;
        SYSTICK_DelayUs(1);
        if (SDA_IsSet()) {
            Data |= 1U;
        }
        SCL_Reset();
        SYSTICK_DelayUs(1);
    }

    SDA_SetDir(true);
    SCL_Reset();
    SYSTICK_DelayUs(1);
    if (bFinal) {
        SDA_Set();
    } else {
        SDA_Reset();
    }
    SYSTICK_DelayUs(1);
    SCL_Set();
    SYSTICK_DelayUs(1);
    SCL_Reset();
    SYSTICK_DelayUs(1);

    return Data;
}

int I2C_Write(uint8_t Data)
{
    uint8_t i;
    int ret = -1;

    SCL_Reset();
    SYSTICK_DelayUs(1);
    for (i = 0; i < 8; i++) {
        if ((Data & 0x80) == 0) {
            SDA_Reset();
        } else {
            SDA_Set();
        }
        Data <<= 1;
        SYSTICK_DelayUs(1);
        SCL_Set();
        SYSTICK_DelayUs(1);
        SCL_Reset();
        SYSTICK_DelayUs(1);
    }

    SDA_SetDir(false);
    SDA_Set();
    SYSTICK_DelayUs(1);
    SCL_Set();
    SYSTICK_DelayUs(1);

    for (i = 0; i < 255; i++) {
        if (!SDA_IsSet()) {
            ret = 0;
            break;
        }
    }

    SCL_Reset();
    SYSTICK_DelayUs(1);
    SDA_SetDir(true);
    SDA_Set();

    return ret;
}

int I2C_ReadBuffer(void *pBuffer, uint8_t Size)
{
    uint8_t *pData = (uint8_t *)pBuffer;
    uint8_t i;

    for (i = 0; i < Size - 1; i++) {
        SYSTICK_DelayUs(1);
        pData[i] = I2C_Read(false);
    }

    SYSTICK_DelayUs(1);
    pData[i] = I2C_Read(true);

    return Size;
}

int I2C_WriteBuffer(const void *pBuffer, uint8_t Size)
{
    const uint8_t *pData = (const uint8_t *)pBuffer;
    uint8_t i;

    for (i = 0; i < Size; i++) {
        if (I2C_Write(*pData++) < 0) {
            return -1;
        }
    }

    return 0;
}

