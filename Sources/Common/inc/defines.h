/**
  ******************************************************************************
  * @file           : defines.h
  * @brief          : Header for defines.c file.
  *                   This file contains defines used in the application.
  ******************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DEFINE_H
#define __DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Подключение заголовочного файла стандартной библиотеки типов данных */
#include <stdint.h>
/* Подключение заголовочного файла стандартной библиотеки математических функций */
#include <math.h>
/* Подключение заголовочного файла стандартной библиотеки ввода/вывода */
#include <stdio.h>
/* Подключение заголовочного файла стандартной библиотеки для выделения памяти*/
#include <stdlib.h>
/* Подключение заголовочного файла стандартной библиотеки для работы со строками */
#include <string.h>
/* Подключение заголовочного файла стандартной библиотеки булевых операндов */
#include <stdbool.h>
/* Подключение заголовочного файла стандартной библиотеки определений функций */
#include <stdarg.h>

/* Подключение заголовочного файла HAL библиотеки перифирии МК */
#include "stm32f3xx_hal.h"
/* Подключение заголовочного файла Std_Periph_Lib АЦП */
#include "stm32f30x_adc.h"
/* Подключение заголовочного файла Std_Periph_Lib ПДП */
#include "stm32f30x_dma.h"
/* Подключение заголовочного функицй прерываний */
#include "stm32f3xx_it.h"
/* Подключение заголовочного файла API функций ОСРВ */
#include "cmsis_os2.h"
/* Подключение заголовочных файлов  ОСРВ */
#include "FreeRTOS.h"              
#include "task.h"                  
#include "semphr.h" 
#include "queue.h"
/* Exported types ------------------------------------------------------------*/
	
	/// Структура для мгновенных оцифрованных данных с АЦП
	/// \note: none \b: none
	typedef struct 
	{
		 uint16_t  adc1 [4];  
		 uint16_t  adc2 [7];  
		 uint16_t  adc3 [8];  
		 uint16_t  adc4 [2];  
	 } ADCPoll_TypeDef;

	/// Структура для обработанных данных с АЦП
	/// \note: Ix1 - датчик тока на входе преобразователя
	/// Ix2 - датчик тока обмотки W11, Ix3 - датчик тока обмотки W12
	/// Ix4 - датчик тока обмотки W13, Ix5 - датчик тока линии
	/// Vx1 - датчик напряжения на входе преобразователя
	/// Vx2 - датчик напряжения на выходе преобразователя 
	/// x - номер фазы \b: none	 
	typedef struct
	{
		 // Токи с фазы А
		 struct  {
			int16_t i5;
			int16_t i4;
			int16_t i3;
			int16_t i2;
			int16_t i1;		
		} current_a;
		// Токи с фазы В
		struct  {
			int16_t i5;
			int16_t i4;
			int16_t i3;
			int16_t i2;
			int16_t i1;		
		} current_b; 
		// Токи с фазы С
		struct  {
			int16_t i5;
			int16_t i4;
			int16_t i3;
			int16_t i2;
			int16_t i1;		
		} current_c; 
		// Напряжение с фазы А 
		struct  {
			int16_t v1;
			int16_t v2;			
		} voltage_a; 
		// Напряжение с фазы В
		struct  {
			int16_t v1;
			int16_t v2;			
		} voltage_b; 
		// Напряжение с фазы А 
		struct  {
			int16_t v1;
			int16_t v2;			
		} voltage_c; 	 
	 } ElectricalData_TypeDef;
	 

	/// Структура для мгновенных оцифрованных данных с АЦП
	/// \note: Ix1 - датчик тока на входе преобразователя
	/// Ix2 - датчик тока обмотки W11, Ix3 - датчик тока обмотки W12
	/// Ix4 - датчик тока обмотки W13, Ix5 - датчик тока линии
	/// Vx1 - датчик напряжения на входе преобразователя
	/// Vx2 - датчик напряжения на выходе преобразователя 
	/// x - номер фазы \b: none	 
	typedef struct 
	{
		 struct  {
			uint16_t i5[360];
			uint16_t i4[360];
			uint16_t i3[360];
			uint16_t i2[360];
			uint16_t i1[360];		
		} current_a;
		struct  {
			uint16_t i5[360];
			uint16_t i4[360];
			uint16_t i3[360];
			uint16_t i2[360];
			uint16_t i1[360];		
		} current_b; 
		struct  {
			uint16_t i5[360];
			uint16_t i4[360];
			uint16_t i3[360];
			uint16_t i2[360];
			uint16_t i1[360];		
		} current_c; 
		struct  {
			uint16_t v1[360];
			uint16_t v2[360];			
		} voltage_a; 
		struct  {
			uint16_t v1[360];
			uint16_t v2[360];			
		} voltage_b; 
		struct  {
			uint16_t v1[360];
			uint16_t v2[360];			
		} voltage_c; 	 
	} InstantData_TypeDef;
	 
	/// Ступени
	/// \note \b 
	typedef enum {
		Stage0		=	0x00, 
		Stage1		=	0x01, 
		Stage2		=	0x02, 
		Stage3		=	0x03, 
		Stage4		=	0x04, 
		Stage5		=	0x05, 
		Stage6		=	0x06, 
		StageNull	=	0x0A
	} swStage_t; 	
	
	/// Фазы
	/// \note \b 
	typedef enum {
		PhaseA				=	0x01,
		PhaseB				=	0x02,
		PhaseC				=	0x03,
		PhaseNull			=	0x00
	} swPhase_t;
	
	/// Состояние ключа
	/// \note \b 
	typedef enum {
		ON				=	GPIO_PIN_SET,
		OFF				=	GPIO_PIN_RESET		
	} swState_t;

	/// Структура для переключения фаз
	/// \note: none \b: none
	typedef struct 
	{
		 swPhase_t  Phase;  
		 swStage_t  Stage;  
		 swState_t  Status;
		 uint16_t 	SP_SC_delay;
	} StageCtrl_TypeDef; 
	 
	 
	 /// Структура для поиска нуля
	/// \note: none \b: none
	typedef struct
	{
		 ADC_TypeDef* ADCx;  
		 uint32_t  		DMAx_FLAG;  
		 uint16_t			ADCx_Poll;  
		 int16_t			AVG_Phase_x;
	 } Cross_TypeDef; 
	
	 
	/// Статус коды для ошибок инициализации FreeRTOS
	/// \note  \b 
	typedef enum  {
		osiOK                   =     0,       ///< Инициализая прошла успешно, ошибок не зарегистрирован
		osiErrorMask            =  0x0F,       ///< Маска для ErrorHandler
		osiErrorMessageQ   			=  0x10,       ///< Ошибка создания очереди сообщений
		osiErrorTimer          	=  0x20,       ///< Ошибка создания таймера
		osiErrorThread         	=  0x30,       ///< Ошибка создания задачи
		osiErrorSemaphore       =  0x40,       ///< Ошибка создания семаформа
		osiErrorMPool         	=  0x50,       ///< Ошибка создания пула памяти
		osiErrorFlag 						=  0x60,       ///< ошибка создания флага
		osiErrorMutex 					=  0x70,       ///< ошибка создания мьютекса
	} osInitStatus_t;
	
	/// Порты измерений  АЦП
	/// \note Расположены в порядке удобным для обработчика \b 
	typedef enum {
		I5A, I4A, I3A, I2A, I1A,		///< Токи фазы А	(0 - 4)
		I5B, I4B, I3B, I2B, I1B, 		///< Токи фазы B	(5 - 9)
		I5C, I4C, I3C, I2C, I1C, 		///< Токи фазы C	(10 - 14)
		V1A, V2A, 									///< Напряжения фазы А	(15 - 16)
		V1B, V2B, 									///< Напряжения фазы В	(17 - 18)
		V1C, V2C										///< Напряжения фазы С	(19 - 20)
	} adcMeasurePorts; 
	
	/// Статус коды для задач ОС
	/// \note \b 
	typedef enum {
    osNull                  = 0x00,   ///< Сброшенный сигнал
		adcDisable							= 0x10,		///< Отключить АЦП
		adcEnable						    = 0x11,		///< Включить АЦП		
		adcConvComplete					= 0x12,		///< Преобразования завершены
		adcConvError						= 0x13,		///< Ошибка преобразования
		adcRmsPrint							= 0x14,		///< Необходимо напечатать результат
		adcAvgCalc							= 0x15,		///< Среднее значение посчитанно
		adcRmsCalc							= 0x16,		///< Действующее значение посчитанно
		adcFlRmsCalc						= 0x17,		///< Действующее значение отфильтровано
		swChangePend						=	0x20,		///< Необходимо переключение ступеней
    swErrorTimeout					=	0x21,		///< Ошибка таймаут
    swErrorUnknown					=	0x22,		///< Неизвестная ошибка
    swCrossFound						=	0x23,		///< Ноль найден
    swCrossNFnd						  =	0x24,		///< Ноль не найден
		swSrhInitOk							=	0x24,		///< Успешная инициализация структуры для поиска нуля
		swPortOn								=	0x25,		///< GPIO Порт ключа включен
		swPortOff								=	0x26,		///< GPIO Порт ключа отключен
		swPortError							=	0x27,		///< Ошибка GPIO порта ключа
		uartOK									=	0x31,		///< ОК
		uartError								=	0x32,		///< Ошибка предачи по UART
		uartBusy								=	0x33,		///< Передатчик занят
		uartTimeout							= 0x34,		///< Таймаут передачи
		simSendError						=	0x40,		///< Ошибка пересылки команды
		simSendOK								=	0x41,		///< Передача успешна
		
	} osSignals_t;
	
	/// Статус коды ошибок для FaultHandler
	/// \note \b 
	typedef enum {   
		errOscInit							= 0x01,	///< Ошибка инициализации тактирования
		errADCInit							= 0x02,	///< Ошибка инициализации АЦП
		errDMAInit							= 0x03,	///< Ошибка инициализации ПДП
		errTIMInit							= 0x04,	///< Ошибка инициализации таймера
		errUART1Init							= 0x05,	///< Ошибка инициализации UART1
		errUART4Init							= 0x06,	///< Ошибка инициализации UART4
		errKernelInit							= 0x07,	///< Ошибка инициализации планировщика
		errRTOSInit							= 0x08,	///< Ошибка инициализации ОСРВ
		errKernelStart							= 0x0A,	///< Ошибка запуска планировщик
		errSIMInit							= 0x0B	///< Ошибка инициализации GSM
	} FaultSignals_t;
 
