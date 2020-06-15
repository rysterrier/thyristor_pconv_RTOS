/**
  ******************************************************************************
  * @file           : uart4.c
  * @brief          : Данный файл содержит функции по работе с UART4
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "uart4.h"
#include "rtos.h"
#include "periph.h"
#include "sw_ctrl.h"
/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CurrentStage		(CurStage == StageNull ? 0 : CurStage)
#define UART4_OE(State) HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12, (State == UART4_OUTPUT ? GPIO_PIN_SET : GPIO_PIN_RESET))
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/**
* @brief  Чтение команд из буфера приема
* @param  
* @retval none
  */
void vCommanRead(void *arg) 
{
	static	char 				msg[32];
	static	swStage_t		stg;
	static	swPhase_t		phase;
	
	memset(&msg, 0, sizeof(msg));
	osDelay(10);
	
	printf("Input command: ");	
	for(;;)
	{		
		
		/* Основной цикл для считывания команд */
    while (scanf("%s", msg)){
			/* Команда смены ступени */
			if (!strcmp(msg,"stage_chg")){                
				printf("Select phase: ");
				memset(&msg, 0, sizeof(msg));
				while (scanf("%s", msg)){
					if (strcmp(msg,"A")|| strcmp(msg,"B") || strcmp(msg,"C")){
						phase = msg[0] - 0x40;
						printf("Select stage: ");									
						while (scanf("%d",&stg)){
							if (stg >=1 && stg <= 6){
								if (CurStage[phase] == stg || CurStage[phase] == stg){
									printf("Stage is currently selected\n ");
									break;
								}	else {
									printf("Starting stage change from %d to %d\n ",CurStage[phase],stg);
									PrevStage[phase] = CurStage[phase];
									CurStage[phase]	= stg;                        
									osEventFlagsSet(evt_sw_ch, swChangePend);	
									break;
								}
							} else {
								printf("Error: phases A,B,C\n ");
								break;
							}
						}
					} else {
						printf("Error: stages comes in 1-6)\n ");
						break;
					}
				}
			}
			/* Печать значений с АЦП */
			else if (!strcmp(msg,"print_rms")){
				osEventFlagsSet(evt_adc_print, adcRmsPrint);
			}
			/* Помощь */
			else if (!strcmp(msg,"help")){
					printf("Commands: \n stage_chg - change stage\n print_rms - print RMS\n help - dispaly commands\n");  
			}
			else{
				printf("Cmd not recognized, type help to get cmd \n "); 
			}				
			
		}
		memset(&msg, 0, sizeof(msg));
	}
}		

/**
	* @brief  Функция отправки байта по UART 4
	* @param *arg - начальные аргументы
	* @retval none
  */
void vTransmitUART4(void *arg) 
{
	static	osStatus_t 	status;
	static	uint8_t UART4_Tx_data;
		
	osDelay(5);
	
	for(;;)
	{			
		// Получение данных из очереди
		status = osMessageQueueGet(uart4_tx_queue, &UART4_Tx_data, 0, osWaitForever); 
		// Проверка на корректное принятие данных
		if (status == osOK) 
		{			
				// Разворот порта на отправку
				UART4_OE(UART4_OUTPUT);
				// Вывод данных
				huart4.Instance->TDR = (UART4_Tx_data & (uint8_t)0xFFU);
				// Разворот порта на прием
				UART4_OE(UART4_INPUT);										
		}
	}		
}



/**
	* @brief Функция вызываемая при прерывании UART4.
	* @param huart - указатель на структуру UART .
  * @retval None
  */
void HAL_UART4_Custom_IRQHandler(UART_HandleTypeDef *huart)
{	
	uint8_t 	UART_Rx_data;
	uint32_t 	cr1its     = READ_REG(huart->Instance->CR1);	
	uint32_t 	isrflags   = READ_REG(huart->Instance->ISR);	
	uint32_t 	errorflags;
	
	/* If no error occurs */
	errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE| USART_ISR_ORE | USART_ISR_NE));
  if (errorflags == RESET)
  {
    /* UART in mode Receiver ---------------------------------------------------*/
    if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
    {
			/* Читаем принятый байт по UART4 */
			UART_Rx_data = (uint16_t) READ_REG(huart->Instance->RDR);		
      /* Занесение полученного байта в очередь */
			osMessageQueuePut(uart4_rx_queue, &UART_Rx_data, NULL, NULL);							
			return;
    }
  }  
	return;
}

/**
	* @brief Переопределение стандартной функции ввода
	* @param none
	* @retval Символ из очереди ввода
  */
int stdin_getchar(void)
{ 
		char	msg; 
	
		osStatus_t status = osMessageQueueGet(uart4_rx_queue, &msg, NULL, osWaitForever); 		
		if (status == osOK) 
			return((int)msg);  
		else  
			return ((int)NULL); 
}

/**
	* @brief  Переопределние стандартной функции вывода
	* @param ch - сивол для занесения в очередь вывода
	* @retval ch - сивол для занесения в очередь вывода
  */
int stdout_putchar(int ch)
{
	osMessageQueuePut(uart4_tx_queue, &ch, 0U, 0U);
	return ch;
}
