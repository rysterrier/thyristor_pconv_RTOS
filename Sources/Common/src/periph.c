/**
  ******************************************************************************
  * @file           : periph.c
  * @brief          : Данный файл содержит функции
	*										инициализация всей необходимой периферии
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "periph.h"
#include "adc.h"
/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;

DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

static uint32_t HAL_RCC_ADC12_CLK_ENABLED=0;
static uint32_t HAL_RCC_ADC34_CLK_ENABLED=0;
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_USART1_Init(void); 
static void MX_DMA_Init(void);
static void MX_ADC_Init(void);
static void MX_ADC_MSP_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_ADC3_Init(void);
static void MX_ADC4_Init(void);
static void MX_TIM6_Init(void);
static void MX_UART4_Init(void);
static void MX_USART1_UART_Init(void);
/* Private user code ---------------------------------------------------------*/
/**
* @brief  Общая функция инициализации
  * @retval none
  */
void vPeripheralInit(void)
{

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
//	MX_ADC_Init();
//  MX_TIM6_Init();
  MX_UART4_Init();
  MX_USART1_UART_Init();
	MX_DMA_USART1_Init();
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler(errOscInit);
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler(errOscInit);
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_UART4
                              |RCC_PERIPHCLK_TIM1|RCC_PERIPHCLK_ADC12
                              |RCC_PERIPHCLK_ADC34;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV16;
  PeriphClkInit.Adc34ClockSelection = RCC_ADC34PLLCLK_DIV16;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLK_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler(errOscInit);
  }
}


