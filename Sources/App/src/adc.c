/**
  ******************************************************************************
  * @file           : adс.c
  * @brief          : Данный файл содержит функции по работе с оцифрованными 
	*										значениями электрических параметров
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "adc.h"
#include "sw_ctrl.h"
#include "rtos.h"
#include "periph.h"
/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define FL_RMS_WR(fl_rms) (fl_rms/5 > 1 ?  ((fl_rms/5) - 1) : (fl_rms/5))
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
// Массив ПДП для мгновенно оцифрованных данных с АЦП	
ADCPoll_TypeDef	adc_poll;				
// Мгновенное оцифрованные данные с АЦП
InstantData_TypeDef	adc_res;				
// Данные о средних значениях параметров
ElectricalData_TypeDef	avg_data;				
// Данные о действующих значениях параметров
ElectricalData_TypeDef 	rms_data[5];		
// Данные о действующих значениях параметров	
ElectricalData_TypeDef 	fl_rms_data;		
// Переменная с разрешениеми работы АЦП			
osSignals_t	adc_work_permisson;		
// Массив коэффициентов для параметрического перевода токов
uint16_t	current_conv_coef[5] = {Koef_I5, Koef_I4,	Koef_I3,	Koef_I2,	Koef_I1};
/* Private function prototypes -----------------------------------------------*/
static 	osSignals_t xAvgCalculation		(InstantData_TypeDef *adc, ElectricalData_TypeDef *avg);
static 	osSignals_t xRMSCalculation		(InstantData_TypeDef *adc, ElectricalData_TypeDef *avg, ElectricalData_TypeDef *rms);
static 	osSignals_t xFlRMSCalculation	(ElectricalData_TypeDef rms[5], ElectricalData_TypeDef *fl_rms);

static 	void vWriteElectricalData	(ElectricalData_TypeDef *structure,uint16_t argument, uint16_t data);

static	uint16_t 	usSelectArgumentInst	(InstantData_TypeDef *structure, uint16_t argument, uint16_t degree);
				int16_t 	sSelectArgumentElec		(ElectricalData_TypeDef *structure, uint16_t argument);
/* Private user code ---------------------------------------------------------*/
/**
	* @brief Задача обработки результатов АЦП  
	* @param *arg - начальные аргументы
	* @retval none
  */
void vAdcDataHandler(void *arg)
{	
	static	uint8_t 				rms_collect;		// Счетчик накопленных действующих значений
	static	osStatus_t 			val_sem;					

	/* Сброс количества накопленных действующих значений */
	rms_collect = 0;
	/* Разрешаем работу АЦП(программное разрешение) */
	adc_work_permisson = adcEnable;
	/* Включение таймера */
	HAL_TIM_Base_Start_IT(&htim6);
	for(;;)
	{	
		val_sem = osSemaphoreAcquire(adc_items_s, osWaitForever);
		if (val_sem == osOK)
		{					
			val_sem = osSemaphoreAcquire(adc_mutex_s, osWaitForever);
			if (val_sem == osOK)
			{				
				/* Рассчет среднего значения */
				xAvgCalculation(&adc_res, &avg_data);
				/* Расчет действующего значения */
				xRMSCalculation(&adc_res, &avg_data, &rms_data[rms_collect]);
				/* Релиз семафора, так как закончена работа с общим ресурсом */
				osSemaphoreRelease(adc_mutex_s);
				/* Нарощение кол-ва рассчитанных значений действующего значения */
				rms_collect++;														
				/* Фильтрация действующего значения */
				if ( rms_collect == 5)
				{
						val_sem = osSemaphoreAcquire(adc_writer_s, osWaitForever);
						if (val_sem == osOK)
						{				
							//Фильтрация действующих значений
							rms_collect = 0;
							xFlRMSCalculation(rms_data, &fl_rms_data);									
							osSemaphoreRelease(adc_writer_s);
						}							
				}					
				/* Разрешение работы АЦП */
				adc_work_permisson = adcEnable;
			}
			osSemaphoreRelease(adc_items_s);
			osThreadYield();
		}
	}
}