/* Exported constants --------------------------------------------------------*/
/*Логические константы*/
#define READY 			1
#define NOT_READY  	0
#define ENABLE_ADC 	1
#define DISABLE_ADC 0
#define TRUE				1
#define FALSE				0
#define POS					1
#define NEG					0
#define FLOAT_NULL	0x7FF
#define	UART4_OUTPUT	0x0E
#define	UART4_INPUT		0x1E

/*Коэффициенты для преобразования оцифровок из кодового значения в параметрические для массива*/
// Величины сопротивления в Ом для резисторов датчиков тока
#define Rdt1	130
#define Rdt2	24
#define Rdt3	130
#define Rdt4	24
#define Rdt5	6.8//3.9
// Коэффициент трансформации датчиков тока
#define Kdt		1000
// Коэффициент передачи датчика напряжени
#define Kdv		570 // коэф-т учитывает падение напряжения на оптопаре ДН
// Опорное напряжение
#define Vref  3.3
// Максимальное оцифрованное значение
#define Nmax  4096
// Коэффициенты преобразования из кодового значения в параметрические для датчиков тока
#define Koef_I1 (Vref*Kdt)/(Nmax*Rdt1)
#define Koef_I2 (Vref*Kdt)/(Nmax*Rdt2)
#define Koef_I3 (Vref*Kdt)/(Nmax*Rdt3)
#define Koef_I4 (Vref*Kdt)/(Nmax*Rdt4)
#define Koef_I5 (Vref*Kdt)/(Nmax*Rdt5)
// Коэффициенты преобразования для датчиков напряжения
#define Koef_V	(Vref*Kdv)/Nmax	

