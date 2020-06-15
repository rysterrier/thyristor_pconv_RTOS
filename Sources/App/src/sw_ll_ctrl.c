/**
  ******************************************************************************
  * @file           : sw_ll_ctrl.c
  * @brief          : Данный файл содержит низкоуровневые 
	*										функции управления ключами
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sw_ll_ctrl.h"
#include "adc.h"
/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define I_KZ 844
#define SW_TIMEOUT_VALUE	20
/* Private macro -------------------------------------------------------------*/
#define	SearchStat(x) 				( x == swCrossNFnd ? swErrorTimeout : swCrossFound)
#define	CrossSrh(inst, avg)		( ((inst >= (avg - 20)) && (inst <= (avg + 20))) ? swCrossFound : swCrossNFnd) 
#define	SwStatus(stat)				( (stat == ON) ? swPortOn : swPortOff) 
/* Private variables ---------------------------------------------------------*/
/* Private functions prototypes ----------------------------------------------*/
static osSignals_t xSrhStructInit (uint16_t current, Cross_TypeDef* Cross_Struct);
static osSignals_t xSearchLoop 		(uint16_t current);
/* Private user code ---------------------------------------------------------*/
/**
* @brief 	Finds cross point of a certain stage 
* @param  PhaseSwitch - active phase
* @retval Flag that tells that zero has been found
* @note if 0 not found in 20ms return False + message in task
  */
osSignals_t xCrossSearch(StageCtrl_TypeDef *PhaseSwitch)
{   
	switch (PhaseSwitch->Phase)
	{
		
		/* ----- Phase A ----- */
		case PhaseA:
		{
			if (fl_rms_data.current_a.i5 > I_KZ)
			{
				if (PhaseSwitch->Stage == 0)
				{
					/* I2A */
          return xSearchLoop(I2A);       					
				}
				else if (PhaseSwitch->Stage != 0)
				{
					/* I4A */
          return xSearchLoop(I4A);       					
				}
			}			
			else
			{	
				if (PhaseSwitch->Stage == 0)
				{
					/* I1A */
          return xSearchLoop(I1A);       					
				}
				else if (PhaseSwitch->Stage != 0)
				{
					/* I3A */
          return xSearchLoop(I3A);       					
				}
			}
			break;
			
		}
		/* ----- Phase A ----- */
		case PhaseB:
		{
			if (fl_rms_data.current_b.i5 > I_KZ)
			{
				if (PhaseSwitch->Stage == 0)
				{
					/* I2B */
          return xSearchLoop(I2B);       					
				}
				else if (PhaseSwitch->Stage != 0)
				{
					/* I4B */
          return xSearchLoop(I4B);       					
				}
			}			
			else
			{	
				if (PhaseSwitch->Stage == 0)
				{
					/* I1B */
          return xSearchLoop(I1B);       					
				}
				else if (PhaseSwitch->Stage != 0)
				{
					/* I3B */
          return xSearchLoop(I3B);       					
				}
			}
			break;
			
		}
		/* ----- Phase C ----- */
		case PhaseC:
		{
			if (fl_rms_data.current_c.i5 > I_KZ)
			{
				if (PhaseSwitch->Stage == 0)
				{
					/* I2C */
          return xSearchLoop(I2C);       					
				}
				else if (PhaseSwitch->Stage != 0)
				{
					/* I4C */
          return xSearchLoop(I4C);       					
				}
			}			
			else
			{	
				if (PhaseSwitch->Stage == 0)
				{
					/* I1C */
          return xSearchLoop(I1C);       					
				}
				else if (PhaseSwitch->Stage != 0)
				{
					/* I3C */
          return xSearchLoop(I3C);       					
				}
			}
			break;
		}
	}

	return swErrorUnknown;
}


/**
* @brief 	Цикл поиска нуля
* @param  current - ток для которого идет поиск
* @retval none
  */
osSignals_t xSearchLoop (uint16_t current)	
{
		Cross_TypeDef Cross_Struct;
		osSignals_t		cross_flg	= swCrossNFnd;
	
		/* Инициализаци структуры для поиска нуля */
		xSrhStructInit(current,&Cross_Struct);
		/* Получение времени начала поска нуля	*/
		uint32_t xCrossSrhStartTime	=	osKernelGetTickCount();
	
		do
		{						
			ADC_StartConversion(Cross_Struct.ADCx);						
			while(DMA_GetFlagStatus(Cross_Struct.DMAx_FLAG) 	== 	RESET);			
			cross_flg = CrossSrh(Cross_Struct.ADCx_Poll,Cross_Struct.AVG_Phase_x); 						
		}while ( (cross_flg == swCrossNFnd)	|| (osKernelGetTickCount() - xCrossSrhStartTime != SW_TIMEOUT_VALUE)  );
			
		return SearchStat(cross_flg); 
	
}
/**
* @brief 	Инициализация структуры для поиска нуля
* @param  current - ток для которого необходимо иницализация
*					Cross_Struct - структура с параметрами для поиска
* @retval Код статус успешной инициализации
  */
