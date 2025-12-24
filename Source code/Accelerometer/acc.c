#include "acc.h"

// Функция калибровки акселерометра (вызывать при неподвижном датчике)
AccelCalibration_t CalibrateAccelerometer(MPU6050_t* accel, int samples = 1e3) {
    AccelCalibration_t accelCalib = {0};
    MPU6050_t accelRaw; // структура для заполнения сырыми данными с датчика 
    double sum_x = 0, sum_y = 0, sum_z = 0;
    
    for(int i = 0; i < samples; i++) {
        MPU6050_Get_Accel_Scale(&accelRaw);
        sum_x += accelRaw.x;
        sum_y += accelRaw.y;
        sum_z += accelRaw.z;
        HAL_Delay(10);
    }
    
    // Считаем, что при калибровке датчик лежит на столе
    // Z-ось должна показывать ~1g, X и Y ~0g
    accelCalib.offset_x = sum_x / samples;
    accelCalib.offset_y = sum_y / samples;
    accelCalib.offset_z = (sum_z / samples) - 1; // Вычитаем гравитацию
    
    // Коэффициенты масштабирования
    accelCalib.scale_x = SCALE_ACC_X;
    accelCalib.scale_y = SCALE_ACC_Y;
    accelCalib.scale_z = SCALE_ACC_Z;

    return accelCalib;
}

// Функция получения калиброванных данных
void GetCalibratedAccel(MPU6050_t* raw, MPU6050_t* calibrated, AccelCalibration_t* accelCalib) {
    calibrated->x = (raw->x - accelCalib.offset_x) * accelCalib.scale_x;
    calibrated->y = (raw->y - accelCalib.offset_y) * accelCalib.scale_y;
    calibrated->z = (raw->z - accelCalib.offset_z) * accelCalib.scale_z;
}

// Простой фильтр низких частот
double LowPassFilter(double input, double previous, double alpha) {
    return alpha * input + (1.0f - alpha) * previous;
}

// Основная функция
int main_acc() {

    double velocity = 0;
    double last_accel_x = 0;
    uint32_t last_time = 0;
    uint32_t reset_counter = 0;
    
    // Инициализация MPU6050
    MPU6050_Init();
    
    // Калибровка (датчик должен быть неподвижен)
    printf("Старт калибровки\n");
    AccelCalibration_t accelCalib = CalibrateAccelerometer(&myAccelScaled, CALIBRATE_SAMPLES);
    printf("Калибровка завершена\n");

    // Получение времени начала
    last_time = HAL_GetTick();

    // Основной цикл
    while(1) {
        MPU6050_t rawAccel, calibratedAccel;
        
        // Получение сырых данных
        MPU6050_Get_Accel_Scale(&rawAccel);
        
        // Калибровка данных
        GetCalibratedAccel(&rawAccel, &calibratedAccel, &accelCalib);
        
        // Фильтрация шумов
        double currnet_accel_x = 0;
        currnet_accel_x = LowPassFilter(calibratedAccel.x, currnet_accel_x, 0.1f);
        
        // Вычисление временного интервала
        uint32_t current_time = HAL_GetTick();
        double dt = (current_time - last_time) * 1000; // в секундах
        last_time = current_time;
        
        // Интегрирование ускорения для получения скорости
        // Используем трапециевидное интегрирование для большей точности
        velocity += (last_accel_x + currnet_accel_x) * 0.5 * dt;
        last_accel_x = currnet_accel_x;
        
        // Периодический сброс дрейфа скорости (нужно настроить под вашу задачу)
        if(reset_counter++ > 1000) { // Каждые ~10 секунд
            // Сбрасываем скорость если ускорение близко к нулю
            if(fabsf(currnet_accel_x) < 0.05f) {
                velocity *= 0.5; // Плавный сброс
            }
            reset_counter = 0;
        }
        
        // Определение удара/столкновения
        double total_g = sqrt(calibratedAccel.x*calibratedAccel.x + 
                           calibratedAccel.y*calibratedAccel.y + 
                           calibratedAccel.z*calibratedAccel.z);
        
        if (total_g > 2.5) {
            // Сброс скорости при ударе
            velocity = 0;
            
            // TODO:
            // тут можно регистрировать столкновение

        }
        
        // TODO:
        // отсюда можно вывести скорость на экранчик
        
        HAL_Delay(10);
    }
}