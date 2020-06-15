/**
  ******************************************************************************
  * @file           : sim800.c
  * @brief          : Данный файл содержит функции по работе с SIM800L
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "sim800.h"
#include "periph.h"
#include "uart4.h"
#include "rtos.h"
#include "main.h"
#include "adc.h"
#include "sw_ctrl.h"
/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// Символ ctrl-z
#define CTRL_Z 26
// Максимальное время ожидание ответа от SIM800 в мс
#define MAX_WAITINGTIME_MS 200
// Начальное количество управляющих телефонов ( см. ctrl_numbers)
#define CTRL_PHONES_INITIAL 1
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
// Структура для SIM модуля
sim800_t 			Sim800 = {
	.Status.RegisterdToNetwork 	= RESET,
	.Status.Operator	=	NULL,
	.Status.Busy			=	RESET,
	.Status.Signal 		= NULL,
	
	.GSM.SmsReady = RESET,
	.GSM.MsgRead	= RESET,
	.GSM.MsgSent	= RESET,
	.GSM.MsgValid	=	RESET,
	
	.GPRS.SendStatus	=	GPRSSendData_Idle
};	
// Структура для АТ команд
static sim800ATcmd_t Sim800AT;
// Массив с номерами, коотрые могут осуществлять контроль
char* ctrl_numbers [CTRL_PHONES_SIZE] = {"+79032081264",0};
// Количество текущих управляющих номеров
uint8_t	ctrl_numbers_cnt = CTRL_PHONES_INITIAL;
/* Private function prototypes -----------------------------------------------*/
static CommsStatus_t Sim800_RxCplt(void);
static CommsStatus_t Sim800_SendStr(char* str, uint16_t size);
static CommsStatus_t Sim800_SendATcmd(char* ATCmd, uint16_t max_answers, uint16_t delay, uint16_t bufsize,...);
static CommsStatus_t Sim800_SendRaw(char *str, uint8_t delay);
static uint8_t sms_message_compile(char* str);
static void create_server_data(char *time);
static Sim800StatusCode_t vSim800ATInit (void);
/* Private user code ---------------------------------------------------------*/
/**
	* @brief  Проверка получение ответа от модуля
  * @retval RxCplt - сообщение полученно
	* 				RxNotCplt - сообщение не полученно
  */
static CommsStatus_t Sim800_RxCplt(void)
{
	
	for (uint16_t pos = 0; pos < sizeof(Sim800AT.ReceiveAnswer); pos++){
		if (Sim800AT.ReceiveAnswer[pos] != NULL){
			while (Sim800AT.ReceiveAnswer[0] == NULL){
				for (uint16_t pos1 = 0; pos1 < sizeof(Sim800AT.ReceiveAnswer); pos1++)
					Sim800AT.ReceiveAnswer[pos1] = Sim800AT.ReceiveAnswer[pos1+1];
			}
			break;
		}
	}	
	return Sim800AT.ReceiveAnswer[strlen(Sim800AT.ReceiveAnswer) - 1] == '\n' ? RxCplt : RxNotCplt;
}

/**
	* @brief  Отправка строки на модуль
	* @param	str - строка для отправки
	*	@param	size - размер ответа
	* @retval TxCplt - успешная отправка
	* 				TxNotCplt - ошибка при отправке
  */
static CommsStatus_t Sim800_SendStr(char *str, uint16_t size)
{
	osStatus_t status;
	
	if ( str != NULL && size != NULL)
	{
		status = osMutexAcquire(uart1_m,osWaitForever);
		if (status == osOK)
		{	
			/* Очистка буффера приема */
			memset(Sim800AT.ReceiveAnswer, 0, sizeof(Sim800AT.ReceiveAnswer));	
			HAL_UART_Receive_DMA(&huart1,(uint8_t*)Sim800AT.ReceiveAnswer,size);
			HAL_UART_Transmit_DMA(&huart1,(uint8_t*)str,strlen(str));    
		}
		return TxCplt;		
	}
	return TxNotCplt;
}

