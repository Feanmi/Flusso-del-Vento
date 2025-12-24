// Масштабирующие коэффициенты калибровки (y = scale*x)
#define SCALE_ACC_X 1.0
#define SCALE_ACC_Y 1.0
#define SCALE_ACC_Z 1.0

#define CALIBRATE_SAMPLES 1e3

// Структура для калибровочных данных
typedef struct {
    double offset_x;
    double offset_y;
    double offset_z;
    double scale_x;
    double scale_y;
    double scale_z;
} AccelCalibration_t;

// Функции
AccelCalibration_t CalibrateAccelerometer(MPU6050_t *accel, int samples = 1e3);
void GetCalibratedAccel(MPU6050_t* raw, MPU6050_t* calibrated, AccelCalibration_t* accelCalib);
double LowPassFilter(double input, double previous, double alpha);