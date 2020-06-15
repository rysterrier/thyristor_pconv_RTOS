/**
  ******************************************************************************
  * @file           : rtos.h
  * @brief          : Заголовочный файл rtos.c.
  ******************************************************************************
	*
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTOS_H
#define __RTOS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "defines.h"
/* Private includes ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern	osEventFlagsId_t	evt_adc_print;	// Флаг событий для печати результатов АЦП
extern	osEventFlagsId_t 	evt_sw_ch;		// Флаг смены режима для ключей

extern	osSemaphoreId_t	adc_writer_s;	// Семафор для печати результатов означающий запись результатов
extern	osSemaphoreId_t	adc_reader_s;	// Семафор для печати результатов
extern	osSemaphoreId_t adc_items_s;	// Семафор для АЦП отражающий появление данных
extern	osSemaphoreId_t	adc_mutex_s;	// Семафор для АЦП отражающий окончание работы прерывания

extern	osThreadId_t	Sim800lInitHandle;		// Задача запуска SIM800L
extern	osThreadId_t 	Sim800lServerHandle;	// Задача отправки данных на сервер
extern	osThreadId_t 	Sim800lCommsHandle;		// Задача коммуникации с помощью SIM800L
extern	osThreadId_t 	TransmitUART4Handle;	// Задача передачи данных по UART4
extern	osThreadId_t 	AdcDataHandle;			// Задача обработки данных с АЦП
extern	osThreadId_t 	AdcPrintHandle;			// Задача печати данных с АЦП
extern	osThreadId_t 	CommandReadHandle;		// Задача общения
extern	osThreadId_t 	StageSwitchHandle;		// Задача переключения

extern 	osMutexId_t		sim800_ctrl_m;	// Мьютекс для Sim800l
extern	osMutexId_t		uart1_m;		// Мьютекс для UART1
extern 	osMessageQueueId_t 	adc_queue;	
extern	osMessageQueueId_t 	uart4_rx_queue; // Очередь для приема данных по UART4
extern	osMessageQueueId_t 	uart4_tx_queue; // Очередь для передачи данных по UART4
extern	osMessageQueueId_t 	uart1_rx_queue; // Очередь для приема данных по UART1
extern	osMessageQueueId_t 	uart1_tx_queue; // Очередь для передачи данных по UAR
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
osInitStatus_t osiFREERTOSInit(void);
uint32_t HAL_GetTick (void);
/* Private defines -----------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* __RTOS_H */