/**
	* @brief  Печать действующего значения в гипертерминал
	* @param *arg - начальные аргументы
	* @retval none
  */
void vPrintResult(void *arg)
{	
	static	osStatus_t 			val_sem;					
	static	uint32_t 				flag;

	for(;;)
	{
		val_sem = osSemaphoreAcquire(adc_reader_s ,500);
		if (val_sem == osOK)			
		{
			flag = osEventFlagsWait(evt_adc_print, adcRmsPrint, osFlagsWaitAny, 10);
			if ( flag == adcRmsPrint)
			{
				osEventFlagsClear( evt_adc_print, adcRmsPrint);
				val_sem = osSemaphoreAcquire(adc_writer_s ,osWaitForever);
				if (val_sem == osOK)			
				{
					osSemaphoreRelease(adc_reader_s);
					
					printf("\n ________ RMS ________\n"); //22
					for (uint8_t argument = I5A; argument <= I5C; argument+=5)
					{
						/* Печать текущего раздела структуры */
						switch (argument)	
						{
							case (I5A):
								printf("PHASE A CURRENTS\n"); // 18
								for (uint8_t i = I5A; i <= I1A; i++)
									printf("I%d = %f:5:3\n",abs(i-5),(double)sSelectArgumentElec(&fl_rms_data,argument)*current_conv_coef[i]);				
								break;					
							case (I5B):
								printf("PHASE B CURRENTS\n");
								for (uint8_t i = I5B; i <= I1B; i++)
									printf("I%d = %f:5:3\n",abs(i-10),(double)sSelectArgumentElec(&fl_rms_data,argument)*current_conv_coef[i-5]);				
								break;
							case (I5C):
								printf("PHASE C CURRENTS\n");
								for (uint8_t i = I5C; i <= I1C; i++)
									printf("I%d = %f:5:3\n",abs(i-15),(double)sSelectArgumentElec(&fl_rms_data,argument)*current_conv_coef[i-10]);				
								break;
							default:
								break;
						}
					}
					for (uint8_t argument = V1A; argument <= V2C; argument+=2)
					{
						/* Печать текущего раздела структуры */
						switch (argument)
						{
							case (V1A):
								printf("PHASE A VOLTAGE\n"); // 18
								for (uint8_t i = V1A; i <= V2A; i++)
								printf("V%d = %f:5:3\n",abs(i-17),(double)sSelectArgumentElec(&fl_rms_data,argument)*Koef_V);				
								break;					
							case (V1B):
								printf("PHASE B VOLTAGE\n");
								for (uint8_t i = V1B; i <= V2B; i++)
									printf("V%d = %f:5:3\n",abs(i-19),(double)sSelectArgumentElec(&fl_rms_data,argument)*Koef_V);				
								break;
							case (V1C):
								printf("PHASE C VOLTAGE\n");
								for (uint8_t i = V1C; i <= V2C; i++)
									printf("V%d = %f:5:3\n",abs(i-21),(double)sSelectArgumentElec(&fl_rms_data,argument)*Koef_V);				
								break;
							default:
								break;
						}
					}	
					
					val_sem = osSemaphoreAcquire(adc_reader_s ,osWaitForever);
					if (val_sem == osOK)			
						osSemaphoreRelease(adc_writer_s);
				}				
			}
			osSemaphoreRelease(adc_reader_s);
		}
//		osDelay(500);
	}
}		

