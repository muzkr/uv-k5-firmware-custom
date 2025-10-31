#ifdef ENABLE_FLASHLIGHT

#include "driver/gpio.h"
#include "flashlight.h"

#if !defined(ENABLE_FEAT_F4HWN) || defined(ENABLE_FEAT_F4HWN_RESCUE_OPS)
    enum FlashlightMode_t  gFlashLightState;

    void FlashlightTimeSlice()
    {
        if (gFlashLightState == FLASHLIGHT_BLINK && (gFlashLightBlinkCounter & 15u) == 0) {
            GPIO_TogglePin(GPIO_PIN_FLASHLIGHT);
            return;
        }

        if (gFlashLightState == FLASHLIGHT_SOS) {
            const uint16_t u = 15;
            static uint8_t c;
            static uint16_t next;

            if (gFlashLightBlinkCounter - next > 7 * u) {
                c = 0;
                next = gFlashLightBlinkCounter + 1;
                return;
            }

            if (gFlashLightBlinkCounter == next) {
                if (c==0) {
                    GPIO_ResetOutputPin(GPIO_PIN_FLASHLIGHT);
                } else {
                    GPIO_TogglePin(GPIO_PIN_FLASHLIGHT);
                }

                if (c >= 18) {
                    next = gFlashLightBlinkCounter + 7 * u;
                    c = 0;
                } else if(c==7 || c==9 || c==11) {
                    next = gFlashLightBlinkCounter + 3 * u;
                } else {
                    next = gFlashLightBlinkCounter + u;
                }
                c++;
            }
        }
    }

    void ACTION_FlashLight(void)
    {
        switch (gFlashLightState) {
            case FLASHLIGHT_OFF:
                gFlashLightState++;
                GPIO_SetOutputPin(GPIO_PIN_FLASHLIGHT);
                break;
            case FLASHLIGHT_ON:
            case FLASHLIGHT_BLINK:
                gFlashLightState++;
                break;
            case FLASHLIGHT_SOS:
            default:
                gFlashLightState = 0;
                GPIO_ResetOutputPin(GPIO_PIN_FLASHLIGHT);
        }
    }
#else
    void ACTION_FlashLight(void)
    {
        static bool gFlashLightState = false;

        if(gFlashLightState)
        {
            GPIO_ResetOutputPin(GPIO_PIN_FLASHLIGHT);
        }
        else
        {
            GPIO_SetOutputPin(GPIO_PIN_FLASHLIGHT);
        }

        gFlashLightState = (gFlashLightState) ? false : true;
    }
#endif
#endif
