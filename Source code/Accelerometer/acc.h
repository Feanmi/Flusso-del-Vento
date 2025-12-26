// Scaling factors (y = scale*x)
#define SCALE_ACC_X 1.0
#define SCALE_ACC_Y 1.0
#define SCALE_ACC_Z 1.0

#define CALIBRATE_SAMPLES 10

// Structure for calibrated data
typedef struct {
    double offset_x;
    double offset_y;
    double offset_z;
    double scale_x;
    double scale_y;
    double scale_z;
} AccelCalibration_t;

// Function prototypes
void CalibrateAccelerometer(int samples);
void GetCalibratedAccel(ScaledData_Def *raw, ScaledData_Def *calibrated);
float LowPassFilter(float input, float previous, float alpha);