/**
	* @brief  Функция вызываемая при возникновении прерывания по событию таймера 6
	* @param  htim - указатель на структуру таймера
	* @retval none
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	// Счетчик электрического градуса для снятия значений по АЦП		
	static 	uint16_t 	degree;		
	InstantData_TypeDef * adc = &adc_res;
	
	/* Проверка, был ли запущен планировщик */
	if (osKernelGetState() == osKernelRunning) 
	{    
		/* Сброс угла при отсутствии разрешения работы */
		if (adc_work_permisson == adcDisable)
			degree = 0;
		
		if (adc_work_permisson == adcEnable)
		{	
			
			/* Старт преобразований */
			ADC_StartConversion(ADC1);
			ADC_StartConversion(ADC2);
			ADC_StartConversion(ADC3);
			ADC_StartConversion(ADC4);
			
			/* Ожидание окончания передачи по ПДП */
			while(DMA_GetFlagStatus(DMA1_FLAG_TC1) 	== 	RESET);
			while(DMA_GetFlagStatus(DMA2_FLAG_TC1)	==	RESET);
			while(DMA_GetFlagStatus(DMA2_FLAG_TC5)	==	RESET);
			while(DMA_GetFlagStatus(DMA2_FLAG_TC2)	==	RESET);			
			
			/* Распределение данных 
			 * по соответсвующим значениям 
			 */
			// ADC1  			
			adc->current_a.i5[degree] = adc_poll.adc1[0];
			adc->current_a.i4[degree] = adc_poll.adc1[1];
			adc->current_a.i3[degree] = adc_poll.adc1[2];
			adc->current_a.i2[degree] = adc_poll.adc1[3];
			// ADC2  
			adc->current_a.i1[degree] = adc_poll.adc2[0];
			adc->current_b.i5[degree] = adc_poll.adc2[1];
			adc->current_b.i4[degree] = adc_poll.adc2[2];
			adc->current_b.i3[degree] = adc_poll.adc2[3];
			adc->current_b.i2[degree] = adc_poll.adc2[4];
			adc->current_b.i1[degree] = adc_poll.adc2[5];
			adc->current_c.i3[degree] = adc_poll.adc2[6];
			// ADC3
			adc->current_c.i4[degree] = adc_poll.adc3[0];
			adc->voltage_a.v1[degree] = adc_poll.adc3[1];
			adc->voltage_c.v1[degree] = adc_poll.adc3[2];
			adc->current_c.i5[degree] = adc_poll.adc3[3];
			adc->current_c.i2[degree] = adc_poll.adc3[4];
			adc->voltage_a.v2[degree] = adc_poll.adc3[5];
			adc->voltage_b.v1[degree] = adc_poll.adc3[6];
			adc->voltage_b.v2[degree] = adc_poll.adc3[7];
			// ADC4
			adc->voltage_c.v2[degree] = adc_poll.adc4[0];
			adc->current_c.i1[degree] = adc_poll.adc4[1];
			/* Инкримент угла */				
			degree++;
			
			if ( degree == 360 )
			{											
				/* Блокировка работы АЦП */
				adc_work_permisson = adcDisable;
				/* Сброс кол-ва отсчитанных градусов */
				degree = 0;
				osMessageQueuePut(adc_queue, &adc_res, 0U, 0U);	
				/* Выдача семафора об окончании преобразований */
				osSemaphoreRelease(adc_mutex_s);
				
			}
		}
		osSemaphoreRelease(adc_items_s);
	}				
}

/**
  * @brief  Функция вызываемая при возникновении прерывания по событию аналогового WatchDog
	* @retval none
  */
void ADC_ShortCircuit_IRQHandler(void)
{
					uint32_t start_time 	= osKernelGetTickCount();
	static 	uint8_t 	call_cnt 		= 0;
	
	/* Сброс флагов */
	if(ADC_GetFlagStatus(ADC1, ADC_FLAG_AWD1) != RESET)
		ADC_ClearITPendingBit(ADC1, ADC_IT_AWD1); 
	if(ADC_GetFlagStatus(ADC2, ADC_FLAG_AWD2) != RESET)
		ADC_ClearITPendingBit(ADC2, ADC_IT_AWD2);
	if(ADC_GetFlagStatus(ADC3, ADC_FLAG_AWD3) != RESET)
		ADC_ClearITPendingBit(ADC3, ADC_IT_AWD3);
	
 /* Если программа зашла в данное прерывание это значит случилось КЗ
	* Если за секунду обработчик вызовется 10 раз, это значит необходимо снимать управление
	*/
	/* Проверка на прохождение минуты */
	if ((call_cnt) && (osKernelGetTickCount() - start_time >= 1000))
		call_cnt = 0;
	/* Начальное время КЗ */
	if (!call_cnt)
		start_time 	= osKernelGetTickCount();
	/* Проверка на достижение небходимого числа вызовов */
	if (++call_cnt == IRQ_KZ_COUNT){
		for( uint8_t phase = PhaseA; phase <= PhaseC; phase++){
			PrevStage[phase] = CurStage[phase];
			CurStage[phase]	= 0;                        
		}
		osEventFlagsSet(evt_sw_ch, swChangePend);	
	}
}