/**
* @brief  Отправка АТ комманды
	* @param	ATCmd - AT команда
	* @param	max_answers - количество ответов от модуля
	* @param	delay - время задержки
	* @param	bufsize - размер буффера для ответа
	* @param	... - ожидаемый ответ
  * @retval RxCplt - сообщение полученно
	* 				RxNotCplt - сообщение не полученно
	*					ATCmdValid - ответ соответсвует ожидаемому
	* 				ATCmdNValid - ответ не соответсвует ожидаемому
  */
static CommsStatus_t Sim800_SendATcmd(char *ATCmd, uint16_t max_answers, uint16_t delay, uint16_t bufsize,...)
{
	va_list tag;
	/* Проверка на занятость модуля */
	while (Sim800.Status.Busy)
		osDelay(100);
	
	/* Сброс статуса модуля */
	Sim800.Status.Busy = SET;	
	/* Копирование АТ комманды для отправки */		
	Sim800AT.Command = ATCmd;
	/* Статус приема комманды */
	Sim800AT.ReceiveCplt	= RxNotCplt;	
	/* Время начала приема */
	Sim800AT.SendCommandStartTime = osKernelGetTickCount();
	/* Выбор количество ячеек в массиве приема */
	Sim800AT.RxBufferSize = bufsize;
	/* Если есть ожидаемый ответ - фиксируем его */
	Sim800AT.CmdValid = ATCmdNSetState;
			
	va_start(tag,bufsize);
	if ((uint8_t)strlen(va_arg(tag,char *))){
		Sim800AT.CmdValid = ATCmdValidExp;	
	}
	va_end(tag);
	
		
	while ( max_answers-- != 0){
		/* Отправка комманды */
		if (Sim800_SendStr(Sim800AT.Command, Sim800AT.RxBufferSize) == sim800TransmitOK){			
			/* Ождиание ответа */
			while ( osKernelGetTickCount() - Sim800AT.SendCommandStartTime <= delay){
				osDelay(10);			
				/* Проверка наличия ответа */
				if (Sim800_RxCplt()){	
					
					/* Если есть вариант ответа - проверить его */
					if (Sim800AT.CmdValid == ATCmdValidExp){
						va_start(tag,bufsize);
						if (strstr(Sim800AT.ReceiveAnswer,va_arg(tag,char *))){
							Sim800AT.CmdValid = ATCmdValid;
						}else{
								Sim800AT.CmdValid = ATCmdNValid;
						}		
						va_end(tag);
					}																
					/* Вывод результата */
					Sim800.Status.Busy = RESET;		
					Sim800AT.ReceiveCplt = RxCplt;		
					osMutexRelease(uart1_m);
					return (Sim800AT.CmdValid == ATCmdNSetState ? Sim800AT.ReceiveCplt : Sim800AT.CmdValid);  					
				}								
			}
		}
	}
	
	/* Передача не прошла */
	Sim800.Status.Busy = RESET;		
	osMutexRelease(uart1_m);
	return Sim800AT.ReceiveCplt = RxNotCplt;
}

/**
	* @brief  Отправка данных на модуль без ожидания ответа
	* @param	str - данные на отправку
	* @param	delay - задержка 
	* @retval TxCplt - успешная отправка
	* 				TxNotCplt - ошибка при отправке
  */
static CommsStatus_t Sim800_SendRaw(char *str, uint8_t delay)
{	
	osStatus_t status;
	
	if ( str != NULL )
	{
		/* Проверка на занятость модуля */
		while (Sim800.Status.Busy)
			osDelay(100);
		
		/* Сброс статуса модуля */
		Sim800.Status.Busy = SET;	
		/* Время начала приема */
		Sim800AT.SendCommandStartTime = osKernelGetTickCount();
		
		status = osMutexAcquire(uart1_m,osWaitForever);
		if (status == osOK)
		{
			/* Отправка данных */
			HAL_UART_Transmit_DMA(&huart1,(uint8_t*)str,strlen(str)); 
			status = osMutexRelease(uart1_m);			
		}
		/* Ождиание по delay */
		while ( osKernelGetTickCount() - Sim800AT.SendCommandStartTime <= delay)
			osDelay(10);						

		Sim800.Status.Busy = RESET;	
		return TxCplt;
	}
	
	return TxNotCplt;
}

/**
	* @brief  Подготовка СМС сообщения
	* @param	str - данные на отправку
  * @retval SET - сообщение подготовленно
*					RESET - обибка подготовки
  */
