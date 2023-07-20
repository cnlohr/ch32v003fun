# System Clock configuration example  
This project demonstrates various system clock configurations for the CH32V003 MCU and shows a way to unbrick the MCU in case the debug system is not responding.  
### SysClk configuration
There are two main sources for the internal clock:  
1. HSI - internal 24MHz oscillator
2. HSE - external crystal or clock source  

HSE bypass has to be set if an external clock generator is used - supplying the clock signal  to the OSCI/PA1 pin.

These two sources can feed the PLL (x2) and once the PLL is stable it can be used as 3rd sysclk source.  

Configuring the clock happens in the `funconfig.h` file:  
```
#define FUNCONF_USE_HSE 1  			// external crystal on PA1 PA2
#define FUNCONF_USE_HSI 0    		// internal 24MHz clock oscillator
#define FUNCONF_USE_PLL 1 			// use PLL x2
#define FUNCONF_HSE_BYPASS 0 		// bypass the HSE when using an external clock source
									// requires enabled HSE
#define CH32V003        1
```
In the above example the MCU is clocked from the onboard 24MHz crystal multiplied x2 by the PLL resulting in a 48MHz system clock.  
Use `make flash monitor` to compile/upload the firmware and open the debug printf terminal. It should print out all the details about the current clock configuration:  
```
System Clock Configuration Example
====================================
HSI Enabled
HSI trim = 16
HSI cal = 36
HSE Enabled
PLL Enabled
PLL Source = HSE
Clk Security Enabled
HSE bypass Disabled
Sysclk source = PLL 
MCO output: Disabled
```  
This message is printed out via the SWIO debug line and using UART, with TX on pin PD5.
## More details about the clock system  
According to the [debug system datasheet](https://github.com/openwch/ch32v003/blob/main/RISC-V%20QingKeV2%20Microprocessor%20Debug%20Manual.pdf) for the CH32V003, the HSI is required to access the chip via SWIO pin. The examples provided by WCH confirm it, in their code configuring the HSE modes does not switch the HSI off, it's left always on. This poses a problem for low power projects when all non required subsystems should be turned off. The real measured difference between the HSE only and with HSI running in background was about 800µA.  
Once the HSI is disabled, the power cycled, the programmer no longer can access the chip via SWIO pin, resulting in error mesages and known "bricking". 
"Unbricking" using the available programmers and software in most cases works. However. i've encountered situations when only using a combination of WCHLinkE and the WCHLinkUtility software on Windows was able to revive the MCU.  
As a countermeasure the CSS (Clock Security System) is enabled and allows emergency HSI restore action. The CSS monitors the HSE and if it fails, the HSI is turned on back by the hardware and set as the sysclock source. This reenables the debug channel.  
A way to reliable "unbrick" the MCU if the HSI has been disabled is to **make the HSE fail by shorting the OSCI/PA1 pin to GND**. Once done, the MCU will run on half the clock rate if the PLL was enabled. HSI will be active and the used can access the chip using usual methods.  
## MCO - clock output  
MCU clock output (pin PC4) can be configure using the following funtion:

`MCO_cfg(cfg);`  
where cfg value is one from the list below:
```
MCO_DISABLED 
MCO_OUT_SYSCLK
MCO_OUT_HSI	
MCO_OUT_HSE	
MCO_OUT_PLL	
```  
## Custom NMI interrupt handler  
Uncommenting `#define USE_CUSTOM_NMI_HANDLER` enables the local custom NMI irq handler. The Non Maskabe Interrupt is triggered by the RCC Clock Security System when the HSE fails. It can be used for other tasks like clock recovery or other ways to detect the clock fail.  
## PA1 PA2 Testing  
PA1 and PA2 are the HSE pins. According to the datasheet the PA12_RM bit in the AFIO_PCFR1 register has to be 1 for the crystal to work. However, the tests show that the HSE is taking control over these pins no matter what the configuration is. 
The example performs a few configuration changes trying to make the HSE fail and trigger the NMI interrupt. Please report if it was successful.
## Reuse printf for UART  
Project also shows how to reuse the printf implementation for both channels: via debug line and UART. Experimenting with clock sources might end up with debug channel disabled, UART being the only way to get out the messages.  


