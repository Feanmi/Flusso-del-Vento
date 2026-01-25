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
#include <math.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "TJ_MPU6050.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// Structure for accelerometer calibration data
typedef struct {
    float offset_x;
    float offset_y;
    float offset_z;
    float scale_x;
    float scale_y;
    float scale_z;
} AccelCalibration_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
RawData_Def myAccelRaw, myGyroRaw;
ScaledData_Def myAccelScaled, myGyroScaled;

// Calibration structure
AccelCalibration_t accelCalib = {0};

// Velocity calculation variables
float velocity = 0.0f;
float vel2 = 0.0f;
float last_accel_x = 0.0f;
long float sko;
uint32_t last_time = 0;
uint8_t is_calibrated = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

// Function prototypes
void CalibrateAccelerometer(int samples);
void GetCalibratedAccel(ScaledData_Def *raw, ScaledData_Def *calibrated);
float LowPassFilter(float input, float previous, float alpha);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Accelerometer calibration function (call when sensor is stationary)
void CalibrateAccelerometer(int samples) {
    float sum_x = 0, sum_y = 0, sum_z = 0;
    ScaledData_Def accelRaw;
    
    // Collect samples for averaging
    for(int i = 0; i < samples; i++) {
        MPU6050_Get_Accel_Scale(&accelRaw);
        sum_x += accelRaw.x;
        sum_y += accelRaw.y;
        sum_z += accelRaw.z;
        HAL_Delay(10);
    }
    
    // Calculate average offsets
    // Assuming sensor is flat on table during calibration:
    // Z-axis should read ~1g, X and Y ~0g
    accelCalib.offset_x = sum_x / samples;
    accelCalib.offset_y = sum_y / samples;
    accelCalib.offset_z = (sum_z / samples) - 1.0f; // Subtract gravity
    
    // Scaling factors (can be calibrated more precisely if needed)
    accelCalib.scale_x = 1.0f;
    accelCalib.scale_y = 1.0f;
    accelCalib.scale_z = 1.0f;
    
    is_calibrated = 1;
}

// Get calibrated accelerometer data
void GetCalibratedAccel(ScaledData_Def *raw, ScaledData_Def *calibrated) {
    calibrated->x = (raw->x - accelCalib.offset_x) * accelCalib.scale_x;
    calibrated->y = (raw->y - accelCalib.offset_y) * accelCalib.scale_y;
    calibrated->z = (raw->z - accelCalib.offset_z) * accelCalib.scale_z;
}

// Simple low-pass filter for noise reduction
float LowPassFilter(float input, float previous, float alpha) {
    return alpha * input + (1.0f - alpha) * previous;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
    MPU_ConfigTypeDef myMpuConfig;
    ScaledData_Def calibratedAccel;
    static float filtered_accel_x = 0;
    static uint32_t reset_counter = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
    MPU6050_Init(&hi2c1);
    
    // Initialize timing
    last_time = HAL_GetTick();
  /* USER CODE END 2 */
    
    myMpuConfig.Accel_Full_Scale = AFS_SEL_4g;
    myMpuConfig.ClockSource = Internal_8MHz;
    myMpuConfig.CONFIG_DLPF = DLPF_184A_188G_Hz;
    myMpuConfig.Gyro_Full_Scale = FS_SEL_500;
    myMpuConfig.Sleep_Mode_Bit = 0;
    MPU6050_Config(&myMpuConfig);
    
    // Wait for MPU6050 to stabilize
    HAL_Delay(100);
    
    // Perform calibration (sensor must be stationary!)
    CalibrateAccelerometer(10);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
        
        // Get sensor data
        MPU6050_Get_Accel_Scale(&myAccelScaled);
        //MPU6050_Get_Gyro_Scale(&myGyroScaled);
        
		
		sko = sqrt(pow(myAccelScaled.x,2)+pow(myAccelScaled.y,2)+pow(myAccelScaled.z,2));
        // Apply calibration if available
        if(is_calibrated || 0) {
            GetCalibratedAccel(&myAccelScaled, &calibratedAccel);
            
            // Remove gravity component (assuming Z-axis points upward)
            calibratedAccel.z -= 1.0f;
            
            // Filter accelerometer data to reduce noise
            filtered_accel_x = LowPassFilter(calibratedAccel.x, filtered_accel_x, 0.1f);
        } else {
            // Use raw data if not calibrated
            filtered_accel_x = LowPassFilter(myAccelScaled.x, filtered_accel_x, 0.1f);
        }
        
        // Calculate time delta since last measurement
        //uint32_t current_time = HAL_GetTick();
        //float dt = (current_time - last_time) / 1000.0f; // Convert to seconds
        //last_time = current_time;
        
        
        sko = sqrt(pow(myAccelScaled.x,2)+pow(myAccelScaled.y,2)+pow(myAccelScaled.z,2));
				
						float sum=0;
						for (int i=0; i<1000; i=i+1){
							
							float dy =(sko-1000) * 0.00980665f;
							float dt=0.001;
							sum += dy * dt;
							MPU6050_Get_Accel_Scale(&myAccelScaled);
							HAL_Delay(1);
						}
						vel2 = sum;
   
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

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

#ifdef  USE_FULL_ASSERT
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