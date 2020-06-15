# thyristor_pconv_RTOS
 Thyristor convertor with RTOS implementation

Master's thesis project on the implementation of real-time operating systems for thyristor convertor’s control system software.
This repository contains only code - full paper availible on demand.
Hardware used: Custom STM32F303VCT6 board
Software used: Keil MDK v5.27
About code:
	1) Uses STM32 HAL and StdPeriph Libraries.
	2) CMSIS RTOS v2 wrapper around all FreeRTOS API	
	3) 8 Tasks with 4 ISR
	4) Simple SIM800L user defined driver lib (see sim800.c for details)
	5) Some features haven't been tested properly 