/**
  ******************************************************************************
  * @file           : sw_ctrl.c
  * @brief          : Данный файл содержит высокоуровневые 
	*										функции управления ключами
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sw_ctrl.h"
#include "sw_ll_ctrl.h"
#include "rtos.h"
#include "adc.h"
/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
// Структура управления ступенями 
StageCtrl_TypeDef	StageCtrl[PHASES_CNT];
// Предыдущая ступень
swStage_t	PrevStage[PHASES_CNT];	
// Текущая ступень
swStage_t	CurStage[PHASES_CNT];		
// Напряжения для подбора необходимой фазы
uint16_t stg_voltages[MAX_STAGES+1]= {
	VOLTAGE_U00,
	VOLTAGE_U01,
	VOLTAGE_U12,
	VOLTAGE_U23,
	VOLTAGE_U34,
	VOLTAGE_U45,
	VOLTAGE_U56,
	VOLTAGE_U60
};	

/* Private functions prototypes ----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/**
	* @brief 	Задача переключения ступеней
	* @param  *argument
	* @retval none
  */
void vSwitchStage (void *arg)
{
	/* Переменная для снятия флага */
	uint32_t flag = osNull;
	/* Переменная для выдерживания задержек */
	uint32_t start_time;
	/* Включение начальной ступени для всех фаз*/
	CurStage[PhaseA] = Stage0;
	CurStage[PhaseB] = Stage0;
	CurStage[PhaseC] = Stage0;
	
	for (swPhase_t phase_cur = PhaseA; phase_cur <= PhaseC; phase_cur++)
	{  
		StageCtrl[phase_cur].Phase		= phase_cur;
		StageCtrl[phase_cur].Status		= ON;
		StageCtrl[phase_cur].Stage		= CurStage[phase_cur];		
		xPhaseControl(StageCtrl);
		vVS7Switch(StageCtrl);
	}
	
	for(;;)
	{
		/* Ожидание сигнала о необходимости переключения */
			flag = osEventFlagsWait	(evt_sw_ch, swChangePend, osFlagsWaitAny, osWaitForever); 
			if (flag == swChangePend )	
			{	
					osEventFlagsClear( evt_sw_ch, swChangePend);
					/* Выключение прерывний */
					vDisableIRQn();
					/* Остановка планировщика */
					vTaskSuspendAll();
						
					/* ----------- Критическая секция ----------- */										
					/* Переключение по фазам */
					for (swPhase_t phase_cur = PhaseA; phase_cur <= PhaseC; phase_cur++)
					{  
						if ( CurStage[phase_cur] != PrevStage[phase_cur]){
							/* Статус флаг о нахождении нуля */
							osSignals_t	cross_flg	= osNull;
							/* Выбор активной фазы (А->B->C) */
							StageCtrl[phase_cur].Phase = phase_cur;
							
							/* Ожидаем окончания переходных процессов и выдерживаем задержку на пусковые токи */
							start_time = osKernelGetTickCount();
							while (osKernelGetTickCount() - start_time > 	StageCtrl[phase_cur].SP_SC_delay);
							
							/* Включение синхронизации */
							vSync(ON);
							/* Поиск нуля */
							cross_flg = xCrossSearch(StageCtrl);
							if (cross_flg == swErrorTimeout ||	cross_flg == swErrorUnknown)
							{	
								/* Таймаут поиска нуля */
								printf("Error Timeout");
								StageCtrl[phase_cur].Phase = PhaseNull;
								break;
							}
							else
							{
								/* Отключение синхронизации */	
								vSync(OFF);
								osDelay(15);		
								
								/* Снятие импульсов управления с текущей ступени */
								StageCtrl[phase_cur].Status	= OFF;
								StageCtrl[phase_cur].Stage	= PrevStage[phase_cur];
								vVS7Switch(StageCtrl);
								xPhaseControl(StageCtrl);
												
								/* Включение синхронизации */
								vSync(ON);
								/* Поиск нуля */
								cross_flg = xCrossSearch(StageCtrl);
								if (cross_flg == swErrorTimeout ||	cross_flg == swErrorUnknown)
								{	
									/* Таймаут поиска нуля */
									printf("Error Timeout");
									StageCtrl[phase_cur].Phase = PhaseNull;
									break;
								}
								else
								{
									osDelay(1);
									/* Подача импульсов управления с на новую ступень */								
									StageCtrl[phase_cur].Status	= ON;	
									StageCtrl[phase_cur].Stage	= CurStage[phase_cur];	
									xPhaseControl(StageCtrl);
									vVS7Switch(StageCtrl);
									
									/* Отключение синхронизации */
									vSync(OFF);
									StageCtrl[phase_cur].Phase = PhaseNull;
								}
							}		
						}							
					}			
					/* ----------- Критическая секция ----------- */
					/* Включение планировщика */
					xTaskResumeAll();
					/* Включение прерывний */
          vEnableIRQn();
			}
	}
}

/**
	* @brief 	Задача переключения ступеней
	* @param  *argument
	* @retval none
  */
void vAutomaticMode (void *arg)
{
	static	osStatus_t	val_sem;	
	static  int16_t			v1_rms[PHASES_CNT];
	for(;;){
		
		/* Захват семафоров для копиравания действующих значений для задачи */
		val_sem = osSemaphoreAcquire(adc_reader_s ,osWaitForever);
		if (val_sem == osOK){			
				val_sem = osSemaphoreAcquire(adc_writer_s ,osWaitForever);
				if (val_sem == osOK){
					osSemaphoreRelease(adc_reader_s);
					
					v1_rms[PhaseA] = fl_rms_data.voltage_a.v1;
					v1_rms[PhaseB] = fl_rms_data.voltage_b.v1;
					v1_rms[PhaseC] = fl_rms_data.voltage_c.v1;
					
					/* Отпускаем семафоры */	
					val_sem = osSemaphoreAcquire(adc_reader_s ,osWaitForever);
					if (val_sem == osOK)			
						osSemaphoreRelease(adc_writer_s);
				}				
			osSemaphoreRelease(adc_reader_s);
		}
		
		/* Поиск необходимой ступени для каждой из фаз*/
		for( uint8_t phase_cur = PhaseA; phase_cur <= PhaseC; phase_cur++){
			for (uint8_t stage = Stage0; stage <= Stage6; stage++){
				if ((stg_voltages[stage] < v1_rms[phase_cur] )&&( v1_rms[phase_cur] <= stg_voltages[stage+1])){
					if (CurStage[phase_cur] != stage){
							/* Выбираем задержку на пп и стартовые токи */						
							if (CurStage[phase_cur] < stage){
								StageCtrl[phase_cur].SP_SC_delay = SP_DELAY_UIN_DEC + SC_DELAY;							
							} else {
								StageCtrl[phase_cur].SP_SC_delay = SP_DELAY_UIN_INC + SC_DELAY;
							}
						PrevStage[phase_cur] 	= CurStage[phase_cur];
						CurStage[phase_cur] 	= stage;	
						/* Сигнал о неоходимости переключения */
						osEventFlagsSet(evt_sw_ch, swChangePend);
					}
				}							
			}
		}			
		osDelay(150);		
	}
	
}