/**
	* @brief  Расчет среднего значения
	* @param  *adc_data - указатель на структуру с данными из АЦП
	* @retval *temp_avg_data - структура со средними значениями эл. параметров
  */
static	osSignals_t xAvgCalculation(InstantData_TypeDef *adc, ElectricalData_TypeDef *avg)
{
	static int32_t	sum_avr;
	
	for (uint8_t argument = I5A; argument <= V2C; argument++)
	{
		sum_avr = 0;
		for(uint16_t degree = 0; degree < 360; degree++)
			sum_avr = sum_avr + (usSelectArgumentInst(adc,argument,degree));
		vWriteElectricalData(avg, argument, (sum_avr/360));
	}
	return adcAvgCalc;
}

/**
	* @brief  Расчет действующего значения
	* @param  *avg 			- указатель на структуру со средними значениями
	*					*adc_data - указатель на структуру с данными из АЦП
	* @retval *temp_avg_data - структура с действющими значениями эл. параметров
  */
static	osSignals_t xRMSCalculation(InstantData_TypeDef *adc, ElectricalData_TypeDef *avg, ElectricalData_TypeDef *rms)
{
	static int32_t	sum_rms;
	static int32_t	fl_res;
	
	for (uint8_t argument = I5A; argument <= V2C; argument++)
	{
		sum_rms = 0;
		/* Фильтрация значений с 0 до 355 эл.градусов */
		for(uint16_t degree = 0; degree < 355; degree++)
		{		
			fl_res = 0;
			
			for (uint8_t i = 0; i < 5; i++)
				fl_res = fl_res + (usSelectArgumentInst(adc,argument,degree+i));				
			
			sum_rms = sum_rms + pow((fl_res/5 - (int32_t)sSelectArgumentElec(avg,argument)),2);
		}
		
		/* Фильтрация значений с 355 до 360 эл.градусов */
		for(uint16_t degree = 355; degree < 360; degree++)
		{		
			fl_res = 0;
	
			for (uint8_t i = 0; i < 360 - degree; i++)
				fl_res = fl_res + (usSelectArgumentInst(adc,argument,degree+i));
			
			for (uint8_t i = 0; i < degree - 355; i++)
				fl_res = fl_res + (usSelectArgumentInst(adc,argument,i));
			
			sum_rms = sum_rms + pow((fl_res/5 - (int32_t)sSelectArgumentElec(avg,argument)),2);
		}
		
		vWriteElectricalData(rms, argument, sqrt(sum_rms/(360)));
		
	}	
	return adcRmsCalc;
}

/**
	* @brief  Фильтрация  действующего значения
	* @param  *rms	- указатель на массив действующих значений
	* @retval none
  */
static	osSignals_t xFlRMSCalculation(ElectricalData_TypeDef *rms, ElectricalData_TypeDef *fl_rms)
{
	 int32_t 		sum_rms = 0;
	 uint8_t		argument;
	
	for (argument = I5A; argument <= V2C; argument++)
	{
		for (uint8_t i = 0; i < 5; i++)
		{
			sum_rms = sum_rms + sSelectArgumentElec(&rms[i],argument);
		}	
		
		vWriteElectricalData(fl_rms,argument,FL_RMS_WR(sum_rms));
		sum_rms = 0;
		
	}
	return adcFlRmsCalc;
}


/**
	* @brief  Выбор рассчитываемого параметра из структуры типа xInstantAdcData
	* @param  *structure	- указатель на структуру с данными из АЦП
	*					argument 		- выбираемый электрический параметр (ток , напряжение на фазе х)
	*					degree 			-	Момент электрического градуса
	* @retval Значение эл.параметра в момент degree эл.градуса
  */
