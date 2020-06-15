/**
  ******************************************************************************
  * @file           : periph.h
  * @brief          : Заголовочный файл periph.c
  *                   Инициализация всей необходимой периферии.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INIT_H
#define __INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "defines.h"
/* Private includes ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim6;

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart1;

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
void vPeripheralInit(void);
/* Private defines -----------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* __INIT_H */

