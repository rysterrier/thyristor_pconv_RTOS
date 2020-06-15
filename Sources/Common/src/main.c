/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Данный файл содержит основную функцию системы main
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "periph.h"
#include "rtos.h"
#include "sim800.h"
/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/**
	* @brief  Точка входа в программу
  * @retval int
  */
int main(void)
{
  /* Конфигурация МК ---------------------------------------------------------*/
	/* Инициализация МК */
	vPeripheralInit();
		
	/* FREERTOS Configuration---------------------------------------------------*/
	/* Инициализация планировщика */
//	if (osKernelGetState() == osKernelInactive )  	                    
//		if (osKernelInitialize() != osOK)	
//			/* Ошибка: Планировщик не инициализирован */
			Error_Handler(errKernelInit);

	/* Инициализация объектов ОСРВ */
	osInitStatus_t stat = osiOK;
	stat = osiFREERTOSInit();	
  if (stat != osiOK)
		/* Ошибка: Объекты ОСРВ не инициализированы */
		Error_Handler(stat|errRTOSInit);
		
  /* Запуск планировщика */
	if (osKernelGetState() == osKernelReady)         
		if (osKernelStart() != osOK)					
			/* Ошибка: Планировщик не запустился */
			Error_Handler(errKernelStart);
	
		/* Бесконечный цикл */
	while(1);
	
}


void Error_Handler(FaultSignals_t err_code)
{		
	char * msg;
	msg = (char*) malloc(64);
	sprintf(msg,"\nError:0x%X\n",(err_code&osiErrorMask));
	switch (err_code&osiErrorMask)
	{
		case errOscInit:
			strcat(msg,"Osc init error\n HardFault\n");
			break;
		case errADCInit:
			strcat(msg,"ADC init error\n HardFault\n");
			break;
		case errDMAInit:
			strcat(msg,"DMA init error\n HardFault\n");
			break;
		case errTIMInit:
			strcat(msg,"TIM6 init error\n HardFault\n");
			break;
		case errUART1Init:
			strcat(msg,"UART1 init error\n HardFault\n");
			break;
		case errUART4Init:
			strcat(msg,"UART4 init error\n HardFault\n");
			break;
		case errKernelInit:
			strcat(msg,"Kernel init error\n HardFault\n");
			break;
		case errRTOSInit:
			strcat(msg,"RTOS init error\n ");
			switch (err_code&(!osiErrorMask))
			{
				case (osiErrorMessageQ):
						strcat(msg,"Msg_Q error\n ");
						break;
				case (osiErrorTimer):
						strcat(msg,"Timer error\n ");
						break;
				case (osiErrorThread):
						strcat(msg,"Thread error\n ");
						break;				
				case (osiErrorSemaphore):
						strcat(msg,"Semaph error\n ");
						break;
				case (osiErrorMPool):
						strcat(msg,"Mpool error\n ");
						break;
				case (osiErrorFlag):
						strcat(msg,"Flag error\n ");
						break;
				case (osiErrorMutex): 
						strcat(msg,"Mutex error\n ");
						break;
			}
			break;
		case errKernelStart:
			strcat(msg,"Kernel start error\n HardFault\n");
			break;
		case errSIMInit:
			strcat(msg,"SIM800L init err\n Restaring thread\n");
			printf(msg);
			free(msg);
			osThreadAttr_t 	thread_attr = {
				.name = "RST_SIM800",
				.cb_mem = NULL,
				.cb_size = NULL,				
				.stack_mem = NULL,	
				.stack_size = 128,
				.priority = osPriorityHigh				
			};			
			osThreadId_t	rstsim_handle;				
			rstsim_handle = osThreadNew(rst_sim800l, NULL,  &thread_attr);
			osThreadExit();
			return;
			break;
	}
		
	if (osKernelGetState() == osKernelRunning)
		taskENTER_CRITICAL();
		
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12, GPIO_PIN_SET);
	
	for (uint8_t i = 0; i < strlen(msg) ; i++)
		UART4->TDR = (*(msg+i) & (uint8_t)0xFFU);
	
	free(msg);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12, GPIO_PIN_RESET);
	
	if (osKernelGetState() == osKernelRunning)
		taskEXIT_CRITICAL();
	
	HardFault_Handler();
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
