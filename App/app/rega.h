/*
 * REGA Alam and Test
 * ==================
 *
 * This function sends out a fixed ZVEI tone sequence for testing or alarm purposes for the Alpine REGA alarm channel.
 * Further information: https://www.rega.ch/en/our-missions/sites-and-infrastructure/emergency-radio
 *
 * There are two REGA ACTIONS: Test and Alarm.
 * The Test action sends out a fixed ZVEI tone sequence for testing purposes.
 * The Alarm action sends out a fixed ZVEI tone sequence for alarm purposes.
 * 
 * These actions can be assigned to a key in the settings menu.
 * 
 * The Test/Alarm function will perform the following actions:
 * - Set the radio configuration to the REGA frequency, CTCSS, FM, full power etc.
 * - Start transmitting
 * - Wait 100ms to allow the radio to switch to transmit mode and stabilize the transmitter
 * - Send out the ZVEI tone sequence
 * - Wait 100ms to allow the radio to finish transmitting
 * - Set the radio back to receive mode
 * 
 * The REGA frequency is 161.300 Mhz
 *
 * The ZVEI tone squence for alarm is: 21414
 * The ZVEI tone squence for test is: 21301 
 *
 * The two tone sequences are hardcoded in two arrays
 *
 * Copyright 2025 Markus Bärtschi
 * 
 * https://github.com/markusb
 *
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

#ifndef REGA_H
#define REGA_H

#include <stdint.h>

#define ZVEI_NUM_TONES 5
#define ZVEI_TONE_LENGTH_MS 70 // 70
#define ZVEI_PAUSE_LENGTH_MS 10 // 10
#define ZVEI_PRE_LENGTH_MS 300
#define ZVEI_POST_LENGTH_MS 100
#define REGA_CTCSS_FREQ_INDEX 18 // dcs.c: CTCSS_Options[18] = 1230
#define REGA_FREQUENCY 16130000 // 43370000 16130000

void ACTION_RegaAlarm(void);
void ACTION_RegaTest(void);
void UI_DisplayREGA(void);
void REGA_TransmitZvei(const uint16_t[], const char[]);

#endif