/**
* @brief ADC 1-4 Initialization
* @param None
* @retval None
*/
static void MX_ADC_Init(void)
{
	MX_ADC_MSP_Init();
	MX_DMA_Init();	
	MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_ADC4_Init();	
	/* ADC AnalogWatchdog IRQn */
  HAL_NVIC_SetPriority(ADC1_2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
}

/**
* @brief ADC MSP Initialization
* This function configures the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
static void MX_ADC_MSP_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

		/* Peripheral clock enable */
    HAL_RCC_ADC12_CLK_ENABLED++;
		
    if(HAL_RCC_ADC12_CLK_ENABLED==1){
      __HAL_RCC_ADC12_CLK_ENABLE();
    }
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
		
    /**ADC1 GPIO Configuration    
    PA1     ------> ADC1_IN2
    PA2     ------> ADC1_IN3
    PA3     ------> ADC1_IN4
    PF4     ------> ADC1_IN5 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);


    /* Peripheral clock enable */
    HAL_RCC_ADC12_CLK_ENABLED++;
    if(HAL_RCC_ADC12_CLK_ENABLED==1){
      __HAL_RCC_ADC12_CLK_ENABLE();
    }
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC2 GPIO Configuration    
    PA4     ------> ADC2_IN1
    PA5     ------> ADC2_IN2
    PA6     ------> ADC2_IN3
    PA7     ------> ADC2_IN4
    PC4     ------> ADC2_IN5
    PC5     ------> ADC2_IN11
    PB2     ------> ADC2_IN12 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    HAL_RCC_ADC34_CLK_ENABLED++;
    if(HAL_RCC_ADC34_CLK_ENABLED==1){
      __HAL_RCC_ADC34_CLK_ENABLE();
    }
  
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**ADC3 GPIO Configuration    
    PB0     ------> ADC3_IN12
    PB1     ------> ADC3_IN1
    PE7     ------> ADC3_IN13
    PE9     ------> ADC3_IN2
    PE10     ------> ADC3_IN14
    PE11     ------> ADC3_IN15
    PE12     ------> ADC3_IN16
    PE13     ------> ADC3_IN3 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);


    /* Peripheral clock enable */
    HAL_RCC_ADC34_CLK_ENABLED++;
    if(HAL_RCC_ADC34_CLK_ENABLED==1){
      __HAL_RCC_ADC34_CLK_ENABLE();
    }
  
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**ADC4 GPIO Configuration    
    PE8     ------> ADC4_IN6
    PE14     ------> ADC4_IN1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_USART1_Init(void) 
{
		  /* DMA controller clock enable */
		__HAL_RCC_DMA1_CLK_ENABLE();


	
	  /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA1_Channel5;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_NORMAL;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler(errDMAInit);
    }

    __HAL_LINKDMA(&huart1,hdmarx,hdma_usart1_rx);

    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA1_Channel4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler(errDMAInit);
    }

	  __HAL_LINKDMA(&huart1,hdmatx,hdma_usart1_tx);
	
		/* DMA interrupt init */
		/* DMA1_Channel4_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 6, 0);
		HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
		/* DMA1_Channel5_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 6, 0);
		HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /*Настройка DMA*/
	DMA_InitTypeDef_Std	DMA_InitStructure;
	
	// Передача данных из периферии в память
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	//Так как у нас массив данных нужно инкрементировать номер ячейки памяти
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	// Настройки размера данных
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	
	
	/*Настройка DMA для АЦП1*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_poll.adc1;
	DMA_InitStructure.DMA_BufferSize = 4;
	//Запуск и иниициализация DMA1_Ch1 = ADC1
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	/*Настройка DMA для АЦП2*/
	DMA_DeInit(DMA2_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC2->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_poll.adc2;
	DMA_InitStructure.DMA_BufferSize = 7;
	//Запуск и иниициализация DMA1_Ch2 = ADC2
	DMA_Init(DMA2_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA2_Channel1, ENABLE);
	
	/*Настройка DMA для АЦП3*/
	DMA_DeInit(DMA2_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC3->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_poll.adc3;
	DMA_InitStructure.DMA_BufferSize = 8;
	//Запуск и иниициализация DMA2_Ch1 = ADC3
	DMA_Init(DMA2_Channel5, &DMA_InitStructure);
	DMA_Cmd(DMA2_Channel5, ENABLE);
	
	/*Настройка DMA для АЦП4*/
	DMA_DeInit(DMA2_Channel2);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC4->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_poll.adc4;
	DMA_InitStructure.DMA_BufferSize = 2;
	//Запуск и иниициализация DMA2_Ch1 = ADC4
	DMA_Init(DMA2_Channel2, &DMA_InitStructure);
	DMA_Cmd(DMA2_Channel2, ENABLE);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{
	ADC_InitTypeDef_Std				ADC_InitStructure;
	ADC_CommonInitTypeDef			ADC_CommonInitStructure;
	
	/* Initialize ADC structure */
	ADC_StructInit(&ADC_InitStructure);
	
	/* Calibration procedure ADC1 */ 
  ADC_VoltageRegulatorCmd(ADC1, ENABLE); 
  ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);
  ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1) != RESET );
	int calibration_value = ADC_GetCalibrationValue(ADC1);
	
	/* Настраиваем непрерывные преобразования  */
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                                                                    
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;                    
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;             
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;                  
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;          
  ADC_CommonInit(ADC1, &ADC_CommonInitStructure);
	ADC_CommonInit(ADC2, &ADC_CommonInitStructure);
	ADC_CommonInit(ADC3, &ADC_CommonInitStructure);
	ADC_CommonInit(ADC4, &ADC_CommonInitStructure);		
  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; 
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;         
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;   
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
  
  ADC_InitStructure.ADC_NbrOfRegChannel = 4;
  ADC_Init(ADC1, &ADC_InitStructure);
	
	 /* ADC1 regular channel2, channel3, channel4, channel5, configuration */ 
	ADC_RegularChannelSequencerLengthConfig(ADC1,4);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 2, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 3, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 4, ADC_SampleTime_7Cycles5);
	
	/* Configure I5A as the single analog watchdog guarded channel */
	ADC_AnalogWatchdog1SingleChannelConfig(ADC1, ADC_Channel_2);
	ADC_AnalogWatchdog1ThresholdsConfig(ADC1, I_KZ_max, 0x1000-I_KZ_max);
	ADC_AnalogWatchdogCmd(ADC1,ADC_AnalogWatchdog_SingleRegEnable);
	ADC_ITConfig(ADC1, ADC_IT_AWD1, ENABLE);
	
  ADC_DMACmd(ADC1, ENABLE);
  ADC_DMAConfig(ADC1, ADC_DMAMode_Circular); 
	
	/* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
	/* wait for ADRDY */
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{
	ADC_InitTypeDef_Std				ADC_InitStructure;
	ADC_CommonInitTypeDef			ADC_CommonInitStructure;
	
	/* Initialize ADC structure */
	ADC_StructInit(&ADC_InitStructure);
	
	/* Calibration procedure ADC2 */ 
	ADC_VoltageRegulatorCmd(ADC2, ENABLE); 
  ADC_SelectCalibrationMode(ADC2, ADC_CalibrationMode_Single);	
  ADC_StartCalibration(ADC2);
	while(ADC_GetCalibrationStatus(ADC2) != RESET );
	int calibration_value = ADC_GetCalibrationValue(ADC2);
	
	/* Настраиваем непрерывные преобразования  */
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                                                                    
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;                    
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;             
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;                  
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;          
  ADC_CommonInit(ADC1, &ADC_CommonInitStructure);
	ADC_CommonInit(ADC2, &ADC_CommonInitStructure);
	ADC_CommonInit(ADC3, &ADC_CommonInitStructure);
	ADC_CommonInit(ADC4, &ADC_CommonInitStructure);		
  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; 
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;         
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;   
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
	
	ADC_InitStructure.ADC_NbrOfRegChannel = 7;
	ADC_Init(ADC2, &ADC_InitStructure);
	
	 /* ADC2 regular channel configuration */ 
	ADC_RegularChannelSequencerLengthConfig(ADC2,7);
  ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_2, 2, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_3, 3, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_4, 4, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_5, 5, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_11, 6, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 7, ADC_SampleTime_7Cycles5);
	
	/* Configure the ADC Thresholds for I_KZ_amplitude=317A (150A*1.5*1.41=317)*/
	ADC_AnalogWatchdog2ThresholdsConfig(ADC2, I_KZ_max>>4, 0x7F-((I_KZ_max>>4)-0x7F));
	ADC_AnalogWatchdogCmd(ADC2,ADC_AnalogWatchdog_SingleRegEnable);
	ADC_ITConfig(ADC2, ADC_IT_AWD2, ENABLE);
	
	ADC_DMACmd(ADC2, ENABLE);
  ADC_DMAConfig(ADC2, ADC_DMAMode_Circular); 
	
		/* Enable ADC2 */
	ADC_Cmd(ADC2, ENABLE);
	/* wait for ADRDY */
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_RDY));
}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{
	ADC_InitTypeDef_Std				ADC_InitStructure;
	ADC_CommonInitTypeDef			ADC_CommonInitStructure;
	
	/* Initialize ADC structure */
	ADC_StructInit(&ADC_InitStructure);
	
	/* Calibration procedure ADC3 */ 
	ADC_VoltageRegulatorCmd(ADC3, ENABLE); 
  ADC_SelectCalibrationMode(ADC3, ADC_CalibrationMode_Single);
  ADC_StartCalibration(ADC3);
	while(ADC_GetCalibrationStatus(ADC3) != RESET );
	int calibration_value = ADC_GetCalibrationValue(ADC3);
	
	/* Настраиваем непрерывные преобразования  */
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                                                                    
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;                    
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;             
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;                  
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;          
  ADC_CommonInit(ADC1, &ADC_CommonInitStructure);
	ADC_CommonInit(ADC2, &ADC_CommonInitStructure);
	ADC_CommonInit(ADC3, &ADC_CommonInitStructure);
	ADC_CommonInit(ADC4, &ADC_CommonInitStructure);		
  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; 
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;         
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;   
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
	
	ADC_InitStructure.ADC_NbrOfRegChannel = 8;
	ADC_Init(ADC3, &ADC_InitStructure);
	
	/* ADC3 regular channel configuration */ 
	ADC_RegularChannelSequencerLengthConfig(ADC3,8);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_1, 1, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_2, 2, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_3, 3, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 4, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 5, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_14, 6, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_15, 7, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_16, 8, ADC_SampleTime_7Cycles5);
	
	/* Configure the ADC Thresholds for I_KZ_amplitude=317A (150A*1.5*1.41=317)*/
	ADC_AnalogWatchdog3ThresholdsConfig(ADC3, I_KZ_max>>4, 0x7F-((I_KZ_max>>4)-0x7F));
	ADC_AnalogWatchdogCmd(ADC3,ADC_AnalogWatchdog_SingleRegEnable);
	ADC_ITConfig(ADC3, ADC_IT_AWD3, ENABLE);
	
	ADC_DMACmd(ADC3, ENABLE);
  ADC_DMAConfig(ADC3, ADC_DMAMode_Circular); 	
	
		/* Enable ADC3 */
	ADC_Cmd(ADC3, ENABLE);
  /* wait for ADRDY */
  while(!ADC_GetFlagStatus(ADC3, ADC_FLAG_RDY));	
}

