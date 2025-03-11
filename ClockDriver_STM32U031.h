/*
 *  Created on: Feb 27, 2025
 *  Author: BalazsFarkas
 *  Project: STM32_WKUP_RTC
 *  Processor: STM32U031
 *  Compiler: ARM-GCC (STM32 IDE)
 *  Header version: 1.0
 *  File: ClockDriver_STM32U031.h
 *  Change history: N/A
 */


#ifndef INC_RCCTIMPWMDELAY_CUSTOM_H_
#define INC_RCCTIMPWMDELAY_CUSTOM_H_

#include "stdint.h"

//LOCAL CONSTANT

//LOCAL VARIABLE

//EXTERNAL VARIABLE

//FUNCTION PROTOTYPES
void SysClockConfig(void);
void TIM6Config (void);
void Delay_us(int micro_sec);
void Delay_ms(int milli_sec);

#endif /* RCCTIMPWMDELAY_CUSTOM_H_ */