static uint8_t sms_message_compile(char *str)
{	
	if (str != NULL){
		/* Освобождение памяти массива */
		memset(Sim800.GSM.MsgSend, 0, sizeof(Sim800.GSM.MsgSend));
		/* Копирование ответа */
		memcpy(Sim800.GSM.MsgSend, str,sizeof(Sim800.GSM.MsgSend));
		/* Добавление CTRL_Z */
		sprintf(Sim800.GSM.MsgSend, "%s%c",Sim800.GSM.MsgSend,CTRL_Z);					
		/* Cообщение готово */
		return SET;	
	}
	else
		return RESET;
}

/**
	* @brief  Формирование данных на сервер
	* @param time - указатель на переменную
	*	содержащую время с сервера
  * @retval none
  */
static void create_server_data(char *time)
{
	static	osStatus_t	val_sem;	
	
	char temp_str1[SERVER_DATA_SIZE];
	memset(temp_str1,0,sizeof(temp_str1));
	
	/* Подготовка запроса на сервер */
	memset(Sim800.GPRS.HttpAction.Data,0,sizeof(Sim800.GPRS.HttpAction.Data));
	sprintf(Sim800.GPRS.HttpAction.Data, "AT+HTTPPARA=\"URL\",\"%s/index.php/?",URL);		
		
	val_sem = osSemaphoreAcquire(adc_reader_s ,osWaitForever);
	if (val_sem == osOK){
		val_sem = osSemaphoreAcquire(adc_writer_s ,osWaitForever);
		if (val_sem == osOK){
			
			osSemaphoreRelease(adc_reader_s);
			
			/* Фаза А */
			strcat(Sim800.GPRS.HttpAction.Data," PhaseA:\n");						
			for (uint8_t arg = I5A; arg <= I1A; arg++){
				memset(temp_str1,0,strlen(temp_str1));
				sprintf(temp_str1,"i%d=%d,",abs(arg-5),sSelectArgumentElec(&fl_rms_data,arg)*current_conv_coef[arg]);				
				strcat(Sim800.GPRS.HttpAction.Data,temp_str1);			
			}
			for (uint8_t arg = V1A; arg <= V2B; arg++){
				memset(temp_str1,0,strlen(temp_str1));
				sprintf(temp_str1,"v%d=%f:5:3,",abs(arg-17),(double)sSelectArgumentElec(&fl_rms_data,arg)*Koef_V);				
				strcat(Sim800.GPRS.HttpAction.Data,temp_str1);			
			}
			
			/* Фаза В */
			strcat(Sim800.GPRS.HttpAction.Data," PhaseB:\n");						
			for (uint8_t arg = I5B; arg <= I1B; arg++)
			{
				memset(temp_str1,0,strlen(temp_str1));
				sprintf(temp_str1,"i%d=%d,",abs(arg-10),sSelectArgumentElec(&fl_rms_data,arg)*current_conv_coef[arg-5]);	
				strcat(Sim800.GPRS.HttpAction.Data,temp_str1);			
			}
			for (uint8_t arg = V1B; arg <= V2B; arg++){
				memset(temp_str1,0,strlen(temp_str1));
				sprintf(temp_str1,"v%d=%f:5:3,",abs(arg-19),(double)sSelectArgumentElec(&fl_rms_data,arg)*Koef_V);			
				strcat(Sim800.GPRS.HttpAction.Data,temp_str1);			
			}
			
			/* Фаза С */
			strcat(Sim800.GPRS.HttpAction.Data," PhaseC:\n");						
			for (uint8_t arg = I5C; arg <= I1C; arg++)				
			{
				memset(temp_str1,0,strlen(temp_str1));
				sprintf(temp_str1,"i%d=%d,",abs(arg-15),sSelectArgumentElec(&fl_rms_data,arg)*current_conv_coef[arg-10]);				
				strcat(Sim800.GPRS.HttpAction.Data,temp_str1);			
			}
			for (uint8_t arg = V1C; arg <= V2C; arg++){
				memset(temp_str1,0,strlen(temp_str1));
				sprintf(temp_str1,"v%d=%f:5:3,",abs(arg-21),(double)sSelectArgumentElec(&fl_rms_data,arg)*Koef_V);		
				strcat(Sim800.GPRS.HttpAction.Data,temp_str1);			
			}
			
			val_sem = osSemaphoreAcquire(adc_reader_s ,osWaitForever);
			if (val_sem == osOK)			
				osSemaphoreRelease(adc_writer_s);											
		}
		osSemaphoreRelease(adc_reader_s);		
	}
	strcat(Sim800.GPRS.HttpAction.Data,"t=");
	memset(temp_str1,0,strlen(temp_str1));
	sprintf(temp_str1, "%s",time);										
	strcat(Sim800.GPRS.HttpAction.Data,"\"\r");
}