osSignals_t xSrhStructInit (uint16_t current, Cross_TypeDef *Cross_Struct)	
{
	
	switch(current)
	{
		/* Фаза A */
		case (I4A):
		{
			Cross_Struct->ADCx					=	ADC1;
			Cross_Struct->ADCx_Poll			=	adc_poll.adc1[1];
			Cross_Struct->AVG_Phase_x		=	avg_data.current_a.i4;
			Cross_Struct->DMAx_FLAG			=	DMA1_FLAG_TC1;
			return swSrhInitOk;
		}
		case (I3A):
		{
			Cross_Struct->ADCx					=	ADC1;
			Cross_Struct->ADCx_Poll			=	adc_poll.adc1[2];
			Cross_Struct->AVG_Phase_x		=	avg_data.current_a.i3;
			Cross_Struct->DMAx_FLAG			=	DMA1_FLAG_TC1;
			return swSrhInitOk;
		}
		case (I2A):
		{
			Cross_Struct->ADCx					=	ADC1;
			Cross_Struct->ADCx_Poll			=	adc_poll.adc1[3];
			Cross_Struct->AVG_Phase_x		=	avg_data.current_a.i2;
			Cross_Struct->DMAx_FLAG			=	DMA1_FLAG_TC1;
			return swSrhInitOk;
		}
		case (I1A):
		{
			Cross_Struct->ADCx					=	ADC2;
			Cross_Struct->ADCx_Poll			=	adc_poll.adc2[0];
			Cross_Struct->AVG_Phase_x		=	avg_data.current_a.i1;
			Cross_Struct->DMAx_FLAG			=	DMA2_FLAG_TC1;
			return swSrhInitOk;
		}

		/* Фаза B */
		case (I4B):
		{
			Cross_Struct->ADCx					=	ADC2;
			Cross_Struct->ADCx_Poll			=	adc_poll.adc2[2];
			Cross_Struct->AVG_Phase_x		=	avg_data.current_b.i4;
			Cross_Struct->DMAx_FLAG			=	DMA2_FLAG_TC1;
			return swSrhInitOk;
		}
		case (I3B):
		{
			Cross_Struct->ADCx					=	ADC2;
			Cross_Struct->ADCx_Poll			=	adc_poll.adc2[3];
			Cross_Struct->AVG_Phase_x		=	avg_data.current_b.i3;
			Cross_Struct->DMAx_FLAG			=	DMA2_FLAG_TC1;
			return swSrhInitOk;
		}
		case (I2B):
		{
			Cross_Struct->ADCx					=	ADC2;
			Cross_Struct->ADCx_Poll			=	adc_poll.adc2[4];
			Cross_Struct->AVG_Phase_x		=	avg_data.current_b.i2;
			Cross_Struct->DMAx_FLAG			=	DMA2_FLAG_TC1;
			return swSrhInitOk;
		}
		case (I1B):
		{
			Cross_Struct->ADCx					=	ADC2;
			Cross_Struct->ADCx_Poll			=	adc_poll.adc2[5];
			Cross_Struct->AVG_Phase_x		=	avg_data.current_b.i1;
			Cross_Struct->DMAx_FLAG			=	DMA2_FLAG_TC1;
			return swSrhInitOk;
		}
		
		/* Фаза C */
		case (I4C):
		{
			Cross_Struct->ADCx					=	ADC3;
			Cross_Struct->ADCx_Poll			=	adc_poll.adc3[0];
			Cross_Struct->AVG_Phase_x		= avg_data.current_c.i4;
			Cross_Struct->DMAx_FLAG			=	DMA2_FLAG_TC5;
			return swSrhInitOk;
		}
		case (I3C):
		{
			Cross_Struct->ADCx					=	ADC3;
			Cross_Struct->ADCx_Poll			=	adc_poll.adc2[6];
			Cross_Struct->AVG_Phase_x		=	avg_data.current_c.i3;
			Cross_Struct->DMAx_FLAG			=	DMA2_FLAG_TC1;
			return swSrhInitOk;
		}
		case (I2C):
		{
			Cross_Struct->ADCx					=	ADC3;
			Cross_Struct->ADCx_Poll			=	adc_poll.adc3[4];
			Cross_Struct->AVG_Phase_x		=	avg_data.current_c.i2;
			Cross_Struct->DMAx_FLAG			=	DMA2_FLAG_TC5;
			return swSrhInitOk;
		}
		case (I1C):
		{
			Cross_Struct->ADCx					=	ADC4;
			Cross_Struct->ADCx_Poll			=	adc_poll.adc4[1];
			Cross_Struct->AVG_Phase_x		=	avg_data.current_c.i1;
			Cross_Struct->DMAx_FLAG			=	DMA2_FLAG_TC2;
			return swSrhInitOk;
		}
		default:
			return swErrorUnknown;
	}
}

