#include "acc.h"
// Structure for accelerometer calibration data

I2C_HandleTypeDef hi2c1;
RawData_Def myAccelRaw, myGyroRaw;
ScaledData_Def myAccelScaled, myGyroScaled;

// Calibration structure
AccelCalibration_t accelCalib = {0};

// Velocity calculation variables
float velocity = 0.0f;
float vel2 = 0.0f;
float last_accel_x = 0.0f;
long float av;
uint32_t last_time = 0;
uint8_t is_calibrated = 0;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

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
    accelCalib.scale_x = SCALE_ACC_X;
    accelCalib.scale_y = SCALE_ACC_Y;
    accelCalib.scale_z = SCALE_ACC_Z;
    
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


int main_acc() {

    double velocity = 0;
    double last_accel_x = 0;
    uint32_t last_time = 0;
    uint32_t reset_counter = 0;
    
    HAL_Init();

  SystemClock_Config();



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
    CalibrateAccelerometer(CALIBRATE_SAMPLES);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while(1) {
        MPU6050_t rawAccel, calibratedAccel;

            MPU6050_Get_Accel_Scale(&myAccelScaled);
        //MPU6050_Get_Gyro_Scale(&myGyroScaled);
        
		
		sko = sqrt(pow(myAccelScaled.x,2)+pow(myAccelScaled.y,2)+pow(myAccelScaled.z,2));
        // Apply calibration if available
        if(is_calibrated) {
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
        
        
        av = sqrt(pow(myAccelScaled.x,2)+pow(myAccelScaled.y,2)+pow(myAccelScaled.z,2));
				
			float sum=0;
			for (int i=0; i<1000; i=i+1){
							
				float dy =(av-1000) * 0.00980665f;
				float dt=0.001;
				sum += dy * dt;
				MPU6050_Get_Accel_Scale(&myAccelScaled);
				HAL_Delay(1);
			}
		vel2 = sum;

        
        // TODO:
        // отсюда можно вывести скорость на экранчик
        
        HAL_Delay(10);
    }
}