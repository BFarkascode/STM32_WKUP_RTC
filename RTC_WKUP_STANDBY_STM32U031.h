/*
 *  Created on: Mar 10, 2025
 *  Author: BalazsFarkas
 *  Project: STM32_WKUP_RTC
 *  Processor: STM32U031
 *  Compiler: ARM-GCC (STM32 IDE)
 *  Header version: 1.0
 *  File: RTC_WKUP_STM32U031.h
 *  Change history: N/A
 */


#ifndef INC_RTC_WKUP_STANDBY_STM32U031_H_
#define INC_RTC_WKUP_STANDBY_STM32U031_H_

#include "stm32u031xx.h"

//LOCAL CONSTANT

//LOCAL VARIABLE

//EXTERNAL VARIABLE

//FUNCTION PROTOTYPES
void set_RTC_WKUP(void);
void set_GPIO_WKUP(void);
void set_standby(void);
void disable_WKUP(void);

#endif /* INC_RTC_WKUP_STANDBY_STM32U031_H_ */