/**
* @brief 	Sync on PD14
* @param  stat - статус синхронизации on/off
* @retval none
  */
osSignals_t vSync(swState_t stat)
{
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, (GPIO_PinState)stat);	
	return SwStatus(stat);
}

/**
* @brief VS7 switch func
* @param  PhaseSwitch - structure with phases and their status
* @retval none
  */
osSignals_t vVS7Switch(StageCtrl_TypeDef *PhaseSwitch)
{

		switch(PhaseSwitch->Phase)
		{
			/* ----- Phase A ----- */
			case (PhaseA):
			{
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, (GPIO_PinState)PhaseSwitch->Status);		
				return SwStatus(PhaseSwitch->Status);										
			}
			/* ----- Phase B ----- */	
			case (PhaseB):
			{
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, (GPIO_PinState)PhaseSwitch->Status);		
				return SwStatus(PhaseSwitch->Status);														
			}
				
			/* ----- Phase C ----- */	
			case (PhaseC):
			{
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, (GPIO_PinState)PhaseSwitch->Status);		
				return SwStatus(PhaseSwitch->Status);										
			}
			default: 
				return swPortError;					
		}		
}
/**
* @brief 	Stage switch func
* @param  PhaseSwitch - structure with phases and their status
* @retval none
  */
osSignals_t xPhaseControl(StageCtrl_TypeDef *PhaseSwitch)
{
		switch(PhaseSwitch->Phase)
		{
			/* ----- Phase A ----- */
			case (PhaseA):
			{
				switch(PhaseSwitch->Stage)
				{		
					case Stage0:
					{
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
					
					case Stage1:
					{
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
					
					case Stage2:
					{
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
					
					case Stage3:
					{
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
					
					case Stage4:
					{
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																									
					}
					
					case Stage5:
					{
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
					
					case Stage6:
					{
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
				}
			}
			/* ----- Phase B ----- */
			case (PhaseB):
			{
				switch(PhaseSwitch->Stage)
				{	
					case Stage0:
					{
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
					
					case Stage1:
					{
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
					
					case Stage2:
					{
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
							
					case Stage3:
					{
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																										
					}
					case Stage4:
					{
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
					
					case Stage5:
					{
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
					
					case Stage6:
					{
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																										
					}								
				}				
			}
			/* ----- Phase C ----- */
			case (PhaseC):
			{
				switch(PhaseSwitch->Stage)
				{
					case Stage0:
					{
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																										
					}
							
					case Stage1:
					{
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																									
					}
					
					case Stage2:
					{
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
					case Stage3:
					{
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);														
					}	
					
					case Stage4:
					{
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																										
					}	
					
					case Stage5:
					{
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
					
					case Stage6:
					{
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, (GPIO_PinState)PhaseSwitch->Status);		
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, (GPIO_PinState)PhaseSwitch->Status);		
						return SwStatus(PhaseSwitch->Status);																				
					}
				}											
			}
			default:
				return swPortError;
		}
}
/**
* @brief 	Отключение прерываний, способных повлиять 
*					на крит.секцию переключения ключей
* @param  none
* @retval none
  */
void vDisableIRQn(void)
{
	/* Отключение прерывания по событию таймера 6 */
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
	/* Отключение прерывания по событию UART4 */
	HAL_NVIC_DisableIRQ(UART4_IRQn);
	/* Отключение прерывания по событию USART1 */
	HAL_NVIC_DisableIRQ(USART1_IRQn);
	/* Отключение прерывания по событию DMA каналов */
	HAL_NVIC_DisableIRQ(DMA1_Channel4_IRQn);	
	HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
}

/**
* @brief 	Включение прерываний, способных повлиять 
*					на крит.секцию переключения ключей
* @param  none
* @retval none
  */
void vEnableIRQn(void)
{
	/* Включение прерывания по событию таймера 6 */
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
	/* Включение прерывания по событию UART4 */
	HAL_NVIC_EnableIRQ(UART4_IRQn);
	/* Включение прерывания по событию USART1 */
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	/* Включение прерывания по событию DMA каналов */
	HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);	
	HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
}

