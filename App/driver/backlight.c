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

#include "driver/backlight.h"
#include "py32f0xx_ll_system.h"
#include "py32f0xx_ll_dma.h"
#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_tim.h"
#include "driver/gpio.h"
#include "driver/systick.h"
#include "settings.h"
#include "external/printf/printf.h"

#ifdef ENABLE_FEAT_F4HWN
    #include "driver/system.h"
    #include "audio.h"
    #include "misc.h"
#endif

#define TIMx TIM1
#define TIM_CHANNEL LL_TIM_CHANNEL_CH1

// this is decremented once every 500ms
uint16_t gBacklightCountdown_500ms = 0;
bool backlightOn;

#ifdef ENABLE_FEAT_F4HWN
    const uint8_t value[] = {0, 3, 6, 9, 15, 24, 38, 62, 100, 159, 255};
#endif

#ifdef ENABLE_FEAT_F4HWN_SLEEP
    uint16_t gSleepModeCountdown_500ms = 0;
#endif

void BACKLIGHT_InitHardware()
{
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);
    LL_APB1_GRP2_ForceReset(LL_APB1_GRP2_PERIPH_TIM1);
    LL_APB1_GRP2_ReleaseReset(LL_APB1_GRP2_PERIPH_TIM1);

    // PA8
    do
    {
        LL_GPIO_InitTypeDef InitStruct;
        InitStruct.Pin = LL_GPIO_PIN_8;
        InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        InitStruct.Alternate = LL_GPIO_AF_2;
        InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        InitStruct.Pull = LL_GPIO_PULL_NO;
        LL_GPIO_Init(GPIOA, &InitStruct);
    } while (0);

    do
    {
        LL_TIM_OC_InitTypeDef InitStruct;
        InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
        InitStruct.OCState = LL_TIM_OCSTATE_ENABLE;
        InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
        InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
        InitStruct.CompareValue = 0;
        LL_TIM_OC_Init(TIMx, TIM_CHANNEL, &InitStruct);
    } while (0);

    do
    {
        LL_TIM_InitTypeDef InitStruct;
        InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
        InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
        InitStruct.Prescaler = SystemCoreClock / 100000 - 1;
        InitStruct.Autoreload = 1000 - 1;
        InitStruct.RepetitionCounter = 0;
        LL_TIM_Init(TIMx, &InitStruct);
    } while (0);

    LL_TIM_EnableAllOutputs(TIMx);
    LL_TIM_EnableCounter(TIMx);
}

static void BACKLIGHT_Sound(void)
{
    if (gEeprom.POWER_ON_DISPLAY_MODE == POWER_ON_DISPLAY_MODE_SOUND || gEeprom.POWER_ON_DISPLAY_MODE == POWER_ON_DISPLAY_MODE_ALL)
    {
        AUDIO_PlayBeep(BEEP_880HZ_60MS_DOUBLE_BEEP);
        AUDIO_PlayBeep(BEEP_880HZ_60MS_DOUBLE_BEEP);
    }

    gK5startup = false;
}


void BACKLIGHT_TurnOn(void)
{
    #ifdef ENABLE_FEAT_F4HWN_SLEEP
        gSleepModeCountdown_500ms = gSetting_set_off * 120;
    #endif

    #ifdef ENABLE_FEAT_F4HWN
        gBacklightBrightnessOld = BACKLIGHT_GetBrightness();
    #endif

    if (gEeprom.BACKLIGHT_TIME == 0) {
        BACKLIGHT_TurnOff();
        #ifdef ENABLE_FEAT_F4HWN
            if(gK5startup == true) 
            {
                BACKLIGHT_Sound();
            }
        #endif
        return;
    }

    backlightOn = true;

#ifdef ENABLE_FEAT_F4HWN
    if(gK5startup == true) {
        #if defined(ENABLE_FMRADIO) && defined(ENABLE_SPECTRUM)
            BACKLIGHT_SetBrightness(gEeprom.BACKLIGHT_MAX);
        #else
            for(uint8_t i = 0; i <= gEeprom.BACKLIGHT_MAX; i++)
            {
                BACKLIGHT_SetBrightness(i);
                SYSTEM_DelayMs(50);
            }
        #endif

        BACKLIGHT_Sound();
    }
    else
    {
        BACKLIGHT_SetBrightness(gEeprom.BACKLIGHT_MAX);
    }
#else
    BACKLIGHT_SetBrightness(gEeprom.BACKLIGHT_MAX);
#endif

    switch (gEeprom.BACKLIGHT_TIME) {
        default:
        case 1 ... 60:  // 5 sec * value
            gBacklightCountdown_500ms = 1 + (gEeprom.BACKLIGHT_TIME * 5) * 2;
            break;
        case 61:    // always on
            gBacklightCountdown_500ms = 0;
            break;
    }
}

void BACKLIGHT_TurnOff()
{
#ifdef ENABLE_BLMIN_TMP_OFF
    register uint8_t tmp;

    if (gEeprom.BACKLIGHT_MIN_STAT == BLMIN_STAT_ON)
        tmp = gEeprom.BACKLIGHT_MIN;
    else
        tmp = 0;

    BACKLIGHT_SetBrightness(tmp);
#else
    BACKLIGHT_SetBrightness(gEeprom.BACKLIGHT_MIN);
#endif
    gBacklightCountdown_500ms = 0;
    backlightOn = false;
}

bool BACKLIGHT_IsOn()
{
    return backlightOn;
}

static uint8_t currentBrightness = 0;

void BACKLIGHT_SetBrightness(uint8_t brigtness)
{
    LL_TIM_OC_SetCompareCH1(TIMx, value[brigtness] * 1000u / 255u);
    currentBrightness = brigtness;
}

uint8_t BACKLIGHT_GetBrightness(void)
{
    return currentBrightness;
}
