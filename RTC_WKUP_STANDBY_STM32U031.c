/*
 *  Created on: Mar 10, 2025
 *  Author: BalazsFarkas
 *  Project: STM32_WKUP_RTC
 *  Processor: STM32U031
 *  Compiler: ARM-GCC (STM32 IDE)
 *  Program version: 1.0
 *  File: RTC_WKUP_STM32U031.c
 *  Change history: N/A
 */

#include <RTC_WKUP_STANDBY_STM32U031.h>

void set_RTC_WKUP(void){

	/*
	 * Function to set up the RTC as a periodic wake-up timer
	 * RTC registers are write-protected - like the FLASH registers - and must be unlocked
	 * RTC can store dates and activate alarms on dates...or can just be a timer to wake the mcu up from a low power state
	 *
	 */


	  //unlock BDCR register
	  PWR->CR1 |= (1<<8);														//access to RTC allowed
	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	//DBP bit must be set to remove write protection from RTC registers
	  RTC->WPR = 0xCA; 															//remove write protection from RTC
	  RTC->WPR = 0x53;

	  //set clocking for the RTC
	  RCC->BDCR	|= (1<<15);														//enable RTC clocking
	//  RCC->BDCR	|= (1<<0);													//enable LSE
	//  RCC->BDCR |= (1<<8);														//select RTC source as LSE
	//  RCC->BDCR &= ~(1<<9);														//select RTC source as LSE
	  RCC->BDCR &= ~(1<<8);														//select RTC source as LSI
	  RCC->BDCR |= (1<<9);														//select RTC source as LSI

	  RTC->CR &= ~(1<<10);														//disable wake-up timer

	  while(((RTC->ICSR) & (1<<2)) == 0);										//while WUTF is LOW, we don't move on
	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	//If WUTF is HIGH, we can update the WKUP timer

	  RTC->WUTR = 0x2000;														//set timer to 8096
	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	//Note: we need the ICSR WUTWF bit to be 1 to change this

	  RTC->CR &= ~(7<<0);														//ck_wut will be 1/16 of RTC
	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	//Note: since the RTC is running on LSI, ck_wut will be 2 kHz
	  RTC->CR |= (1<<14);														//enable WUTIE - WKUP interrupt
	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	//Any RTC interrupt will wake the mcu from standby mode

	  RTC->SCR |= (1<<2);														//clear WUTF flag for the RTC

	  RTC->CR |= (1<<10);														//enable wake-up timer

	  PWR->CR1 &= ~(1<<8);														//re-apply write protection

}


void set_GPIO_WKUP(void){
	/*
	 * Set the WKUP1 pin on the mcu
	 *
	 */

	  RCC->APBENR1 |= (1<<28);													//enable PWR interface clocking if it isn't already
	  PWR->CR3 |= (1<<0);														//enable wake up pin on PA0
	  PWR->SCR |= (1<<0);														//clear WUF (wake up flag) for WKUP1 pin

}


void set_standby(void){

	/*
	 * Activate standby mode
	 * Standby mode completely turns off the mcu
	 * When waking up, it is considered a system reset - NOT a power-on reset!
	 * Difference is SBF and RTC register are not reset
	 *
	 */

	  // Select Standby mode
	  PWR->CR1 |= (3<<0);										 			//set LPMS bits for standby mode
	  PWR->CR1 &= ~(1<<2);										  			//set LPMS bits for standby mode
	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	//Note: we need standby mode since shutdown mode resets also the SBF flag.

	  // Set SLEEPDEEP bit of Cortex System Control Register
	  SCB->SCR |= (1<<2);													//sleepdeep set in CMSIS

	  // This option is used to ensure that store operations are completed
	  //Note: without this section here, we might not go to standby mode, just stop mode, a less power-efficient mode
	  #if defined ( __CC_ARM)
		  __force_stores();
	  #endif

	  __WFI();


}


void disable_WKUP(void){

	/*
	 * We deactivate the two wkup sources
	 * This is to avoid double-starting of the mcu
	 *
	 */

	  RCC->APBENR1 |= (1<<28);																//enable PWR interface clocking
	  PWR->CR3 &= ~(1<<0);																	//disable WKUP1 (PA0)
	  RTC->CR &= ~(1<<10);																	//disable wake-up timer

}
