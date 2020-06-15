/**
  ******************************************************************************
  * @file           : sw_ll_ctrl.h
  * @brief          : Заголовочный файл sw_ll_ctrl.c 
  ******************************************************************************
	*
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SW_LL_CTRL_H
#define __SW_LL_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "defines.h"
/* Private includes ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
osSignals_t xCrossSearch(StageCtrl_TypeDef* PhaseSwitch);
osSignals_t vSync(swState_t stat);
osSignals_t vVS7Switch(StageCtrl_TypeDef* PhaseSwitch);
osSignals_t xPhaseControl(StageCtrl_TypeDef* PhaseSwitch);
void vDisableIRQn(void);
void vEnableIRQn(void);
/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __SW_LL_CTRL_H */
