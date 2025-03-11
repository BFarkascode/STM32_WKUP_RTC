# STM32_WKUP_RTC
Bare metal implementation of wake-up mechanism relying on external input or RTC on a STM32U031R8 Nucleo board.

## General description
This is just a mini project to showcase one of the sleep/low power modes of an STM32 and how to wake from said sleep.
I will be using a relatively new STM32U031R8 mcu for this project, though all of my findings will be (mostly) applicable to the L053R8 or the F429ZI as well. The reason why I did so is that I need a very low power mcu to run a CR2032 battery powered solution and the U031R8 was offering the best power characteristics there to be had amongst all ST mcus (almost 10 times lower power consumption in standby mode). In retrospective though, I don’t recommend the U031, since, despite the power demand being actually as promoted, ST managed to break the SPI peripheral on it by adding a 32 bit FIFO on both TX and RX while forgetting to add a mechanism to reset these FIFOs. This is obviously a massive oversight and something that makes the SPI particularly complicated to use with 8-bit sensors…
Anyway, after this heads-up, we will be implementing a simple blinky on the mcu but without using the superloop. Instead, we will write the code within the setup and then force the device to standby mode, waking up the mcu either by a periodic wake-up signal from the real time clock (RTC) or from an external GPIO input.
Let’s get to it.

### Sleepy-sleepy
Mcus are constructed from different components, each separately clocked and powered. This means that we can selectively turn components off or clock them at a lower rate, resulting in lower power consumption. Technically, when we are selecting and activating a low power mode, we select, how ham we want to go with turning off elements within the mcu.
Of course we can “manually” save power by decreasing the voltage scaling of the mcu core – telling the internal voltage regulator to apply less voltage to the mcu core – and by decreasing the system clock rate, albeit in all of these cases, all system elements remain active and are clocked down at the same time. We also retain all functions of the system, meaning that we are staying in “run” or “normal mode”.
When we engage low-power modes, that isn’t the case anymore. We might stop clocking some peripheral completely or stop clocking the FLASH, practically resetting all code progress within the device. There are altogether 11 different low power modes within the U031 with where the most extreme type - “shutdown mode” - will reset everything in the mcu, practically doing the same as if we were powering down the entire device.
Engaging with sleep and low power modes can be useful to save power, forcing the device to “chill” between actions. It must be done with caution though, since we might reset sections of our mcu that we might be in need of later when we would want to go back to “run mode”.
We will be using “standby mode” in this project. This low-power mode is identical to “shutdown mode”, except that the device will keep the SBF bit in the PWR.CR1 register between sleep sessions. This is useful since we can tell if the mcu was already awake before or not. This can be used to do some sensor initiation on the very first power-on (where SBF will be 0) followed by simply reading out the sensors when the mcu is woken up. The mcu will be sleeping between readouts though, saving power.

### Wake up! It’s past noon already!
The big difference between powering the device down and using sleep modes is that we can restart/wake-up the device without user input. As it goes, even in the most extreme cases of sleep, it is possible to keep some triggers of the mcu still powered. They can wait for some form of event or input which, once detected, will automatically force the mcu to return to full power mode – i.e., wake up.
There are two main ways to wake a device up from sleep:
1) We can put a trigger on a GPIO. This GPIO can either be an EXTI or one of the simpler WKUP pins. The WKUPs are directly connected to the PWR manager of the mcu and thus are a more efficient way to wake. It is just that WKUP pins are designated to certain GPIOs and can not be randomly selected, unlike EXTIs. The WKUP pin MUST BE activated in the PWR manager.
2) We can us an event generation from an RTC. This is used if we want our device to wake at regular intervals.
Of note, we can always wake up a device by doing a power-on reset (cut power and then reapply it) or by a system reset (pull the NRST reset pin HIGH), albeit these both demand an input from the user. The NRST pin is pretty much identical to the WKUP, except it does not need to be activated. Mind, a power-on reset wipes all volatile elements within the mcu while system reset does not.
Here we will be using WKUP1 on the PA0 GPIO and an event generated every 4 seconds to be our wake-up sources.

