/**
  ******************************************************************************
  * @file   : rtos.c
  * @brief  : Данный файл содержит функции иницализации ОСРВ
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtos.h"
#include "uart4.h"
#include "adc.h"
#include "sw_ctrl.h"
#include "sim800.h"
/* Private function prototypes -----------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
osEventFlagsId_t 		evt_adc_print;	// Флаг событий для печати результатов АЦП
osEventFlagsId_t 		evt_sw_ch;		// Флаг смены режима для ключей

osSemaphoreId_t			adc_writer_s;	// Семафор для печати результатов означающий запись результатов
osSemaphoreId_t			adc_reader_s;	// Семафор для печати результатов
osSemaphoreId_t 		adc_items_s;	// Семафор для АЦП отражающий появление данных
osSemaphoreId_t			adc_mutex_s;	// Семафор для АЦП отражающий окончание работы прерывания

osThreadId_t 				TransmitUART4Handle;	// Задача передачи данных по UART4
osThreadId_t 				Sim800lInitHandle;		// Задача запуска SIM800L
osThreadId_t 				Sim800lServerHandle;	// Задача отправки данных на сервер
osThreadId_t 				Sim800lCommsHandle;		// Задача коммуникации с помощью SIM800L
osThreadId_t 				AdcDataHandle;			// Задача обработки данных с АЦП
osThreadId_t 				AdcPrintHandle;			// Задача печати данных с АЦП
osThreadId_t 				CommandReadHandle;		// Задача общения с пользователем
osThreadId_t 				StageSwitchHandle;		// Задача переключения

osMutexId_t					sim800_ctrl_m;	// Мьютекс для Sim800l
osMutexId_t					uart1_m;		// Мьютекс для UART1

osMessageQueueId_t 	adc_queue;	// Очередь для приема данных по UART4
osMessageQueueId_t 	uart4_rx_queue;	// Очередь для приема данных по UART4
osMessageQueueId_t 	uart4_tx_queue;	// Очередь для передачи данных по UART4
osMessageQueueId_t 	uart1_rx_queue;	// Очередь для приема данных по UART1
osMessageQueueId_t 	uart1_tx_queue;	// Очередь для передачи данных по UART1
/* Private user code ---------------------------------------------------------*/	
/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
osInitStatus_t osiFREERTOSInit(void) 
{
	osThreadAttr_t 	thread_attr ={
		.cb_mem = NULL,
		.cb_size = NULL,
		.stack_mem = NULL	
	};
	
	osMutexAttr_t mutex_attr={
		"Sim800_Mutex",
		osMutexPrioInherit,
		NULL,
		0U
	};

	/* RTOS_MPOOLS */
	
  /* RTOS_MUTEX */
	sim800_ctrl_m = osMutexNew(&mutex_attr);
	if (sim800_ctrl_m == NULL) return (osiErrorMutex);
	uart1_m = osMutexNew(&mutex_attr);
	if (uart1_m == NULL) return (osiErrorMutex);
	
  /* RTOS_SEMAPHORES */
	adc_items_s  = osSemaphoreNew(1U, 0U, NULL);
	if (adc_items_s  == NULL) return (osiErrorSemaphore);	
	adc_mutex_s  = osSemaphoreNew(1U, 0U, NULL);
	if (adc_mutex_s  == NULL) return (osiErrorSemaphore);	
	adc_writer_s  = osSemaphoreNew(1U, 1U, NULL);
	if (adc_writer_s  == NULL) return (osiErrorSemaphore);	
	adc_reader_s  = osSemaphoreNew(4U, 4U, NULL);
	if (adc_reader_s  == NULL) return (osiErrorSemaphore);
	
	/* RTOS_FLAGS */
	evt_adc_print = osEventFlagsNew(NULL);
	if (evt_adc_print == NULL) return osiErrorFlag;
	evt_sw_ch = osEventFlagsNew(NULL);
	if (evt_sw_ch == NULL) return osiErrorFlag;

  /* RTOS_TIMERS */

  /* RTOS_QUEUES */
	adc_queue = osMessageQueueNew(RX_QUEUE_SIZE, sizeof(InstantData_TypeDef*), NULL);
	if (adc_queue 		== NULL) return (osiErrorMessageQ);	
	uart4_rx_queue 	= osMessageQueueNew(RX_QUEUE_SIZE, sizeof(uint8_t), NULL);
	if (uart4_rx_queue 		== NULL) return (osiErrorMessageQ);	
	uart4_tx_queue 	= osMessageQueueNew(TX_QUEUE_SIZE, sizeof(uint8_t), NULL);
	if (uart4_tx_queue 	== NULL) return (osiErrorMessageQ);
		
  /* RTOS_THREADS */ 	

  /* Передача через UART4 */
	thread_attr.name = "UART4Send_Thread";
	thread_attr.priority	=	osPriorityBelowNormal;
	thread_attr.stack_size = 512;

  TransmitUART4Handle = osThreadNew(vTransmitUART4, NULL, &thread_attr);	
	if (TransmitUART4Handle == NULL) return (osiErrorThread);
	
//	/* SIM модуль */
//	thread_attr.name = "SIM800_Init";
//	thread_attr.priority	=	osPriorityNormal;
//	thread_attr.stack_size = 512;

//  Sim800lInitHandle = osThreadNew(vSim800InitThread, NULL, &thread_attr);	
//	if (Sim800lInitHandle == NULL) return (osiErrorThread);
//	
	/* Считывание комманд */
	thread_attr.name = "UART4Comms_Thread";
	thread_attr.priority	=	osPriorityNormal;
	thread_attr.stack_size = 512;
		
	CommandReadHandle = osThreadNew(vCommanRead, NULL,  &thread_attr);
	if (!CommandReadHandle) return (osiErrorThread);
	
//	/* Обработчик АЦП */
//	thread_attr.name = "ADCCalc_Thread";
//	thread_attr.priority	=	osPriorityLow;
//	thread_attr.stack_size = 1024;

//	AdcDataHandle = osThreadNew(vAdcDataHandler, NULL, &thread_attr);	
//	if (!AdcDataHandle) return (osiErrorThread);

//	/* Печать АЦП */
//	thread_attr.name = "ADCPrint_Thread";
//	thread_attr.priority	=	osPriorityBelowNormal;
//	thread_attr.stack_size = 512;
//	
//	AdcPrintHandle = osThreadNew(vPrintResult, NULL, &thread_attr);	
//	if (!AdcDataHandle) return (osiErrorThread);
//	  

//  /* Контроль за ступенями */
//	thread_attr.name = "SWCtrl_Thread";
//	thread_attr.priority	=	osPriorityRealtime ;
//	thread_attr.stack_size = 1024;
//		
//	StageSwitchHandle = osThreadNew(vSwitchStage, NULL,  &thread_attr);
//	if (!StageSwitchHandle) return (osiErrorThread);
//	
//	/* Контроль за ступенями */
//	thread_attr.name = "AutoCTRL_Thread";
//	thread_attr.priority	=	osPriorityHigh ;
//	thread_attr.stack_size = 512;
//		
//	StageSwitchHandle = osThreadNew(vAutomaticMode, NULL,  &thread_attr);
//	if (!StageSwitchHandle) return (osiErrorThread);
	
	return(osiOK);
}

/**
	* @brief  Provides a tick value in millisecond.
  * @note   The function is declared as __Weak  to be overwritten  in case of other 
  *         implementations  in user file.
  * @retval tick value
  */
uint32_t HAL_GetTick (void) 
{
    return ((uint32_t)osKernelGetTickCount());
}
