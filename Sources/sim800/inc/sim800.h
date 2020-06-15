/**
  ******************************************************************************
  * @file           : sim800.h
  * @brief          : Заголовочный файл sim80.h
  *                   
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SIM_800_H
#define __SIM_800_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "defines.h"
/* Private includes ----------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Размеры буфферов */
#define CURRENT_CMD_SIZE 128     // размер буфера под передаваемую SIM800 команду
#define REC_BUF_SIZE 128         // размер буфера под принимаемые от SIM800 данные 
#define SMS_DATA_SIZE 128    		 // размер буфера под СМС сообщение
#define SERVER_DATA_SIZE 256 		 // размер буфера под данные на сервер
#define PHONE_NUM_SIZE 12        // размер буфера под телефонный номер 
#define CTRL_PHONES_SIZE 16      // размер буфера под массив контролирующих номеров

#define ANSWER_RETRIES_MAX	5	// Количество попыток запроса комманды

/* USSD запросы разных операторов */
#define Beeline_balance_request   "#102#"
#define MTS_balance_request       "#100#"
#define MegaPhone_balance_request "#100#"
#define Tele2_balance_request     "#105#"

/* Операторы сотовой связи (настройки для GPRS) */
#define OPER_beeline		"Beeline"
#define APN_beeline     "internet.beeline.ru"  
#define USER_beeline    "beeline"  
#define PWD_beeline     "beeline"

#define OPER_mts				"MTS"
#define APN_mts         "internet.mts.ru"      
#define USER_mts        "mts"  
#define PWD_mts         "mts"

#define OPER_megafon		"MegaPhone"
#define APN_megafon     "internet.megafon.ru"  
#define USER_megafon    "megafon"  
#define PWD_megafon     "megafon"

#define OPER_tele2			"Tele2"
#define APN_tele2       "internet.tele2.ru"    
#define USER_tele2      "tele2"  
#define PWD_tele2       "tele2"

/* Домен сервера, на которые будет осуществляться отправка данных */
#define URL            "http://www.anapteks.ru/Sim800L.php"

/// Статус коды для ошибок Sim800
/// \note  \b 
typedef enum  {
		sim800OK			=	0,       
		sim800EchoErr			=	0x01,       
		sim800RegNetworkErr			=	0x02,       
		sim800WeakSignalErr			=	0x03,       
		sim800SimcardErr			=	0x04,       
		sim800ActStatErr			=	0x05,       
		sim800TransmitOK			=	0x06,       
		sim800TransmitErr			=	0x07     
} Sim800StatusCode_t;
	
	
typedef enum  {
		RxNotCplt			= RESET,	
		RxCplt			=	SET,
		TxNotCplt			=	0x02,
		TxCplt			=	0x03,
		ATCmdValidExp			= 0x0A,  
		ATCmdNValid			= 0x0B,       
		ATCmdValid			= 0x0C,       
		ATCmdNSetState			= 0x0D
} CommsStatus_t;

/* Exported types ------------------------------------------------------------*/
typedef struct
{
	uint8_t RegisterdToNetwork;
	char		Operator[9];
	uint8_t Busy:1;	
	uint8_t Signal; 
	
}sim800Status_t;

typedef struct
{	
	char	*Command;
	uint32_t	SendCommandStartTime;
	uint16_t	RxBufferSize;  	
	CommsStatus_t	CmdValid;   
	CommsStatus_t	ReceiveCplt;   
	char	Usart[REC_BUF_SIZE]; 
	char	ReceiveAnswer[REC_BUF_SIZE]; 
	
}sim800ATcmd_t;

typedef struct
{  
	uint8_t	SmsReady:1; 
	uint8_t	MsgRead:1;  
	uint8_t	MsgSent:1;  
	uint8_t	MsgValid:1;
	char	MsgReceive[SMS_DATA_SIZE]; 
	char	MsgSend[SMS_DATA_SIZE]; 
	char	SentNumber[PHONE_NUM_SIZE];
}sim800GSM_t;

typedef enum
{
  GPRSSendData_Idle=0,
  GPRSSendData_SendOK,
  GPRSSendData_SendFail,
  
}GPRSSendData_t;

typedef struct 
{
  uint16_t	ResultCode;
  char	Data[SERVER_DATA_SIZE];
  
}GPRSHttpAction_t;

typedef struct
{
  char	APN[17];
  char	APN_UserName[17];
  char	APN_Password[17];
  char	LocalIP[17]; 
  GPRSSendData_t	SendStatus;  
  GPRSHttpAction_t	HttpAction;  
  
}sim800GPRS_t;

typedef struct
{	
	sim800Status_t	Status;
	sim800GSM_t	GSM;
	sim800GPRS_t	GPRS;	
	
}sim800_t;

/* Exported variables --------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
void vSim800InitThread(void *arg);
void rst_sim800l (void * arg);
/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif 