// Уровни напряжений на входе РСН для изменения ступени регулирования
#define  VOLTAGE_U00 (260*Nmax)/(Kdv*Vref) 	// 260В. Верхняя граница, значение взято из расчета никогда не реализуемого. 
#define  VOLTAGE_U01 (222*Nmax)/(Kdv*Vref) 	// 209В. =(209*2*4096)/(3.3*1000*1.17) уровень срабатывания перелючения с 0 в 1 ступень. Коэффициент 1.17 взят 
#define  VOLTAGE_U12 (215*Nmax)/(Kdv*Vref) 	// 202В. уровень срабатывания перелючения с 1 в 2 ступень	
#define  VOLTAGE_U23 (207*Nmax)/(Kdv*Vref) 	// 195В. уровень срабатывания перелючения с 2 в 3 ступень	
#define  VOLTAGE_U34 (196*Nmax)/(Kdv*Vref) 	// 184В. уровень срабатывания перелючения с 3 в 4 ступень	
#define  VOLTAGE_U45 (184*Nmax)/(Kdv*Vref) 	// 172В. уровень срабатывания перелючения с 4 в 5 ступень	
#define  VOLTAGE_U56 (172*Nmax)/(Kdv*Vref) 	// 161. уровень срабатывания перелючения с 5 в 6 ступень	
#define  VOLTAGE_U60 (161*Nmax)/(Kdv*Vref) 	// 155. уровень напряжения при котором регулятор уходит в байпасс		

// Ток линии 100А действующего значения
#define I_KZ						844	  
// Максимальное значение тока КЗ
#define I_KZ_max				3398	
#define IRQ_KZ_COUNT		10
// Задержка на пусковые токи = 100мс
#define SC_DELAY	100
// Задержка на переходные процессы при увеличении Uвх = 100мс
#define SP_DELAY_UIN_INC	100
// Задержка на переходные процессы при уменьшении Uвх = 1000мс
#define SP_DELAY_UIN_DEC	1000
// Максимальное количество ступеней 
#define MAX_STAGES 7
// Размеры очередей 
#define TX_QUEUE_SIZE    		1024
#define RX_QUEUE_SIZE   		128

#ifdef __cplusplus
}
#endif


#endif /* __DEFINE_H */

