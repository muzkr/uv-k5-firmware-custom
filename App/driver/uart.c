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

#include <stdbool.h>
#include <string.h>
#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_system.h"
#include "py32f0xx_ll_dma.h"
#include "py32f0xx_ll_gpio.h"
#include "py32f0xx_ll_usart.h"
#include "driver/uart.h"

#define DMA_CHANNEL LL_DMA_CHANNEL_2

static bool UART_IsLogEnabled;
uint8_t UART_DMA_Buffer[256];

void UART_Init(void)
{
    // PA2 TX
    // PA3 RX

    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);

    // Pins
    do
    {
        LL_GPIO_InitTypeDef GPIO_InitStruct;

        LL_GPIO_StructInit(&GPIO_InitStruct);
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Alternate = LL_GPIO_AF1_USART1;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;

        //
        GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
        LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
        LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    } while (0);

    // DMA
    do
    {
        LL_DMA_DisableChannel(DMA1, DMA_CHANNEL);

        LL_DMA_InitTypeDef DMA_InitStruct;
        LL_DMA_StructInit(&DMA_InitStruct);

        DMA_InitStruct.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
        DMA_InitStruct.Mode = LL_DMA_MODE_CIRCULAR;
        DMA_InitStruct.PeriphOrM2MSrcAddress = LL_USART_DMA_GetRegAddr(USART1);
        DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
        DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
        DMA_InitStruct.MemoryOrM2MDstAddress = (uint32_t)UART_DMA_Buffer;
        DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
        DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
        DMA_InitStruct.NbData = sizeof(UART_DMA_Buffer);
        DMA_InitStruct.Priority = LL_DMA_PRIORITY_HIGH;

        LL_DMA_Init(DMA1, DMA_CHANNEL, &DMA_InitStruct);

        LL_SYSCFG_SetDMARemap_CH2(LL_SYSCFG_DMA_MAP_USART1_RX);

    } while (0);

    // USART
    do
    {
        LL_USART_Disable(USART1);

        LL_USART_InitTypeDef USART_InitStruct;
        LL_USART_StructInit(&USART_InitStruct);

        USART_InitStruct.BaudRate = 38400;
        USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
        LL_USART_Init(USART1, &USART_InitStruct);

        LL_USART_EnableDMAReq_RX(USART1);

    } while (0);

    LL_DMA_EnableChannel(DMA1, DMA_CHANNEL);
    LL_USART_Enable(USART1);
    LL_USART_TransmitData8(USART1, 0);
}

void UART_Send(const void *pBuffer, uint32_t Size)
{
    const uint8_t *pData = (const uint8_t *)pBuffer;
    uint32_t i;

    for (i = 0; i < Size; i++) {
        while (!LL_USART_IsActiveFlag_TXE(USART1))
            ;
        LL_USART_TransmitData8(USART1, pData[i]);
    }
}

void UART_LogSend(const void *pBuffer, uint32_t Size)
{
    if (UART_IsLogEnabled) {
        UART_Send(pBuffer, Size);
    }
}

#ifdef ENABLE_FEAT_F4HWN_SCREENSHOT
    bool UART_IsCableConnected(void) {
        for (size_t i = 0; i < sizeof(UART_DMA_Buffer); i++) {
            if (UART_DMA_Buffer[i] == 0x55) {
                UART_DMA_Buffer[i] = 0x00;  // Clear only the matched byte
                return true;
            }
        }
        return false;
    }
#endif
