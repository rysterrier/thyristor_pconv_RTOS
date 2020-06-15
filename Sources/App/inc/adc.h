/**
  ******************************************************************************
  * @file           : adc.h
  * @brief          : Заголовочный файл adc.c 
  *										Данный файл содержит функции по работе с оцифрованными 
	*										значениями электрических параметров
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H
#define __ADC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "defines.h"
/* Private includes ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern	ADCPoll_TypeDef	adc_poll;	// Массив ПДП для мгновенно оцифрованных данных с АЦП	

extern	InstantData_TypeDef	adc_res;	// Мгновенное оцифрованные данные с АЦП

extern	ElectricalData_TypeDef	avg_data;		// Данные о средних значениях параметров
extern	ElectricalData_TypeDef 	rms_data[5];	// Данные о действующих значениях параметров
extern	ElectricalData_TypeDef 	fl_rms_data;	// Данные о действующих значениях параметров	

extern	osSignals_t	adc_work_permisson;		// Переменная с разрешениеми работы АЦП			
extern	uint16_t	current_conv_coef[5];	// Массив коэффициентов для параметрического перевода токов
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
void vAdcDataHandler(void * arg);
void 	vPrintResult(void *arg);
void ADC_ShortCircuit_IRQHandler(void);
int16_t sSelectArgumentElec		(ElectricalData_TypeDef *structure, uint16_t argument);
/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H */

