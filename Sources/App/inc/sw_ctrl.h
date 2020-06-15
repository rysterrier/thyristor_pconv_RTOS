/**
  ******************************************************************************
  * @file           : sw_ctrl.h
  * @brief          : Заголовочный файл sw_ctrl.c
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SW_CTRL_H
#define __SW_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "defines.h"
/* Exported types ------------------------------------------------------------*/
/* Exported defines ----------------------------------------------------------*/
#define  PHASES_CNT 4
/* Exported variables --------------------------------------------------------*/
// Предыдущая ступень
extern swStage_t	PrevStage[PHASES_CNT];	
// Текущая ступень
extern swStage_t	CurStage[PHASES_CNT];		
/* Exported functions prototypes ---------------------------------------------*/
void vSwitchStage (void * arg);
void vAutomaticMode (void *arg);

#ifdef __cplusplus
}
#endif

#endif /* __SW_CTRL_H */
