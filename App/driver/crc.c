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

#include "driver/crc.h"

void CRC_Init(void)
{
}

uint16_t CRC_Calculate(const void *pBuffer, uint16_t Size)
{
    const uint8_t *pData = (const uint8_t *)pBuffer;
    uint16_t i, Crc;

    Crc = 0;
    for (i = 0; i < Size; i++)
    {
        Crc ^= (pData[i] << 8);

        for (int j = 0; j < 8; j++)
        {
            // Check bit [15]
            if (Crc >> 15)
            {
                Crc = (Crc << 1) ^ 0x1021;
            }
            else
            {
                Crc = Crc << 1;
            }
        }
    }

    return Crc;
}