static	uint16_t usSelectArgumentInst(InstantData_TypeDef *structure, uint16_t argument, uint16_t degree)
{
	switch(argument)
	{
		/* Фаза A */
		case (I5A):
			return structure->current_a.i5[degree];
		case (I4A):
			return structure->current_a.i4[degree];
		case (I3A):
			return structure->current_a.i3[degree];
		case (I2A):
			return structure->current_a.i2[degree];
		case (I1A):
			return structure->current_a.i1[degree];
		case (V1A):
			return structure->voltage_a.v1[degree];
		case (V2A):
			return structure->voltage_a.v2[degree];

		/* Фаза B */
		case (I5B):
			return structure->current_b.i5[degree];
		case (I4B):
			return structure->current_b.i4[degree];
		case (I3B):
			return structure->current_b.i3[degree];
		case (I2B):
			return structure->current_b.i2[degree];
		case (I1B):
			return structure->current_b.i1[degree];
		case (V1B):
			return structure->voltage_b.v1[degree];
		case (V2B):
			return structure->voltage_b.v2[degree];
		
		/* Фаза C */
		case (I5C):
			return structure->current_c.i5[degree];
		case (I4C):
			return structure->current_c.i4[degree];
		case (I3C):
			return structure->current_c.i3[degree];
		case (I2C):
			return structure->current_c.i2[degree];
		case (I1C):
			return structure->current_c.i1[degree];
		case (V1C):
			return structure->voltage_c.v1[degree];
		case (V2C):
			return structure->voltage_c.v2[degree];
	}
	return 0;
}

/**
	* @brief  Выбор рассчитываемого параметра из структуры типа xElectricalData
	* @param  *structure	- указатель на структуру с данными из АЦП
	*					argument 		- выбираемый электрический параметр (ток , напряжение на фазе х)
	*					degree 			-	Момент электрического градуса
	* @retval Значение эл.параметра в момент degree эл.градуса
  */
int16_t sSelectArgumentElec(ElectricalData_TypeDef *structure, uint16_t argument)
{
	switch(argument)
	{
		/* Фаза A */
		case (I5A):
			return structure->current_a.i5;
		case (I4A):
			return structure->current_a.i4;
		case (I3A):
			return structure->current_a.i3;
		case (I2A):
			return structure->current_a.i2;
		case (I1A):
			return structure->current_a.i1;
		case (V1A):
			return structure->voltage_a.v1;
		case (V2A):
			return structure->voltage_a.v2;

		/* Фаза B */
		case (I5B):
			return structure->current_b.i5;
		case (I4B):
			return structure->current_b.i4;
		case (I3B):
			return structure->current_b.i3;
		case (I2B):
			return structure->current_b.i2;
		case (I1B):
			return structure->current_b.i1;
		case (V1B):
			return structure->voltage_b.v1;
		case (V2B):
			return structure->voltage_b.v2;
		
		/* Фаза C */
		case (I5C):
			return structure->current_c.i5;
		case (I4C):
			return structure->current_c.i4;
		case (I3C):
			return structure->current_c.i3;
		case (I2C):
			return structure->current_c.i2;
		case (I1C):
			return structure->current_c.i1;
		case (V1C):
			return structure->voltage_c.v1;
		case (V2C):
			return structure->voltage_c.v2;
	}
	return 0;
}

/**
	* @brief  Запись результата или данных в определеную ячейку структуру xElectricalData
	* @param  argument 	- выбираемый электрический параметр (ток , напряжение на фазе х)
	*					data			- Данные для записи в структуру типа xElectricalData
	* @retval structure - структура xElectricalData с заполненой ячейкой
  */
