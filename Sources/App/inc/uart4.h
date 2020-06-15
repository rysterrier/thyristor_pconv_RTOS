/**
  ******************************************************************************
  * @file           : uart4.h
  * @brief          : Заголовочный файл uart4.c 
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART4_H
#define __UART4_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "defines.h"
/* Private includes ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
void vCommanRead(void *arg);
void vTransmitUART4(void *arg);
void HAL_UART4_Custom_IRQHandler(UART_HandleTypeDef *huart);
void flush_str(char *msg, uint16_t len);
/* Private defines -----------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __UART4_H */