### Get real
RTC – or real time clock – is a timer that is intentionally disconnected from the mcu, meaning that it has a separate counter, a (potentially) separate source of clocking and a (potentially) separate power source. We can choose if we want to clock it through the LSI or the LSE, plus it is powered through Vdd or Vbat, depending on which has more voltage on it.
Why is that useful? Well, since it is liberated from all other mcu functions, it can run independent of it. This is very useful if we want to track the passage of time or store a calendar on the device. Both will be done without the mcu doing anything – or even being powered on. An example would be the RTC always being connected to a battery while the mcu having a power-on switch.
If it isn’t obvious already, since the RTC is completely independent of the mcu, it can be used as a wake-up source for the device. It can send a flag when a certain date and time is reached or if its internal timer has ticked over, similar to how a normal timer would generate an interrupt.
In some cases, the RTC can be installed externally to the mcu as a small extra IC. In our case though, the RTC is implemented within the mcu thus we can use that instead. This means that we can do internal RTC signals and events and not rely on any GPIOs.

## Previous relevant projects
There isn’t really any relevant project from the past that I can recommend.

## To read
Checking the mcu refman is enough.

## Particularities
### Initiating the RTC
Like how the FLASH needs to be unlocked manually before any kind of writing, the RTC is also write protected when the device is powered on. This is to avoid accidentally corrupting the timing data. If we want to update our RTC – change the date in it or update the timer – we need to unlock the write protection on the registers first. Failing to do so will not lead to an error message though: the register will simply not be updated, keeping us stuck with the original value.
To unlock the RTC – and allow any of its registers to be updated – we will need to set the “DBP” bit within the PWR.CR1 register, followed by writing 0xCA and 0x53 to the RTC.WPR register. Once done, we will be able to modify the RTC registers. Of note, the RTC-related RCC register is also write protected before the unlock (RCC.BDCR).

### RTC clocking
The RTC can be clocked from various sources. We can select it by modifying the RCC.BDCR register. Usually, it is recommended to use the LSE – that is, an external 32.768 kHz crystal – since this truly makes the RTC autonomous from the mcu. Here we will be using the LSI instead though since I am not sure that the U031R8 nucleo board  has an LSE connected (the L053R8 does have one).
Apart from that, we can define the actual clocking of the RTC by selecting pre-scaling or downright defining it to be 1 Hz (see WUCKSEL bits in RTC.CR register). We will be using 1/16 pre-scaling here, putting the RTC clock at 2.048 kHz, then define the WUT value in RTC.WUTR register as 8096. This will generate a WUTF wake-up flag on the RTC internal wakeup line (WUTIE) every 4 seconds.

### Setting up the wake-ups
The WUTF flag will be generated automatically by the RTC and wake the mcu if WUTIE is enabled (see RTC.CR register).
The WKUP1 is activated separately in the PWR.CR3 register and will wake the mcu every time when there is a rising edge detected on that pin (the edge detection can be selected in PWR.CR4 for the pin).
Be aware though that the flags for both the RTC-based WUTF flag and the WKUP-based WUF flag must be reset after each a wake-up. Failing to do so will either lead to an immediate wake-up after engaging standby (in the case of the RTC) or failing to wake-up entirely (in the case of WKUP).
Lastly, it must be mentioned that the WUTF and the WUF flags are independent of the state of the mcu. This means that they WILL reset the mcu when activated, even if the mcu isn’t sleeping. It is highly recommended to deactivate both wake-up capabilities and reset them upon wake up and only re-activate them before putting back the mcu to sleep.

### Going to sleep
Putting the mcu to sleep is rather straight forwards. We need to set the designated “sleepdeep” bit in the SCB.SCR core register (it is a CORE REGISTER and not an STM32 one!), plus select the low power mode we wish to engage in the PWR.CR1 register (called LPMS bits in the U031, called PDDS in the L053). Lastly, we ought to wait until all ongoing actions are finished (“__force_stores();” assembly command), followed by the “__WFI();” assembly command to activate the mode.
Mind, if any of the elements above are not properly set, the device will still go to a low power mode, just not necessarily the one we want to. It is highly recommended to check the current running through the mcu using the designated pins (JP5 on the U031R8, JP6 on the L053R8). In standby mode, the current should be only a few uA-s compared to other low power modes in the range of mAs.
Emerging from standby is the same as a full system reset.

## User guide
When the mcu is powered on, we should be able to see the in-built LED blinking for 1 second every 4 seconds. We can manually turn it on by pulling the PA0 pin HIGH.
There is a printf to indicate if we are waking up or starting the first time.

## Conclusion
This is just a quick mini project to document some realisations I had regarding while playing around with low power modes and wake-ups.