static	void vWriteElectricalData(ElectricalData_TypeDef *structure, uint16_t argument, uint16_t data)
{
	switch(argument)
	{
		/* Фаза A */
		case (I5A):
		{
			structure->current_a.i5 = data;
			break;
		}
		case (I4A):
		{
			structure->current_a.i4 = data;
			break;
		}
		case (I3A):
		{
			structure->current_a.i3 = data;
			break;
		}
		case (I2A):
		{
			structure->current_a.i2 = data;
			break;
		}
		case (I1A):
		{
			structure->current_a.i1 = data;
			break;
		}
		case (V1A):
		{
			structure->voltage_a.v1 = data;
			break;
		}
		case (V2A):
		{
			structure->voltage_a.v2 = data;
			break;
		}

		/* Фаза B */
		case (I5B):
		{
			structure->current_b.i5 = data;
			break;
		}
		case (I4B):
		{
			structure->current_b.i4 = data;
			break;
		}
		case (I3B):
		{
			structure->current_b.i3 = data;
			break;
		}
		case (I2B):
		{
			structure->current_b.i2 = data;
			break;
		}
		case (I1B):
		{
			structure->current_b.i1 = data;
			break;
		}
		case (V1B):
		{
			structure->voltage_b.v1 = data;
			break;
		}
		case (V2B):
		{
			structure->voltage_b.v2 = data;
			break;
		}
		
		/* Фаза C */
		case (I5C):
		{
			structure->current_c.i5 = data;
			break;
		}
		case (I4C):
		{
			structure->current_c.i4 = data;
			break;
		}
		case (I3C):
		{
			structure->current_c.i3 = data;
			break;
		}
		case (I2C):
		{
			structure->current_c.i2 = data;
			break;
		}
		case (I1C):
		{
			structure->current_c.i1 = data;
			break;
		}
		case (V1C):
		{
			structure->voltage_c.v1 = data;
			break;
		}
		case (V2C):
		{
			structure->voltage_c.v2 = data;
			break;
		}
	}
}
/**
* @brief  Запись результата или данных в определеную ячейку структуру xInstantAdcData
* @param  argument 	- выбираемый электрический параметр (ток , напряжение на фазе х)
*					data			- Данные для записи в структуру типа xInstantAdcData
*					degree 		-	Момент электрического градуса
* @retval *structure - структура xInstantAdcData с заполненой ячейкой

void vWriteInstantData(InstantData_TypeDef *structure, uint16_t argument, uint16_t data, uint16_t degree)
{	
	switch(argument)
	{
		// Фаза A 
		case (I5A):
		{
			structure->current_a.i5[degree] = data;
			break;
		}
		case (I4A):
		{
			structure->current_a.i4[degree] = data;
			break;
		}
		case (I3A):
		{
			structure->current_a.i3[degree] = data;
			break;
		}
		case (I2A):
		{
			structure->current_a.i2[degree] = data;
			break;
		}
		case (I1A):
		{
			structure->current_a.i1[degree] = data;
			break;
		}
		case (V1A):
		{
			structure->voltage_a.v1[degree] = data;
			break;
		}
		case (V2A):
		{
			structure->voltage_a.v2[degree] = data;
			break;
		}

		// Фаза B 
		case (I5B):
		{
			structure->current_b.i5[degree] = data;
			break;
		}
		case (I4B):
		{
			structure->current_b.i4[degree] = data;
			break;
		}
		case (I3B):
		{
			structure->current_b.i3[degree] = data;
			break;
		}
		case (I2B):
		{
			structure->current_b.i2[degree] = data;
			break;
		}
		case (I1B):
		{
			structure->current_b.i1[degree] = data;
			break;
		}
		case (V1B):
		{
			structure->voltage_b.v1[degree] = data;
			break;
		}
		case (V2B):
		{
			structure->voltage_b.v2[degree] = data;
			break;
		}
		
		// Фаза C 
		case (I5C):
		{
			structure->current_c.i5[degree] = data;
			break;
		}
		case (I4C):
		{
			structure->current_c.i4[degree] = data;
			break;
		}
		case (I3C):
		{
			structure->current_c.i3[degree] = data;
			break;
		}
		case (I2C):
		{
			structure->current_c.i2[degree] = data;
			break;
		}
		case (I1C):
		{
			structure->current_c.i1[degree] = data;
			break;
		}
		case (V1C):
		{
			structure->voltage_c.v1[degree] = data;
			break;
		}
		case (V2C):
		{
			structure->voltage_c.v2[degree] = data;
			break;
		}
	}
}
*/
