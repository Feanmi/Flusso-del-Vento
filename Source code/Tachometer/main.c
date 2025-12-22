/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdio.h"
#include "string.h"
#include <math.h>

#define AVG_SAMPLES        8
#define TIM_FREQ           8000000.0f
#define MAX_ACCEPT_HZ      400.0f   
#define MAX_REL_CHANGE     2.0f     
#define TIMEOUT_MS         500
#define WHEEL_DIAMETER     0.03f
#define PI 3.14159265f

volatile uint32_t last_pulse_tick = 0; 

UART_HandleTypeDef huart1;
TIM_HandleTypeDef htim2;

volatile uint32_t last_capture = 0;
volatile float frequency = 0.0f;
volatile uint32_t last_pulse_time = 0; 
volatile float frequency_filtered = 0.0f;

float freq_buf[AVG_SAMPLES];
uint8_t freq_idx = 0;
uint8_t freq_count = 0; 

float freq_buf[AVG_SAMPLES];
uint8_t buf_index = 0;


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        uint32_t capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        uint32_t diff;

        if (capture >= last_capture)
            diff = capture - last_capture;
        else
            diff = (0xFFFF - last_capture) + capture + 1;

        last_capture = capture;

        if (diff == 0) return; 

        
          float hz = TIM_FREQ / (float)diff; 

      
        float cur_avg = 0.0f;
        if (freq_count > 0) {
            for (uint8_t i = 0; i < freq_count; ++i) cur_avg += freq_buf[i];
            cur_avg /= (float)freq_count;
        }

        
        if (hz > MAX_ACCEPT_HZ) {
            
            return;
        }

        if (freq_count > 0 && cur_avg > 0.0f) {
            float ratio = (hz > cur_avg) ? (hz / cur_avg) : (cur_avg / hz);
            if (ratio > MAX_REL_CHANGE) {
                return;
            }
        }

        freq_buf[freq_idx] = hz;
        freq_idx = (freq_idx + 1) % AVG_SAMPLES;
        if (freq_count < AVG_SAMPLES) freq_count++;

        float sum = 0.0f;
        for (uint8_t i = 0; i < freq_count; ++i) sum += freq_buf[i];
        frequency_filtered = sum / (float)freq_count;

        last_pulse_tick = HAL_GetTick();
    }
}

float SPEED_MEASURED_OPT(void)
{
    if((HAL_GetTick()-last_pulse_tick) > TIMEOUT_MS){
        frequency_filtered = 0;
        freq_idx = 0;
        freq_count = 0;
    }
    return frequency_filtered;
}

void SEND_UART_FREQ(float f)
{
    char msg[40];
    int len = sprintf(msg,"Freq = %.2f Hz\r\n",f);
    HAL_UART_Transmit(&huart1,(uint8_t*)msg,len,100);
}

float GET_SPEED_MS(float frequency)
{
    float wheel_circumference = PI * WHEEL_DIAMETER;  
    float speed = wheel_circumference * frequency;    // v = f * C
    return speed;
}

void SEND_UART_SPEED(float speed)
{
    char msg[40];
    int len = sprintf(msg,"Speed = %.3f m/s\r\n", speed);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, len, 100);
}

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	  last_capture = 0;
    last_pulse_tick = 0;
    frequency_filtered = 0.0f;
    for(int i = 0; i < AVG_SAMPLES; i++) {
    freq_buf[i] = 0;
    }
    freq_idx = 0;
    freq_count = 0;
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_TIM2_Init();

    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

    char msg[64];

    while (1)
    {
       //float f = SPEED_MEASURED_OPT(); 
       //SEND_UART_FREQ(f);              
       //HAL_Delay(200);
	    float f = SPEED_MEASURED_OPT(); 
      //float speed = GET_SPEED_MS(f);  

      SEND_UART_FREQ(f);             
      //SEND_UART_SPEED(speed);        
      HAL_Delay(1000);
    }

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 15;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
