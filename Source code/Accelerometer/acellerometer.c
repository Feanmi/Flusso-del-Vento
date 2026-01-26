/* ====== Внутренние глобальные переменные ====== */
static I2C_HandleTypeDef *mpu_i2c;

static RawData_Def myAccelRaw;
static ScaledData_Def myAccelScaled;
static ScaledData_Def calibratedAccel;

static AccelCalibration_t accelCalib = {0};

static uint8_t is_calibrated = 0;
static float filtered_accel_x = 0;

/* ====== Внешние глобальные переменные ====== */
int16_t SPEED_MEASURED_ACC = 0;
uint8_t DATA_VALID_FLAG[2] = {0};

/* ====== Функция для инициализации акселерометра-гироскопа ====== */
void init_gyroscope(I2C_HandleTypeDef *hi2c)
{
    MPU_ConfigTypeDef mpu_cfg;
    mpu_i2c = hi2c;
    MPU6050_Init(mpu_i2c);

    mpu_cfg.Accel_Full_Scale = AFS_SEL_4g;
    mpu_cfg.Gyro_Full_Scale  = FS_SEL_500;
    mpu_cfg.ClockSource      = Internal_8MHz;
    mpu_cfg.CONFIG_DLPF      = DLPF_184A_188G_Hz;
    mpu_cfg.Sleep_Mode_Bit   = 0;

    MPU6050_Config(&mpu_cfg);
    HAL_Delay(100);
    CalibrateAccelerometer(CALIBRATE_SAMPLES);
    DATA_VALID_FLAG[2] = 0;
}

/* ====== Функция для обновления данных о текущей скорости ====== */
void read_accelleration(void)
{
    DATA_VALID_FLAG[2] = 0
    float sko;
    float sum = 0;

    MPU6050_Get_Accel_Scale(&myAccelScaled);

    if (is_calibrated) {
        GetCalibratedAccel(&myAccelScaled, &calibratedAccel);
    }

    /* Модуль ускорения */
    sko = sqrtf(myAccelScaled.x * myAccelScaled.x +
                myAccelScaled.y * myAccelScaled.y +
                myAccelScaled.z * myAccelScaled.z);

    /* Интегрирование ускорения -> скорость */
    for (uint16_t i = 0; i < INTEGRATE_SAMPLES; i++) {
        float acc = sko * G_CONST;
        float dt  = 0.001f;
        sum += acc * dt;

        MPU6050_Get_Accel_Scale(&myAccelScaled);
        HAL_Delay(1);
    }

    SPEED_MEASURED_ACC = ConvertSpeed(sum);
    DATA_VALID_FLAG[2] = 1;
}

/* ====== Функция для начальной калибровки акселерометра ====== */
static void CalibrateAccelerometer(uint16_t samples = CALIBRATE_SAMPLES)
{
    float sx = 0, sy = 0, sz = 0;
    ScaledData_Def acc;

    for (uint16_t i = 0; i < samples; i++) {
        MPU6050_Get_Accel_Scale(&acc);
        sx += acc.x;
        sy += acc.y;
        sz += acc.z;
        HAL_Delay(10);
    }

    accelCalib.offset_x = sx / samples;
    accelCalib.offset_y = sy / samples;
    accelCalib.offset_z = (sz / samples) - 1.0f;

    accelCalib.scale_x = SCALE_ACC_X;
    accelCalib.scale_y = SCALE_ACC_Y;
    accelCalib.scale_z = SCALE_ACC_Z;

    is_calibrated = 1;
}

/* ====== Функция для получения данных о текущей скорости с учётом калиброаки ====== */
static void GetCalibratedAccel(ScaledData_Def *raw,
                               ScaledData_Def *calibrated)
{
    calibrated->x = (raw->x - accelCalib.offset_x) * accelCalib.scale_x;
    calibrated->y = (raw->y - accelCalib.offset_y) * accelCalib.scale_y;
    calibrated->z = (raw->z - accelCalib.offset_z) * accelCalib.scale_z;
}

/* ====== Функция для перевода скорости в диапазон [-MAX_SPEED; MAX_SPEED] ====== */
int ConvertSpeed(float speed, int max_acc = MAX_ACC){
    return speed / max_acc * 255;
}