/**
  * @brief ADC4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC4_Init(void)
{
	ADC_InitTypeDef_Std				ADC_InitStructure;
	ADC_CommonInitTypeDef			ADC_CommonInitStructure;
	
	/* Initialize ADC structure */
	ADC_StructInit(&ADC_InitStructure);
	
		/* Calibration procedure ADC4 */ 
	ADC_VoltageRegulatorCmd(ADC4, ENABLE); 
  ADC_SelectCalibrationMode(ADC4, ADC_CalibrationMode_Single);
  ADC_StartCalibration(ADC4);
	while(ADC_GetCalibrationStatus(ADC4) != RESET );
	int calibration_value = ADC_GetCalibrationValue(ADC4);
	
	/* Настраиваем непрерывные преобразования  */
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                                                                    
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;                    
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;             
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;                  
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;          
  ADC_CommonInit(ADC1, &ADC_CommonInitStructure);
	ADC_CommonInit(ADC2, &ADC_CommonInitStructure);
	ADC_CommonInit(ADC3, &ADC_CommonInitStructure);
	ADC_CommonInit(ADC4, &ADC_CommonInitStructure);		
  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; 
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;         
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;   
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
	
	ADC_InitStructure.ADC_NbrOfRegChannel = 2;
	ADC_Init(ADC4, &ADC_InitStructure);
	
	/* ADC4 regular channel configuration */ 
	ADC_RegularChannelSequencerLengthConfig(ADC4,2);
	ADC_RegularChannelConfig(ADC4, ADC_Channel_1, 1, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC4, ADC_Channel_6, 2, ADC_SampleTime_7Cycles5);
	
		ADC_DMACmd(ADC4, ENABLE);
  ADC_DMAConfig(ADC4, ADC_DMAMode_Circular);

	/* Enable ADC4 */
	ADC_Cmd(ADC4, ENABLE);
  /* wait for ADRDY */
  while(!ADC_GetFlagStatus(ADC4, ADC_FLAG_RDY));
}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 4000;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler(errTIMInit);
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler(errTIMInit);
  }

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler(errUART4Init);
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
	huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler(errUART1Init);
  }
	
	/* Сброс SIM800L */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_SET);
}


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();


  /*Configure GPIO pins : PE2 PE3 PE4 PE5 
                           PE6 PE0 PE1 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6|GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PD10 PD11 PD12 PD13 
                           PD14 PD15 PD0 PD1 
                           PD2 PD3 PD4 PD5 
                           PD6 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13 
                          |GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1 
                          |GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	/* Сброс SIM800L */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_SET);

  /*Configure GPIO pins : PC6 PC7 PC8 PC9 
                           PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9 
                          |GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA12 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB4 PB5 PB6 PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 
                          |GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}