/**
	* @brief  Задача обслуживания отправки данных на сервер
	* @param *arg - начальные аргументы
  * @retval none
  */
void vSim800ServerThread(void *arg)
{
	osStatus_t status;
	char	time[16];
	for(;;)
	{
		/* Подготовка запроса на сервер */		
		memset(Sim800.GPRS.HttpAction.Data,0,sizeof(Sim800.GPRS.HttpAction.Data));
		sprintf(Sim800.GPRS.HttpAction.Data, "AT+HTTPPARA=\"URL\",\"%s/index.php/?id=0\"\r",URL);		
		Sim800.GPRS.SendStatus = GPRSSendData_Idle;
		
		/* Отправка команды на сервер для чтения времени*/
		status = osMutexAcquire(sim800_ctrl_m,osWaitForever);
		if (status == osOK){
			/* Запрос сервера */
			Sim800_SendATcmd(Sim800.GPRS.HttpAction.Data,ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
			
			/* Разрешение отправки сообщений на сервер */
			Sim800_SendATcmd("AT+HTTPACTION=0\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
			if (strstr(Sim800AT.ReceiveAnswer,": 0,200,")){						
				/* Чтение времени с сервера */
				Sim800_SendATcmd("AT+HTTPREAD\r",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
				if (strstr(Sim800AT.ReceiveAnswer,"Time")){
					strncpy(time,strstr(Sim800AT.ReceiveAnswer, "Time")+4,sizeof(time));
					status = osMutexRelease(sim800_ctrl_m);					
					/* Подготовка данных к отправке */
					create_server_data(time);
					
					status = osMutexAcquire(sim800_ctrl_m,osWaitForever);
					if (status == osOK){
						Sim800_SendRaw(Sim800.GPRS.HttpAction.Data, 0);
						Sim800_SendATcmd("AT+HTTPACTION=0\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
						if (!strstr(Sim800AT.ReceiveAnswer,": 0,200,")){	
							Sim800.GPRS.SendStatus = GPRSSendData_SendFail;
						}else{
							Sim800.GPRS.SendStatus = GPRSSendData_SendOK;
						}
						status = osMutexRelease(sim800_ctrl_m);			
					}
				}else{
					Sim800.GPRS.SendStatus = GPRSSendData_SendFail;
					status = osMutexRelease(sim800_ctrl_m);			
				}
			}else{
				Sim800.GPRS.SendStatus = GPRSSendData_SendFail;
				status = osMutexRelease(sim800_ctrl_m);			
			}
			
		}			
		/* задержка - минута */
		osDelay(60000);
		}							
}

/**
	* @brief Задача коммуникации с помощью  SIM800L
	* @param *arg - начальные аргументы
  * @retval none
  */
void vSim800CommsThread(void *arg)
{
	osStatus_t status;
	uint8_t num;
										
	for(;;)
	{			
		/* Проверка СМС -------------------------------------------*/		
		status = osMutexAcquire(sim800_ctrl_m,osWaitForever);
		if (status == osOK){
			Sim800_SendATcmd("AT+CMGR=1\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);	
			
			/* Пришло СМС */
			if (strstr(Sim800AT.ReceiveAnswer, "REC")){
				 /* !!!! ПОМЕНЯЙ МЕСТАМИ ПРОВЕРКУ КОДОВОГО СЛОВА И НОМЕРА ДЛЯ ОПТИМИЗАЦИИ RUNTIME*/
				/* Простановка флагов */
				Sim800.GSM.MsgRead = SET;
				Sim800.GSM.MsgSent = RESET;
				
				/* Снятие номера с которого пришло сообщение и проверка его на номера для управления */
				for (num = 0; num < CTRL_PHONES_SIZE; num++){
					if (strstr(Sim800AT.ReceiveAnswer, ctrl_numbers[num])){
						/* Сообщение пришло с контрольного номера */
						Sim800.GSM.MsgValid = SET;
						/* Копирование СМС для обработки */
						memcpy(Sim800.GSM.SentNumber, ctrl_numbers[num], sizeof(ctrl_numbers));
						memset(Sim800.GSM.MsgReceive, 0 , sizeof(Sim800.GSM.MsgReceive));
						memcpy(Sim800.GSM.MsgReceive, Sim800AT.ReceiveAnswer,strlen(Sim800AT.ReceiveAnswer));
						break;
					}
					else{
						/* Сообщение пришло не с контрольного номера  и не рассматривается*/
						Sim800.GSM.MsgValid = RESET;
					}
				}
				
				/* Удаляем СМС */
				Sim800_SendATcmd("AT+CMGDA=\"DEL ALL\"\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);					
				Sim800.GSM.MsgRead = RESET;
			}			
			status = osMutexRelease(sim800_ctrl_m);
		}
		/* --------------------------------------------------------*/
		
		/* Обработка СМС ------------------------------------------*/		
		if (Sim800.GSM.MsgValid){
			
			Sim800.GSM.MsgValid = RESET;
			
			/* Проверка на наличие кодового слова */
			if (strstr(Sim800.GSM.MsgReceive,"TRN:")){
				/* Подчистка сообщения*/
				strcpy(Sim800.GSM.MsgReceive,strstr(Sim800.GSM.MsgReceive,"TRN:"));			
				for (uint16_t i = (strlen(Sim800.GSM.MsgReceive) - (strlen(strstr(Sim800.GSM.MsgReceive,"\r")))); i < sizeof(Sim800.GSM.MsgReceive) ; i++)
					Sim800.GSM.MsgReceive[i] = NULL;		
			}
																	
			/* Проверка Текста СМС */
			/* Помощь */
			if (!strcmp(Sim800.GSM.MsgReceive,"TRN:help") || !strcmp(Sim800.GSM.MsgReceive,"TRN:Help")){
				Sim800.GSM.SmsReady = sms_message_compile("stage_chg=x - change stage to x\nreg_num:+79xxxx - register num\nstat - get stat");		
			}						
			else if (!strcmp(Sim800.GSM.MsgReceive,"TRN:stat") || !strcmp(Sim800.GSM.MsgReceive,"TRN:Stat")){
				
			}
			else if (strstr(Sim800.GSM.MsgReceive,"TRN:reg_num:")){
				if(strlen(strstr(Sim800.GSM.MsgReceive,"+")) < 12 
					|| strlen(strstr(Sim800.GSM.MsgReceive,"+")) > 12
					|| !strstr(Sim800.GSM.MsgReceive,"+7")){
						Sim800.GSM.SmsReady = sms_message_compile("Wrong Format");		
				}
				else{
					if (ctrl_numbers_cnt > CTRL_PHONES_SIZE){
						Sim800.GSM.SmsReady = sms_message_compile("Max Number of phones reached");		
					}
					else{
						memcpy(ctrl_numbers[++ctrl_numbers_cnt], strstr(Sim800.GSM.MsgReceive,"+"),PHONE_NUM_SIZE);						
						Sim800.GSM.SmsReady = sms_message_compile("Added number to the phonebook");		
					}
				}
			}
		}
		/* --------------------------------------------------------*/
		
		/* Ответ на СМС -------------------------------------------*/
		if (Sim800.GSM.SmsReady){
						
			/* Подготовка данных номера*/				
			char num_str[CURRENT_CMD_SIZE];			
			memset(num_str, 0 , sizeof(num_str));
			sprintf(num_str, "AT+CMGS=\"%s\"\r",Sim800.GSM.SentNumber);							
				
			status = osMutexAcquire(sim800_ctrl_m,osWaitForever);
			if (status == osOK){			
				
				/* Перевод в режим СМС */
				Sim800_SendATcmd("AT+CMGF=1\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);				
				/* Отправка номера, на которое должно придти сообщение */				
				Sim800_SendRaw(num_str,0);
				/* Отправка сообщения */							
				Sim800_SendRaw(Sim800.GSM.MsgSend,0);
				
				/* Простановка флагов */ 
				Sim800.GSM.SmsReady = RESET;		
				Sim800.GSM.MsgSent	=	SET;  
				
				status = osMutexRelease(sim800_ctrl_m);
			}
		}
		/* --------------------------------------------------------*/
		osDelay(500);		
	}
}

/**
	* @brief  Инициализация SIM800L
	* @param *arg - начальные аргументы
  * @retval none
  */
void vSim800InitThread(void *arg)
{	
	osThreadAttr_t 	thread_attr;
	char ATcmd[CURRENT_CMD_SIZE];	
	
	/* Сброс SIM800L */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_RESET);
	
	/* Инициализация модуля АТ коммандами */
	/* Проверка модуля */
	if (Sim800_SendATcmd("AT\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE,"OK") != ATCmdValid)
		Error_Handler(errSIMInit);	
	/* Сброс настроек */
	if  (Sim800_SendATcmd("AT&F\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE,"OK") != ATCmdValid)
		Error_Handler(errSIMInit);	
	/* Отключение эхо */
	if  (Sim800_SendATcmd("ATE0\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE, "OK") != ATCmdValid)
		Error_Handler(errSIMInit);
	
	/* Проверка регистрации в сети */
	Sim800_SendATcmd("AT+CREG?\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE,"");
	if(strstr(Sim800AT.ReceiveAnswer,"+CREG: 0,1"))
		Sim800.Status.RegisterdToNetwork = 1;
	else
		Error_Handler(errSIMInit);
		
	/* Проверка связи */
	Sim800_SendATcmd("AT+CSQ\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
	Sim800.Status.Signal = Sim800AT.ReceiveAnswer[8]*10 + Sim800AT.ReceiveAnswer[9];
	if (Sim800.Status.Signal < 7)
		printf("Warning: Weak Signal");
	
	/* Проверка SIM карты */
	if( Sim800_SendATcmd("AT+CCID\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE, "OK") != ATCmdValid)	
		Error_Handler(errSIMInit);
	
	/* Запрос оператора сотовой связи */
	Sim800_SendATcmd("AT+COPS?\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE,"");
	if(strstr(Sim800AT.ReceiveAnswer,"ERROR"))	
		Error_Handler(errSIMInit);
	else
	{
		if (strstr(Sim800AT.ReceiveAnswer,OPER_beeline))
		{
			strncpy(Sim800.Status.Operator, OPER_beeline, strlen(OPER_beeline));
			strncpy(Sim800.GPRS.APN, APN_beeline, strlen(APN_beeline));
			strncpy(Sim800.GPRS.APN_UserName, USER_beeline, strlen(USER_beeline));
			strncpy(Sim800.GPRS.APN_Password, PWD_beeline, strlen(PWD_beeline));
		}
		else if (strstr(Sim800AT.ReceiveAnswer,OPER_mts))
		{
			strncpy(Sim800.Status.Operator, OPER_mts, strlen(OPER_mts));
			strncpy(Sim800.GPRS.APN, APN_mts, strlen(APN_mts));
			strncpy(Sim800.GPRS.APN_UserName, USER_mts, strlen(PWD_mts));
			strncpy(Sim800.GPRS.APN_Password, PWD_mts, strlen(PWD_mts));
		}
		else if (strstr(Sim800AT.ReceiveAnswer,OPER_megafon))
		{
			strncpy(Sim800.Status.Operator, OPER_beeline, strlen(OPER_beeline));
			strncpy(Sim800.GPRS.APN, APN_megafon, strlen(APN_megafon));
			strncpy(Sim800.GPRS.APN_UserName, USER_megafon, strlen(USER_megafon));
			strncpy(Sim800.GPRS.APN_Password, PWD_megafon, strlen(PWD_megafon));
		}
		else if (strstr(Sim800AT.ReceiveAnswer,OPER_tele2))
		{
			strncpy(Sim800.Status.Operator, OPER_tele2, strlen(OPER_tele2));
			strncpy(Sim800.GPRS.APN, APN_tele2, strlen(APN_tele2));
			strncpy(Sim800.GPRS.APN_UserName, USER_tele2, strlen(USER_tele2));
			strncpy(Sim800.GPRS.APN_Password, PWD_tele2, strlen(PWD_tele2));
		}
	}
	/* Перевод в режим ASCII */
	Sim800_SendATcmd("AT+CMGF=1\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);		
	/* Удаление всех СМС */
	Sim800_SendATcmd("AT+CMGDA=\"DEL ALL\"\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
		
	
	/* Проверка GPRS соединения */
	Sim800_SendATcmd("AT+SAPBR=2,1\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
	/* GPRS соединение открыто */
	if(strstr(Sim800AT.ReceiveAnswer,"1,1"))	
		Sim800_SendATcmd("AT+SAPBR=0,1\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
	/* GPRS соединение не установлено */
	else if(strstr(Sim800AT.ReceiveAnswer,"1,3"))	
		Sim800_SendATcmd("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
	
	/* Установка точки доступа */
	Sim800_SendATcmd("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
			
	/* Ввод APN сервера */
	memset(ATcmd, 0 , sizeof(ATcmd));
	sprintf(ATcmd,"AT+SAPBR=3,1,\"APN\",\"%s\"\r\n",Sim800.GPRS.APN);
	Sim800_SendATcmd(ATcmd,ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
	
	/* Ввод логина */	
	memset(ATcmd, 0 , sizeof(ATcmd));
	sprintf(ATcmd,"AT+SAPBR=3,1,\"USER\",\"%s\"\r\n",Sim800.GPRS.APN_UserName);
	Sim800_SendATcmd(ATcmd,ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
	
	/* Ввод пароля */
	memset(ATcmd, 0 , sizeof(ATcmd));
	sprintf(ATcmd,"AT+SAPBR=3,1,\"PWD\",\"%s\"\r\n",Sim800.GPRS.APN_Password);
	Sim800_SendATcmd(ATcmd,ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
	
	/* Открытие соединения */
	Sim800_SendATcmd("AT+SAPBR=1,1\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
	
	/* Ининциализация HTTP сервиса */
	Sim800_SendATcmd("AT+HTTPINIT\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
	/* Ининциализация CID для HTTP сессии */
	Sim800_SendATcmd("AT+HTTPPARA=\"CID\",1\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);	
	
	/* Сохранение настроек */
	Sim800_SendATcmd("AT&W\r\n",ANSWER_RETRIES_MAX, MAX_WAITINGTIME_MS, REC_BUF_SIZE);
	
	/* Создание задачи обработчика СМС */	
	memset(&thread_attr, 0, sizeof(thread_attr));	
	thread_attr.name = "SIMComms_Thread";
	thread_attr.priority	=	osPriorityAboveNormal;
	thread_attr.stack_size = 512;
		
	Sim800lCommsHandle = osThreadNew(vSim800CommsThread, NULL, &thread_attr);	
	if (Sim800lCommsHandle == NULL)
		Error_Handler(errRTOSInit|osiErrorThread);
	
	
	/* Создание задачи отправки данных на сервер */
	memset(&thread_attr, 0, sizeof(thread_attr));	
	thread_attr.name = "SIM800_ServerThread";
	thread_attr.priority	=	osPriorityAboveNormal;
	thread_attr.stack_size = 512;

	Sim800lServerHandle = osThreadNew(vSim800ServerThread, NULL, &thread_attr);	
	if (Sim800lServerHandle == NULL)
		Error_Handler(errRTOSInit|osiErrorThread);
	
	osThreadExit();
}

/**
	* @brief  Перезапуск задачи SIM800L
	* @param *arg - начальные аргументы
  * @retval none
  */
void rst_sim800l (void * arg){
	
	osThreadAttr_t 	thread_attr ={
		.cb_mem = NULL,
		.cb_size = NULL,
		.stack_mem = NULL	
	};
	
	thread_attr.name = "SIM800_Init";
	thread_attr.priority	=	osPriorityNormal;
	thread_attr.stack_size = 512;

  Sim800lInitHandle = osThreadNew(vSim800InitThread, NULL, &thread_attr);	
	if (Sim800lInitHandle == NULL) 
		Error_Handler(osiErrorThread|errRTOSInit);
	
	osThreadExit();
	
}
