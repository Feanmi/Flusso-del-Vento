#include "stm32f4xx_hal.h"
#include "TJ_MPU6050.h"
#include <math.h>

#define SCALE_ACC_X       1.0
#define SCALE_ACC_Y       1.0
#define SCALE_ACC_Z       1.0
#define CALIBRATE_SAMPLES 10
#define INTEGRATE_SAMPLES 1000
#define MAX_SPEED         10 // 10 m/s
#define G_CONST           9.80665

/* ====== External data ====== */
extern float SPEED_MEASURED_ACC;
extern uint8_t DATA_VALID_FLAG[2];

/* ====== Public API ====== */
void init_gyroscope(I2C_HandleTypeDef *hi2c);
void read_accelleration(void);

/* ====== Private types ====== */
typedef struct {
    float offset_x;
    float offset_y;
    float offset_z;
    float scale_x;
    float scale_y;
    float scale_z;
} AccelCalibration_t;

/* ====== Private functions ====== */
static void CalibrateAccelerometer(uint16_t samples);
static void GetCalibratedAccel(ScaledData_Def *raw, ScaledData_Def *calibrated);
static float LowPassFilter(float input, float previous, float alpha);
int ConvertSpeed(float speed, int max_acc = MAX_ACC);
